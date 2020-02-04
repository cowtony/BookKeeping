#include "InvestmentAnalysis.h"
#include "ui_InvestmentAnalysis.h"

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
      roiItem->setTextColor(Qt::red);
    }
    roiItem->setTextAlignment(Qt::AlignRight);
    ui->investmentTableWidget->setItem(row, 1, roiItem);
  }
  ui->investmentTableWidget->resizeColumnsToContents();

  // Setup plot chart
  on_resetButton_clicked();
  ui->chartView->setRenderHint(QPainter::Antialiasing);
  ui->chartView->setRubberBand(QChartView::HorizontalRubberBand);
}

InvestmentAnalysis::~InvestmentAnalysis() {
  delete ui;
}

void InvestmentAnalysis::analysisInvestment(const QString& investmentName) {
  // 1. Setup two related account from Asset & Revenue.
  Account asset(Account::Asset, "", investmentName);
  Account revenue(Account::Revenue, "Investment", investmentName);
  for (const QString& categoryName : g_book.getCategories(Account::Asset)) {
    if (g_book.accountExist(Account(Account::Asset, categoryName, investmentName))) {
      if (!asset.m_category.isEmpty()) {
        // TODO: make this an error message.
        qDebug() << "Error! More than one account in asset has the investment name.";
        return;
      }
      asset = Account(Account::Asset, categoryName, investmentName);
    }
  }

  // 2. Scan and analysis through all related transactions
  QMap<QDate, double> returnHistory; // Key: date, Value: log2(daily return) until date.
  QList<Money> transferHistory; // Store all the transfer activities since last summary.
  Money runningBalance(QDate(), USD, 0.00), gainOrLoss(QDate(), USD, 0.00), accountChange(QDate(), USD, 0.00);
  const QDateTime start = QDateTime(QDate(1990, 05, 25), QTime());
  QList<Transaction> transactions = g_book.queryTransactions(start, QDateTime::currentDateTime(), "", {asset, revenue}, true, true);
  for (int i = 0; i < transactions.size(); i++) {
    // Init the day before first transaction date and set log(ROI) to 0.
    if (returnHistory.empty()) {
      returnHistory.insert(transactions.at(i).m_dateTime.date().addDays(-1), 0.00);
    }

    gainOrLoss += transactions.at(i).getMoneyArray(revenue).sum();
    accountChange += transactions.at(i).getMoneyArray(asset).sum();
    runningBalance += accountChange;

    // Skip calculation if next transaction is in the same day.
    if (i + 1 < transactions.size()) {
      if (transactions.at(i + 1).m_dateTime.date() == transactions.at(i).m_dateTime.date()) {
        continue;
      }
    }

    // If has activity in revenue
    if (transactions.at(i).accountExist(revenue)) {
      double log2_ROI = getLog2DailyROI(transferHistory, gainOrLoss); // Get the log2 return so that we can use + instead of * later.
      if (!returnHistory.contains(transactions.at(i).m_dateTime.date())) {
        returnHistory.insert(transactions.at(i).m_dateTime.date(), log2_ROI);
      } else {
        // We should never have duplicated date since it's merged in the begining.
        qDebug() << "Duplicate date found!";
        qDebug() << investmentName << transactions.at(i).m_dateTime << transactions.at(i).m_description;
        system("pause");
      }
      transferHistory.clear();
      transferHistory << runningBalance;
    }
    else {
      transferHistory << accountChange - gainOrLoss;
    }
    gainOrLoss = Money(QDate(), USD, 0.00);
    accountChange = Money(QDate(), USD, 0.00);
  }

  m_data.insert(investmentName, returnHistory);
}

double InvestmentAnalysis::getLog2DailyROI(const QList<Money>& history, const Money& gainOrLoss) {
  if (history.empty()) {
    return 0;  // This suppose never happen.
  }

  double min = -1.0, max = 1.0;
  double log2_dailyROI = 0.0;
  while (true) {
    Money gOrL = calculateGainOrLoss(history, log2_dailyROI, gainOrLoss.m_date);
    if (qFabs((gOrL - gainOrLoss).m_amount) < 0.001) {
      return log2_dailyROI;
    } else if (gOrL < gainOrLoss) {
      min = log2_dailyROI;
    } else {
      max = log2_dailyROI;
    }
    log2_dailyROI = (min + max) / 2;
  }
}

