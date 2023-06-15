#ifndef BAR_CHART_H
#define BAR_CHART_H

#include <QMainWindow>
#include <QtCharts>

class BarChart : public QMainWindow {
    Q_OBJECT
public:
    explicit BarChart(QWidget *parent = nullptr);

    void addBarSet(const QString& name, const QList<qreal>& data);
    void addLine(const QString& name, const QList<qreal>& data);
    void addBarSetToStackedBarSeries(const QString& name, const QList<qreal> &data);
    void setAxisX(const QStringList& m_axisX);
    void setTitle(const QString& title);

    void show();
    void showStackedBarChart();

signals:

public slots:

private:
    QBarSeries *bar_series_;
    QLineSeries *line_series_;
    QStackedBarSeries *stacked_bar_series_;

    QChart *chart_;
    QBarCategoryAxis *x_axis_;
    QValueAxis *y_axis_;
    QChartView *chart_view_;
};

#endif // BAR_CHART_H
