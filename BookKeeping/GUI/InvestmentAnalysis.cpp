#include "InvestmentAnalysis.h"
#include "ui_InvestmentAnalysis.h"

#include <QtCharts>

#include "Book.h"

InvestmentAnalysis::InvestmentAnalysis(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::InvestmentAnalysis) {
  ui->setupUi(this);

  // Scan, analysis and save all investment product:
  QStringList investments = g_book.getAccountNames(Account::Revenue, "Investment");
  QApplication::setOverrideCursor(Qt::WaitCursor);
  for (const QString& investmentName : investments) {
    analysisInvestment(investmentName);
  }
  QApplication::restoreOverrideCursor();

  // Get all investment info into list:
  ui->investmentTableWidget->setRowCount(investments.size());
  for (int row = 0; row < investments.size(); row++) {
    ui->investmentTableWidget->setRowHeight(row, 20);
    QTableWidgetItem* item = new QTableWidgetItem(investments.at(row));
    item->setCheckState(Qt::Unchecked);   // To Show the checkbox.
    ui->investmentTableWidget->setItem(row, 0, item);
    double AROI = (calculateAROI(investments.at(row)) - 1.0) * 100;
    QTableWidgetItem* roiItem = new QTableWidgetItem(QString::number(AROI, 'f', 2) + "%");
    if (AROI < 0) {
      roiItem->setTextColor(QColor(Qt::red));
    }
    roiItem->setTextAlignment(Qt::AlignRight);
    ui->investmentTableWidget->setItem(row, 1, roiItem);
  }
  ui->investmentTableWidget->resizeColumnsToContents();

  // Setup plot chart
  QChart* chart = new QChart();
  QChartView *chartView = new QChartView(chart, ui->chartViewWidget);
}

InvestmentAnalysis::~InvestmentAnalysis() {
  delete ui;
}

void InvestmentAnalysis::on_investmentTableWidget_cellClicked(int row, int column) {
  QTableWidgetItem* item = ui->investmentTableWidget->item(row, 0);
  if (item->checkState() == Qt::Unchecked) {
    item->setCheckState(Qt::Checked);
  } else if (item->checkState() == Qt::Checked) {
    item->setCheckState(Qt::Unchecked);
  } else {
    return;
  }

  QStringList checkedItem;
  for (int i = 0; i < ui->investmentTableWidget->rowCount(); i++) {
    if (ui->investmentTableWidget->item(i, 0)->checkState() == Qt::Checked) {
      checkedItem << ui->investmentTableWidget->item(i, 0)->text();
    }
  }
  plotInvestments(checkedItem);
}

void InvestmentAnalysis::analysisInvestment(const QString& investmentName) {
  // 1. Setup two related account from Asset & Revenue.
  QList<Account> accounts;
  accounts << Account(Account::Revenue, "Investment", investmentName);
  for (const QString& categoryName : g_book.getCategories(Account::Asset)) {
    if (g_book.accountExist(Account(Account::Asset, categoryName, investmentName))) {
      accounts << Account(Account::Asset, categoryName, investmentName);
    }
  }
  if (accounts.size() != 2) {
    // TODO: make this an error message.
    qDebug() << "Error! No account or more than one account in asset has the investment name.";
    for (auto account : accounts) {
      qDebug() << account.getTableName() << account.m_category << account.m_name;
    }
    return;
  }

  // 2. Scan and analysis through all related transactions
  QMap<QDate, double> returnHistory; // Key: date, Value: log2(daily return) until date.
  QList<Money> transferHistory; // Store all the transfer activities since last summary.
  Money runningBalance(QDate(), USD, 0.00);
  for (const Transaction& transaction : g_book.queryTransactions(QDateTime(), QDateTime::currentDateTime(), "", accounts, true, true)) {
    // Init to the first transaction date and set log(ROI) to 0.
    if (returnHistory.empty()) {
      returnHistory.insert(transaction.m_dateTime.date(), 0.00);
    }
    Money gainOrLoss = transaction.getMoneyArray(accounts.at(0)).sum();
    Money accountChange = transaction.getMoneyArray(accounts.at(1)).sum();
    Money transfer = accountChange - gainOrLoss;
    runningBalance += accountChange;

    // If has activity in revenue
    if (transaction.accountExist(accounts.at(0))) {
      double log2_ROI = getLog2DailyROI(transferHistory, gainOrLoss); // Get the log2 return so that we can use + instead of * later.
      if (!returnHistory.contains(transaction.m_dateTime.date())) {
        returnHistory.insert(transaction.m_dateTime.date(), log2_ROI);
      } else /* The second revenue happened in the same day, which should always have 0 ROI. */ {
        if (log2_ROI != 0.0) {system("pause");}
      }
      transferHistory.clear();
      transferHistory << runningBalance;
    }
    else {
      transferHistory << transfer;
    }
  }

  m_data.insert(investmentName, returnHistory);
}

double InvestmentAnalysis::getLog2DailyROI(const QList<Money>& history, const Money& gainOrLoss) {
  if (history.empty()) {
    return 0;  // This suppose never happen.
  }
  if (history.first().m_amount < 0) {
    return 0;  // Special handler for "牛荻红SPY".
  }
  double min = -1.0, max = 1.0;
  double log2_dailyROI = 0.0;
  while (true) {
    Money npv = calculateGainOrLoss(history, log2_dailyROI, gainOrLoss.m_date);
    if (qFabs((npv - gainOrLoss).m_amount) < 0.001) {
      return log2_dailyROI;
    } else if (npv < gainOrLoss) {
      min = log2_dailyROI;
    } else {
      max = log2_dailyROI;
    }
    log2_dailyROI = (min + max) / 2;
  }
}

Money InvestmentAnalysis::calculateGainOrLoss(const QList<Money>& history, const double& log2_dailyROI, const QDate& date) {
  Money ret(QDate(), USD, 0.00);
  for (const Money& money : history) {
    ret += money * (qPow(2.0, log2_dailyROI * money.m_date.daysTo(date)) - 1.0);
  }
  return ret;
}

double InvestmentAnalysis::calculateAROI(const QString& investmentName) const {
  if (!m_data.contains(investmentName)) {
    return 0.0;
  }
  const QMap<QDate, double>& history = m_data.value(investmentName);
  double log2_AROI = 0.0;
  QDate previousDate(1990, 05, 25);
  for (const QDate& date : history.keys()) {
    log2_AROI += previousDate.daysTo(date) * history.value(date);
    previousDate = date;
  }
  log2_AROI /= history.firstKey().daysTo(history.lastKey()) / 365.0;
  return qPow(2.0, log2_AROI);
}

void InvestmentAnalysis::plotInvestments(const QStringList& investments) {
  for (const QString& investmentName : investments) {
    QLineSeries* lineSeries = new QLineSeries();
    for (const QDate& date : m_data.value(investmentName).keys()) {
      lineSeries->append(date.day(), m_data.value(investmentName).value(date));
    }

//    chart->addSeries(lineSeries);

  }
}
