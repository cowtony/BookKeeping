#ifndef INVESTMENTANALYSIS_H
#define INVESTMENTANALYSIS_H

#include <QMainWindow>

#include <QtCharts>

#include "book/book.h"
#include "investment_analyzer.h"

namespace Ui {
class InvestmentAnalysis;
}

class InvestmentAnalysis : public QMainWindow {
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
    void plotInvestments();

    Ui::InvestmentAnalysis *ui;
    Book& book_;

    QMap<QString, InvestmentAnalyzer> investments_;
};

#endif // INVESTMENTANALYSIS_H
