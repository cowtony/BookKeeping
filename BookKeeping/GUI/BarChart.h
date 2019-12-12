#ifndef BARCHART_H
#define BARCHART_H

#include <QMainWindow>
#include <QtCharts>

class BarChart : public QMainWindow
{
    Q_OBJECT
public:
    explicit BarChart(QWidget *parent = nullptr);
    ~BarChart();
    void addBarSet(const QString &name, const QList<qreal> &data);
    void addLine(const QString &name, const QList<qreal> &data);
    void addBarSetToStackedBarSeries(const QString &name, const QList<qreal> &data);
    void setAxisX(const QStringList &m_axisX);
    void setTitle(const QString &title);

    void show();
    void showStackedBarChart();

signals:

public slots:

private:
    QBarSeries *m_barSeries;
    QLineSeries *m_lineSeries;
    QStackedBarSeries *m_stackedBarSeries;

    QChart *m_chart;
    QBarCategoryAxis *m_axisX;
    QValueAxis *m_axisY;
    QChartView *chartView;
};

#endif // BARCHART_H
