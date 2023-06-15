#include "bar_chart.h"

BarChart::BarChart(QWidget *parent) : QMainWindow(parent) {
    bar_series_ = new QBarSeries();

    line_series_ = new QLineSeries();
    //    m_lineSeries->setColor(QColor(0, 0, 0));

    stacked_bar_series_ = new QStackedBarSeries();

    chart_ = new QChart();
    chart_->setAnimationOptions(QChart::SeriesAnimations);
    chart_->legend()->setVisible(true);
    chart_->legend()->setAlignment(Qt::AlignBottom);

    x_axis_ = new QBarCategoryAxis();
    x_axis_->setLabelsFont(QFont("Time New Roman", 8, 1, false));
    x_axis_->setLabelsAngle(270);
    x_axis_->setTitleText("Summary by month");

    y_axis_ = new QValueAxis();
    y_axis_->setTitleText("US Dollar");
    y_axis_->setLabelFormat("$%d");

    chart_view_ = new QChartView();
    chart_view_->setRenderHint(QPainter::Antialiasing);
    chart_view_->setRubberBand(QChartView::HorizontalRubberBand);
}

void BarChart::addBarSet(const QString &name, const QList<qreal> &data) {
    QBarSet *barSet = new QBarSet(name);
    barSet->append(data);
    bar_series_->append(barSet);
}

void BarChart::addLine(const QString &name, const QList<qreal> &data) {
    line_series_->clear();
    line_series_->setName(name);
    for (int i = 0; i < data.size(); i++)
        line_series_->append(i, data.at(i));
}

void BarChart::addBarSetToStackedBarSeries(const QString &name, const QList<qreal> &data) {
    QBarSet *barSet = new QBarSet(name);
    barSet->append(data);
    stacked_bar_series_->append(barSet);
}

void BarChart::setAxisX(const QStringList &p_axisX) {
    x_axis_->append(p_axisX);
}

void BarChart::setTitle(const QString &title) {
    chart_->setTitle(title);
}

void BarChart::show() {
    chart_->addSeries(bar_series_);
    chart_->addAxis(x_axis_, Qt::AlignBottom);
    bar_series_->attachAxis(x_axis_);
    chart_->addAxis(y_axis_, Qt::AlignLeft);
    bar_series_->attachAxis(y_axis_);
    y_axis_->applyNiceNumbers();
    chart_view_->setChart(chart_);
    setCentralWidget(chart_view_);
    resize(1280, 600);

    QMainWindow::show();
}

void BarChart::showStackedBarChart()
{
    chart_->addSeries(stacked_bar_series_);
    chart_->addSeries(line_series_);

    chart_->addAxis(x_axis_, Qt::AlignBottom);
    stacked_bar_series_->attachAxis(x_axis_);
    line_series_->attachAxis(x_axis_);

    chart_->addAxis(y_axis_, Qt::AlignLeft);
    stacked_bar_series_->attachAxis(y_axis_);
    line_series_->attachAxis(y_axis_);
    y_axis_->applyNiceNumbers();

    chart_view_->setChart(chart_);
    setCentralWidget(chart_view_);
    resize(1280, 600);

    QMainWindow::show();
}
