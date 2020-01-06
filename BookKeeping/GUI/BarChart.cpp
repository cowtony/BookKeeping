#include "BarChart.h"

BarChart::BarChart(QWidget *parent) : QMainWindow(parent)
{
    m_barSeries = new QBarSeries();

    m_lineSeries = new QLineSeries();
//    m_lineSeries->setColor(QColor(0, 0, 0));

    m_stackedBarSeries = new QStackedBarSeries();

    m_chart = new QChart();
    m_chart->setAnimationOptions(QChart::SeriesAnimations);
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);

    m_axisX = new QBarCategoryAxis();
    m_axisX->setLabelsFont(QFont("Time New Roman", 8, 1, false));
    m_axisX->setLabelsAngle(270);
    m_axisX->setTitleText("Summary by month");

    m_axisY = new QValueAxis();
    m_axisY->setTitleText("US Dollar");

    m_chartView = new QChartView();
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setRubberBand(QChartView::HorizontalRubberBand);
}

BarChart::~BarChart()
{
}

void BarChart::addBarSet(const QString &name, const QList<qreal> &data)
{
    QBarSet *barSet = new QBarSet(name);
    barSet->append(data);
    m_barSeries->append(barSet);
}

void BarChart::addLine(const QString &name, const QList<qreal> &data)
{
    m_lineSeries->clear();
    m_lineSeries->setName(name);
    for (int i = 0; i < data.size(); i++)
        m_lineSeries->append(i, data.at(i));
}

void BarChart::addBarSetToStackedBarSeries(const QString &name, const QList<qreal> &data)
{
    QBarSet *barSet = new QBarSet(name);
    barSet->append(data);
    m_stackedBarSeries->append(barSet);
}

void BarChart::setAxisX(const QStringList &p_axisX)
{
    m_axisX->append(p_axisX);
}

void BarChart::setTitle(const QString &title)
{
    m_chart->setTitle(title);
}

void BarChart::show()
{
    m_chart->addSeries(m_barSeries);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_barSeries->attachAxis(m_axisX);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_barSeries->attachAxis(m_axisY);
    m_chartView->setChart(m_chart);
    setCentralWidget(m_chartView);
    resize(1280, 600);

    QMainWindow::show();
}

void BarChart::showStackedBarChart()
{
    m_chart->addSeries(m_stackedBarSeries);
    m_chart->addSeries(m_lineSeries);

    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_stackedBarSeries->attachAxis(m_axisX);
    m_lineSeries->attachAxis(m_axisX);

    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_stackedBarSeries->attachAxis(m_axisY);
    m_lineSeries->attachAxis(m_axisY);

    m_chartView->setChart(m_chart);
    setCentralWidget(m_chartView);
    resize(1280, 600);

    QMainWindow::show();
}
