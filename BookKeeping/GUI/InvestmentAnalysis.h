#ifndef INVESTMENTANALYSIS_H
#define INVESTMENTANALYSIS_H

#include <QMainWindow>

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

private:
  Ui::InvestmentAnalysis *ui;

  QMap<QString, QMap<QDate, double>> m_data;  // Key1: investment name, Value: (Key2: date, Value: log2(ROI) until date)

  void analysisInvestment(const QString& investmentName);
  static double getLog2DailyROI(const QList<Money>& history, const Money& gainOrLoss);
  static Money calculateGainOrLoss(const QList<Money>& history, const double& log2_dailyROI, const QDate& date);
  double calculateAROI(const QString& investmentName) const;
  void plotInvestments(const QStringList& investments);
};

#endif // INVESTMENTANALYSIS_H