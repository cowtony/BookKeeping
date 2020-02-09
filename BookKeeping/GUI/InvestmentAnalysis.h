#ifndef INVESTMENTANALYSIS_H
#define INVESTMENTANALYSIS_H

#include <QMainWindow>

#include <QtCharts>

#include "Money.h"

namespace Ui {
class InvestmentAnalysis;
}

class InvestmentAnalysis : public QMainWindow
{
  Q_OBJECT

public:
  explicit InvestmentAnalysis(QWidget *parent = nullptr);
  ~InvestmentAnalysis();

private slots:
  void on_investmentTableWidget_cellClicked(int row, int column);

  void on_startDateEdit_dateChanged(const QDate& date);
  void on_axisX_rangeChanged(const QDateTime& start, const QDateTime& end);

  void on_resetButton_clicked();

private:
  Ui::InvestmentAnalysis *ui;

  QMap<QString, QMap<QDate, double>> m_data;  // Key1: investment name, Value: (Key2: date, Value: log2(ROI) until date)

  void analysisInvestment(const QString& investmentName);
  static double getLog2DailyROI(const QList<Money>& history, const Money& gainOrLoss);

  static Money getNPV(const QList<Money>& history, double log2_dailyROI, const QDate& present);
  static Money calculateGainOrLoss(const QList<Money>& history, double log2_dailyROI, const QDate& present);
  double calculateAROI(const QString& investmentName) const;
  void plotInvestments();
};

#endif // INVESTMENTANALYSIS_H
