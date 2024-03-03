#include "investment_analyzer.h"


namespace {

QList<Transaction> aggregateTransactionByDate(const QList<Transaction>& transactions) {
    if (transactions.isEmpty()) {
        return transactions;
    }

    QList<Transaction> aggregated_transactions;
    Transaction sum;
    sum.date_time = transactions.front().date_time;
    for (const Transaction& transaction : transactions) {
        if (transaction.date_time.date() == sum.date_time.date()) {
            sum += transaction;
        } else {
            aggregated_transactions << sum;
            sum = transaction;
        }
    }
    aggregated_transactions << sum;

    return aggregated_transactions;
}

}  // namespace

void InvestmentAnalyzer::runAnalysis() {
    if (transactions_.isEmpty()) {
        return;
    }

    // Aggregate transactions into days.
    transactions_ = aggregateTransactionByDate(transactions_);

    // 2. Scan and analysis through all related transactions
    auto revenue = Account::create(-1, -1, Account::Revenue, "Investment", investment_.accountName(), "", investment_.currencyType(), false);
    // TODO: confirm can we get the account_id & category_id here.
    auto loan_account = Account::create(-1, -1, Account::Liability, "Loan", investment_.accountName(), "", investment_.currencyType(), false);
    return_history_.clear();
    QList<Money> local_transfer_history; // Store all the transfer activities since last summary.
    QList<Money> alltime_transfer_history;
    Money principal(QDate(), Currency::USD, 0.00);
    for (const Transaction& transaction : transactions_) {
        // Init the day before first transaction date and set log(ROI) to 0.
        if (return_history_.empty()) {
        return_history_.insert(transaction.date_time.date().addDays(-1), 0.00);
        }
        Money gain_or_loss = transaction.getHouseholdMoney(*revenue).sum().changeCurrency(Currency::USD);
        Money loan_change = transaction.getHouseholdMoney(*loan_account).sum().changeCurrency(Currency::USD);
        Money balance_change = transaction.getHouseholdMoney(investment_).sum().changeCurrency(Currency::USD);

        principal += balance_change - loan_change;
        local_transfer_history   << balance_change - loan_change - gain_or_loss;
        alltime_transfer_history << balance_change - loan_change - gain_or_loss;
        asset_history_.insert(transaction.date_time.date(), principal.amount_);

        // If has activity in revenue
        if (gain_or_loss.amount_ != 0.0) {
            double discount_rate = calculateIRR(local_transfer_history, principal); // log2(daily_discount_rate)
            // We should never have duplicated date since it's aggregated in the begining.
            return_history_.insert(transaction.date_time.date(), discount_rate);
//          local_transfer_history.clear();
//          local_transfer_history << principal;
        }
    }

    discount_rate_ = qPow(2.0, calculateIRR(alltime_transfer_history, principal) * 365);
}

// static
double InvestmentAnalyzer::calculateIRR(const QList<Money>& history, const Money& current_asset) {
  if (history.empty()) {
    return 0;  // This suppose never happen.
  }

  const double kMin = -1.0, kMax= 1.0, kTolerance = 1.0e-8;
  double min = kMin, max = kMax;
  double log2_rate = 0.0;

  // Find out if the NPV increase while ROI increase?
  bool monotonic_increase = calculateValueForDate(history, 0.0, current_asset.utcDate) <
                            calculateValueForDate(history, 0.01, current_asset.utcDate);

  while (true) {
    Money temp_npv = calculateValueForDate(history, log2_rate, current_asset.utcDate);

    // Return if found a accurate enough ROI.
    if (qFabs((temp_npv - current_asset).amount_) < kTolerance) {
      return log2_rate;
    }

    if ((temp_npv < current_asset) xor monotonic_increase) {
      max = log2_rate;
    } else {
      min = log2_rate;
    }
    log2_rate = (min + max) / 2;

    // Special handle when ROI converge but still not meet NPV requirement.
    // For example, Bitcoin has no previous transaction but the first one is recoreded as loss, which will resulted in a infinity ROI.
    if (qFabs(log2_rate - kMin) < kTolerance or qFabs(log2_rate - kMax) < kTolerance) {
      return log2_rate;
    }
  }
}

// static
// The NPV may monotonic increase or DECREASE with ROI.
// This is depends on how history is ordered and negative values inside.
// Can be Future Value (FV) or Net Present Value (NPV).
Money InvestmentAnalyzer::calculateValueForDate(const QList<Money>& history, double log2_rate, const QDate& date) {
    Money ret(QDate(), Currency::USD, 0.00);
    for (Money money : history) {
        ret += money * (qPow(2.0, log2_rate * money.utcDate.daysTo(date)));
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
