#ifndef INVESTMENTANALYSIS_H
#define INVESTMENTANALYSIS_H

#include <QMainWindow>

#include <QtCharts>

#include "Money.h"
#include "Book.h"

namespace Ui {
class InvestmentAnalysis;
}

class InvestmentAnalysis : public QMainWindow {
  Q_OBJECT

public:
  explicit InvestmentAnalysis(std::shared_ptr<Book> book, QWidget *parent = nullptr);
  ~InvestmentAnalysis();

private slots:
  void on_investmentTableWidget_cellClicked(int row, int column);

  void on_startDateEdit_dateChanged(const QDate& date);
  void on_axisX_rangeChanged(const QDateTime& start, const QDateTime& end);

  void on_resetButton_clicked();

private:
  Ui::InvestmentAnalysis *ui;
  std::shared_ptr<Book> book_;

  QMap<QString, double> discount_rates_;  // Value: annual discount rate
  QMap<QString, QMap<QDate, double>> return_histories_;  // Key1: investment name, Value: (Key2: date, Value: log2(ROI) until date)

  void analysisInvestment(const QString& investmentName);

  // Returns the log2(daily_discount_rate) when reverse caluclate NPV.
  static double backCalculateNPV(const QList<Money>& history, const Money& npv);
  // Returns net present value.
  static Money calculateNPV(const QList<Money>& history, double log2_dailyROI, const QDate& present);
  static double calculateAPR(const QMap<QDate, double>& returnHistory);

  void plotInvestments();
};

#endif // INVESTMENTANALYSIS_H
