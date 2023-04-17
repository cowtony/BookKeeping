#ifndef INVESTMENTANALYZER_H
#define INVESTMENTANALYZER_H

#include "book/account.h"
#include "book/money.h"
#include "book/transaction.h"

class InvestmentAnalyzer {
public:
  InvestmentAnalyzer() : investment_(Account::Asset, "", "") {}
  InvestmentAnalyzer(const AssetAccount& investment, const QList<Transaction>& transactions)
    : investment_(investment), transactions_(transactions) {}

  void runAnalysis();


  static double calculateAPR(const QMap<QDate, double>& returnHistory);

  double discountRate() const { return discount_rate_; }
  const QMap<QDate, double>& getIrrHistory() const { return return_history_; }
  const QMap<QDate, double>& getCashFlow() const { return asset_history_; }

private:
  // Returns the log2(daily_discount_rate) when reverse caluclate NPV.
  static double calculateIRR(const QList<Money>& history, const Money& npv);
  // Returns net present value.
  static Money calculateValueForDate(const QList<Money>& history, double log2_dailyROI, const QDate& present);

  AssetAccount investment_;
  QList<Transaction> transactions_;

  QList<Money> history_;
  double discount_rate_;

  QMap<QDate, double> return_history_;  // <date, log2(daily return)> until current date.
  QMap<QDate, double> asset_history_;
};

#endif // INVESTMENTANALYZER_H
