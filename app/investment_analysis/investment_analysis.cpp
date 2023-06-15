#include "investment_analysis.h"
#include "ui_investment_analysis.h"

#include "home_window/home_window.h"

InvestmentAnalysis::InvestmentAnalysis(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::InvestmentAnalysis),
      book_(static_cast<HomeWindow*>(parent)->book),
      user_id_(static_cast<HomeWindow*>(parent)->user_id) {
    ui->setupUi(this);

    // Scan, analysis and save all investment product:
    QList<AssetAccount> investments = book_.getInvestmentAccounts(user_id_);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    for (const AssetAccount& investment : investments) {
        InvestmentAnalyzer analyzer(investment,
                                    book_.queryTransactions(user_id_, TransactionFilter()
                                                                          .addAccount(QSharedPointer<Account>(new AssetAccount(investment)))
                                                                          .addAccount(Account::create(-1, -1, Account::Revenue, "Investment", investment.accountName()))
                                                                          .endTime(QDateTime::currentDateTime())
                                                                          .orderByAscending()
                                                                          .useOr()));
        analyzer.runAnalysis();
        investments_.insert(investment.accountName(), std::move(analyzer));
    }
    QApplication::restoreOverrideCursor();

  // Get all investment info into list:
  ui->investmentTableWidget->setRowCount(investments.size());
  for (int row = 0; row < investments.size(); row++) {
    const QString investment_name = investments.at(row).accountName();
    const auto& analyzer = investments_.value(investment_name); // TODO: no default ctor

    ui->investmentTableWidget->setRowHeight(row, 20);
    QTableWidgetItem* item = new QTableWidgetItem(investment_name);
    item->setCheckState(Qt::Unchecked);   // To Show the checkbox.
    ui->investmentTableWidget->setItem(row, 0, item);

    // Set column 1: Discount Rate
    double discount_rate = (analyzer.discountRate() - 1.0) * 100;
    QTableWidgetItem* discount_rate_item = new QTableWidgetItem(QString::number(discount_rate, 'f', 2) + "%");
    if (discount_rate < 0) {
      discount_rate_item->setForeground(Qt::red);
    }
    discount_rate_item->setTextAlignment(Qt::AlignRight);
    ui->investmentTableWidget->setItem(row, 1, discount_rate_item);

    // Set column 2: APR.
    double apr = 0;
//    apr = (InvestmentAnalyzer::calculateAPR(analyzer.getIrrHistory()) - 1.0) * 100;

    QTableWidgetItem* apr_item = new QTableWidgetItem(QString::number(apr, 'f', 2) + "%");
    if (apr < 0) {
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
      QLineSeries* line_series = new QLineSeries();
      QLineSeries* asset = new QLineSeries();
      line_series->setName(investmentName);

      double value = 0;
      QDate previousDate = ui->startDateEdit->date();
      line_series->append(ui->startDateEdit->dateTime().toMSecsSinceEpoch(), value);  // Add first data point as begin.
      for (const QDate& date : investments_.value(investmentName).getIrrHistory().keys()) {
        if (date <= ui->startDateEdit->date()) {
          continue;
        }
//        value += previousDate.daysTo(date) * investments_.value(investmentName).getIrrHistory().value(date);
        value = investments_.value(investmentName).getIrrHistory().value(date) * 365;
        line_series->append(QDateTime(date, QTime(0, 0, 0)).toMSecsSinceEpoch(), value);
        asset      ->append(QDateTime(date, QTime(0, 0, 0)).toMSecsSinceEpoch(), investments_.value(investmentName).getCashFlow().value(date));

        minY = qMin(value, minY);
        maxY = qMax(value, maxY);
        previousDate = date;
      }
      line_series->append(QDateTime::currentDateTime().toMSecsSinceEpoch(), value);  // Add last data point as current.

      QValueAxis *axisY2 = new QValueAxis;
      axisY->setLinePenColor(asset->pen().color());
      chart->addAxis(axisY2, Qt::AlignLeft);

      chart->addSeries(line_series);
      chart->addSeries(asset);
      // Attach axis must after chart->addSeries().
      line_series->attachAxis(axisX);
      line_series->attachAxis(axisY);
      asset->attachAxis(axisY2);
      asset->attachAxis(axisX);
    }
  }

  // Must connect this after everything done to avoid range change during append data, which is causeing recursive calling.
  QObject::connect(axisX, &QDateTimeAxis::rangeChanged, this, &InvestmentAnalysis::on_axisX_rangeChanged);

  double extra = (maxY - minY) * 0.05;
  axisY->setRange(qMax(minY - extra, qLn(0.5) / qLn(2)), qMin(maxY + extra, qLn(1.5) / qLn(2)));
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
