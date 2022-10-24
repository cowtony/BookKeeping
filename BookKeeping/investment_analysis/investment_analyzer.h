#ifndef INVESTMENTANALYZER_H
#define INVESTMENTANALYZER_H

#include "account.h"
#include "money.h"
#include"transaction.h"

class InvestmentAnalyzer {
public:
  InvestmentAnalyzer() : investment_(Account::Asset, "", "") {}
  InvestmentAnalyzer(const AssetAccount& investment, const QList<Transaction>& transactions)
    : investment_(investment), transactions_(transactions) {}

  void runAnalysis();
  double discountRate() const { return discount_rate_; }

  static double calculateAPR(const QMap<QDate, double>& returnHistory);

private:
  // Returns the log2(daily_discount_rate) when reverse caluclate NPV.
  static double backCalculateNPV(const QList<Money>& history, const Money& npv);
  // Returns net present value.
  static Money calculateNPV(const QList<Money>& history, double log2_dailyROI, const QDate& present);

  AssetAccount investment_;
  QList<Transaction> transactions_;

  QList<Money> history_;
  double discount_rate_;
public:
  QMap<QDate, double> return_history_;  // Key: date, Value: log2(daily return) until date.
};

#endif // INVESTMENTANALYZER_H
