#include "investment_analyzer.h"

void InvestmentAnalyzer::runAnalysis() {
  if (transactions_.isEmpty()) {
    return;
  }

  // Aggregate transactions into days.
  QList<Transaction> aggregated_transactions;
  {
    Transaction sum;
    sum.date_time = transactions_.front().date_time;
    for (const Transaction& transaction : transactions_) {
      if (transaction.date_time.date() == sum.date_time.date()) {
        sum += transaction;
      } else {
        aggregated_transactions << sum;
        sum = transaction;
      }
    }
    aggregated_transactions << sum;
  }

  // 2. Scan and analysis through all related transactions
  Account revenue(Account::Revenue, "Investment", investment_.name);
  return_history_.clear();
  QList<Money> local_transfer_history; // Store all the transfer activities since last summary.
  QList<Money> alltime_transfer_history;
  Money running_balance(QDate(), Currency::USD, 0.00);
  for (const Transaction& transaction : aggregated_transactions) {
    // Init the day before first transaction date and set log(ROI) to 0.
    if (return_history_.empty()) {
      return_history_.insert(transaction.date_time.date().addDays(-1), 0.00);
    }

    Money gain_or_loss = transaction.getMoneyArray(revenue).sum().changeCurrency(Currency::USD);
    Money balance_change = transaction.getMoneyArray(investment_).sum().changeCurrency(Currency::USD);
    running_balance += balance_change;
    local_transfer_history   << balance_change - gain_or_loss;
    alltime_transfer_history << balance_change - gain_or_loss;

    // If has activity in revenue
    if (gain_or_loss.amount_ != 0.0) {
      double log2_ROI = backCalculateNPV(local_transfer_history, running_balance); // Get the log2 return so that we can use + instead of * later.
      // We should never have duplicated date since it's aggregated in the begining.
      return_history_.insert(transaction.date_time.date(), log2_ROI);
      local_transfer_history.clear();
      local_transfer_history << running_balance;
    }
  }

  discount_rate_ = qPow(2.0, backCalculateNPV(alltime_transfer_history, running_balance) * 365);
}

// static
double InvestmentAnalyzer::backCalculateNPV(const QList<Money>& history, const Money& npv) {
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

// static
// The NPV may monotonic increase or DECREASE with ROI.
// This is depends on how history is ordered and negative values inside.
Money InvestmentAnalyzer::calculateNPV(const QList<Money>& history, double log2_dailyROI, const QDate& present) {
  Money ret(QDate(), Currency::USD, 0.00);
  for (Money money : history) {
    ret += money * (qPow(2.0, log2_dailyROI * money.date_.daysTo(present)));
  }
  return ret;
}

// static
double InvestmentAnalyzer::calculateAPR(const QMap<QDate, double>& returnHistory) {
  double log2_AROI = 0.0;
  QDate previousDate(1990, 05, 25);
  for (const QDate& date : returnHistory.keys()) {
    log2_AROI += previousDate.daysTo(date) * returnHistory.value(date);
    previousDate = date;
  }
  log2_AROI /= returnHistory.firstKey().daysTo(returnHistory.lastKey()) / 365.0;
  return qPow(2.0, log2_AROI);
}
