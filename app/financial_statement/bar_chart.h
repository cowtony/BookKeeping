#ifndef BAR_CHART_H
#define BAR_CHART_H

#include <QMainWindow>
#include <QtCharts>

class BarChart : public QMainWindow {
    Q_OBJECT
public:
    explicit BarChart(QWidget *parent = nullptr);

    void setAxisX(const QStringList& m_axisX);
    void setTitle(const QString& title);

    void addBarSet(const QString& name, const QList<qreal>& data);
    void addBarSeries();

    void addLine(const QString& name, const QList<qreal>& data);

    void addBarSetToStackedBarSeries(const QString& name, const QList<qreal> &data);
    void addStackedBarSeries();

    void show();

signals:

public slots:

private:
    QBarSeries *bar_series_;
    QStackedBarSeries *stacked_bar_series_;

    QBarCategoryAxis *x_axis_;
    QChart *chart_;
    QChartView *chart_view_;
};

#endif // BAR_CHART_H