Money InvestmentAnalysis::calculateGainOrLoss(const QList<Money>& history, double log2_dailyROI, const QDate& date) {
  Money ret(QDate(), USD, 0.00);
  for (Money money : history) {
    if (money.m_amount < 0) {
      money.m_amount = -money.m_amount;
    }
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

void InvestmentAnalysis::on_investmentTableWidget_cellClicked(int row, int column) {
  QTableWidgetItem* item = ui->investmentTableWidget->item(row, 0);
  if (item->checkState() == Qt::Unchecked) {
    item->setCheckState(Qt::Checked);
  } else if (item->checkState() == Qt::Checked) {
    item->setCheckState(Qt::Unchecked);
  } else {
    return;
  }

  plotInvestments();
}

void InvestmentAnalysis::on_startDateEdit_dateChanged(const QDate &date) {
  plotInvestments();
}

void InvestmentAnalysis::on_axisX_rangeChanged(const QDateTime& start, const QDateTime& end) {
  ui->startDateEdit->setDate(start.date());
  plotInvestments();
}

void InvestmentAnalysis::plotInvestments() {
  QChart* chart = new QChart;
//  chart->setAnimationOptions(QChart::SeriesAnimations);

  QDateTimeAxis* axisX = new QDateTimeAxis;
  axisX->setTitleText("Date");
  axisX->setFormat("yyyy/MM/dd");
  axisX->setTickCount(10);
  axisX->setLinePenColor(Qt::darkGray);
  axisX->setGridLineColor(Qt::darkGray);
  chart->addAxis(axisX, Qt::AlignBottom);

  QValueAxis* axisY = new QValueAxis;
  axisY->setTitleText("Log2(Rate of Return)");
  axisY->setLabelFormat("%.2f");
//  axisY->setLinePenColor(Qt::darkGray);
//  axisY->setGridLineColor(Qt::darkGray);
  chart->addAxis(axisY, Qt::AlignLeft);

  QLogValueAxis* axisYlog = new QLogValueAxis;
  axisYlog->setTitleText("Rate of Return");
  axisYlog->setLabelFormat("%.0f%");

//  axisYlog->setMinorTickCount(9);
  axisYlog->setLinePenColor(Qt::darkGray);
  axisYlog->setGridLineColor(Qt::darkGray);
  chart->addAxis(axisYlog, Qt::AlignRight);

  double minY = 1e12, maxY = -1e12;
  for (int i = 0; i < ui->investmentTableWidget->rowCount(); i++) {
    if (ui->investmentTableWidget->item(i, 0)->checkState() == Qt::Checked) {
      QString investmentName = ui->investmentTableWidget->item(i, 0)->text();
      QLineSeries* lineSeries = new QLineSeries();
      lineSeries->setName(investmentName);

      double value = 0;
      QDate previousDate = ui->startDateEdit->date();
      lineSeries->append(ui->startDateEdit->dateTime().toMSecsSinceEpoch(), value);  // Add first data point as begin.
      for (const QDate& date : m_data.value(investmentName).keys()) {
        if (date <= ui->startDateEdit->date()) {
          continue;
        }
        value += previousDate.daysTo(date) * m_data.value(investmentName).value(date);
        lineSeries->append(QDateTime(date).toMSecsSinceEpoch(), value);

        minY = value < minY ? value : minY;
        maxY = value > maxY ? value : maxY;
        previousDate = date;
      }
      lineSeries->append(QDateTime::currentDateTime().toMSecsSinceEpoch(), value);  // Add last data point as current.

      chart->addSeries(lineSeries);
      // Attach axis must after chart->addSeries().
      lineSeries->attachAxis(axisX);
      lineSeries->attachAxis(axisY);
    }
  }

  // Must connect this after everything done to avoid range change during append data, which is causeing recursive calling.
  QObject::connect(axisX, &QDateTimeAxis::rangeChanged, this, &InvestmentAnalysis::on_axisX_rangeChanged);

  double extra = (maxY - minY) * 0.05;
  axisY->setRange(minY - extra, maxY + extra);
  axisY->applyNiceNumbers();

  int tickCount = 15;
  int count = int(qLn(100) / qLn(2) / (maxY - minY) * tickCount);  // get the integer count value for: base ^ count = 100
  axisYlog->setBase(qPow(100.0, 1.0 / count));
  axisYlog->setRange(qPow(2.0, axisY->min()) * 100, qPow(2.0, axisY->max()) * 100);
  ui->chartView->setChart(chart);
}

void InvestmentAnalysis::on_resetButton_clicked() {
  ui->startDateEdit->setDateTime(g_book.getFirstTransactionDateTime());
}
