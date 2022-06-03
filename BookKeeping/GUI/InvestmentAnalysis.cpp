#include "InvestmentAnalysis.h"
#include "ui_InvestmentAnalysis.h"

InvestmentAnalysis::InvestmentAnalysis(Book& book, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::InvestmentAnalysis), book_(book) {
  ui->setupUi(this);

  // Scan, analysis and save all investment product:
  QList<Account> investments = book_.queryAccounts(Account::Revenue, "Investment");
  QApplication::setOverrideCursor(Qt::WaitCursor);
  for (const Account& investment : investments) {
    analysisInvestment(investment.name);
  }
  QApplication::restoreOverrideCursor();

  // Get all investment info into list:
  ui->investmentTableWidget->setRowCount(investments.size());
  for (int row = 0; row < investments.size(); row++) {
    const QString investment_name = investments.at(row).name;

    ui->investmentTableWidget->setRowHeight(row, 20);
    QTableWidgetItem* item = new QTableWidgetItem(investment_name);
    item->setCheckState(Qt::Unchecked);   // To Show the checkbox.
    ui->investmentTableWidget->setItem(row, 0, item);

    // Set column 1: Discount Rate
    double discount_rate = (discount_rates_.value(investment_name) - 1.0) * 100;
    QTableWidgetItem* discount_rate_item = new QTableWidgetItem(QString::number(discount_rate, 'f', 2) + "%");
    if (discount_rate < 0) {
      discount_rate_item->setForeground(Qt::red);
    }
    discount_rate_item->setTextAlignment(Qt::AlignRight);
    ui->investmentTableWidget->setItem(row, 1, discount_rate_item);

    // Set column 2: APR.
    double APR = 0;
    if (return_histories_.contains(investment_name)) {
      APR = (calculateAPR(return_histories_.value(investment_name)) - 1.0) * 100;
    }
    QTableWidgetItem* apr_item = new QTableWidgetItem(QString::number(APR, 'f', 2) + "%");
    if (APR < 0) {
      apr_item->setForeground(Qt::red);
    }
    apr_item->setTextAlignment(Qt::AlignRight);
    ui->investmentTableWidget->setItem(row, 2, apr_item);


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
  for (const QString& categoryName : book_.queryCategories(Account::Asset)) {
    if (book_.accountExist(Account(Account::Asset, categoryName, investmentName))) {
      if (!asset.category.isEmpty()) {
        // TODO: make this an error message.
        qDebug() << "Error! More than one account in asset has the investment name.";
        return;
      }
      asset = Account(Account::Asset, categoryName, investmentName);
    }
  }

  // 2. Scan and analysis through all related transactions
  QMap<QDate, double> returnHistory; // Key: date, Value: log2(daily return) until date.
  QList<Money> local_transfer_history; // Store all the transfer activities since last summary.
  QList<Money> alltime_transfer_history;
  Money runningBalance(QDate(), Currency::USD, 0.00), gainOrLoss(QDate(), Currency::USD, 0.00), balanceChange(QDate(), Currency::USD, 0.00);
  QList<Transaction> transactions =
      book_.queryTransactions(TransactionFilter({asset, revenue})
                              .toTime(QDateTime::currentDateTime())
                              .useAnd());
  for (int i = 0; i < transactions.size(); i++) {
    // Init the day before first transaction date and set log(ROI) to 0.
    if (returnHistory.empty()) {
      returnHistory.insert(transactions.at(i).date_time_.date().addDays(-1), 0.00);
    }

    gainOrLoss += transactions.at(i).getMoneyArray(revenue).sum();
    balanceChange += transactions.at(i).getMoneyArray(asset).sum();

    // Skip calculation if next transaction is in the same day.
    if (i + 1 < transactions.size()) {
      if (transactions.at(i + 1).date_time_.date() == transactions.at(i).date_time_.date()) {
        continue;
      }
    }
    runningBalance += balanceChange;
    local_transfer_history   << balanceChange - gainOrLoss;
    alltime_transfer_history << balanceChange - gainOrLoss;

    // If has activity in revenue
    if (gainOrLoss.amount_ != 0.0) {
      double log2_ROI = backCalculateNPV(local_transfer_history, runningBalance); // Get the log2 return so that we can use + instead of * later.
      if (!returnHistory.contains(transactions.at(i).date_time_.date())) {
        returnHistory.insert(transactions.at(i).date_time_.date(), log2_ROI);
      } else {
        // We should never have duplicated date since it's merged in the begining.
        qDebug() << "Duplicate date found!";
        qDebug() << investmentName << transactions.at(i).date_time_ << transactions.at(i).description_;
        system("pause");
      }
      local_transfer_history.clear();
      local_transfer_history << runningBalance;
    }

    gainOrLoss = Money(QDate(), Currency::USD, 0.00);
    balanceChange = Money(QDate(), Currency::USD, 0.00);
  }

  return_histories_.insert(investmentName, returnHistory);
  discount_rates_.insert(investmentName, qPow(2.0, backCalculateNPV(alltime_transfer_history, runningBalance) * 365));
}

double InvestmentAnalysis::backCalculateNPV(const QList<Money>& history, const Money& npv) {
  if (history.empty()) {
    return 0;  // This suppose never happen.
  }

  const double kMin = -1.0, kMax= 1.0, kTolerance = 1.0e-8;
  double min = kMin, max = kMax;
  double log2_dailyROI = 0.0;

  // Find out if the NPV increase while ROI increase?
  bool monotonic_increase = calculateNPV(history, 0.0, npv.date_) <
                            calculateNPV(history, 0.1, npv.date_);

  while (true) {
    Money temp_npv = calculateNPV(history, log2_dailyROI, npv.date_);

    // Return if found a accurate enough ROI.
    if (qFabs((temp_npv - npv).amount_) < kTolerance) {
      return log2_dailyROI;
    }

    if ((temp_npv < npv) xor monotonic_increase) {
      max = log2_dailyROI;
    } else {
      min = log2_dailyROI;
    }
    log2_dailyROI = (min + max) / 2;

    // Special handle when ROI converge but still not meet NPV requirement.
    // For example, Bitcoin has no previous transaction but the first one is recoreded as loss, which will resulted in a infinity ROI.
    if (qFabs(log2_dailyROI - kMin) < kTolerance or qFabs(log2_dailyROI - kMax) < kTolerance) {
      return log2_dailyROI;
    }
  }
}

// The NPV may monotonic increase or DECREASE with ROI.
// This is depends on how history is ordered and negative values inside.
Money InvestmentAnalysis::calculateNPV(const QList<Money>& history, double log2_dailyROI, const QDate& present) {
  Money ret(QDate(), Currency::USD, 0.00);
  for (Money money : history) {
    ret += money * (qPow(2.0, log2_dailyROI * money.date_.daysTo(present)));
  }
  return ret;
}

double InvestmentAnalysis::calculateAPR(const QMap<QDate, double>& returnHistory) {
  double log2_AROI = 0.0;
  QDate previousDate(1990, 05, 25);
  for (const QDate& date : returnHistory.keys()) {
    log2_AROI += previousDate.daysTo(date) * returnHistory.value(date);
    previousDate = date;
  }
  log2_AROI /= returnHistory.firstKey().daysTo(returnHistory.lastKey()) / 365.0;
  return qPow(2.0, log2_AROI);
}

void InvestmentAnalysis::on_investmentTableWidget_cellClicked(int row, int /* column */) {
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

void InvestmentAnalysis::on_startDateEdit_dateChanged(const QDate& /* date */) {
  plotInvestments();
}

void InvestmentAnalysis::on_axisX_rangeChanged(const QDateTime& start, const QDateTime& /* end */) {
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
      for (const QDate& date : return_histories_.value(investmentName).keys()) {
        if (date <= ui->startDateEdit->date()) {
          continue;
        }
        value += previousDate.daysTo(date) * return_histories_.value(investmentName).value(date);
        lineSeries->append(QDateTime(date, QTime(0, 0, 0)).toMSecsSinceEpoch(), value);

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
  ui->startDateEdit->setDateTime(book_.getFirstTransactionDateTime());
}
