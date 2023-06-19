#include "bar_chart.h"

BarChart::BarChart(QWidget *parent) : QMainWindow(parent) {
    bar_series_ = new QBarSeries();

    stacked_bar_series_ = new QStackedBarSeries();

    chart_ = new QChart();
    chart_->setAnimationOptions(QChart::SeriesAnimations);
    chart_->legend()->setVisible(true);
    chart_->legend()->setAlignment(Qt::AlignBottom);

    x_axis_ = new QBarCategoryAxis();
    x_axis_->setLabelsFont(QFont("Time New Roman", 8, 1, false));
    x_axis_->setLabelsAngle(270);
    x_axis_->setTitleText("Summary by month");

    chart_view_ = new QChartView();
    chart_view_->setRenderHint(QPainter::Antialiasing);
    chart_view_->setRubberBand(QChartView::HorizontalRubberBand);
    setCentralWidget(chart_view_);
    resize(1280, 600);
}

void BarChart::addBarSet(const QString &name, const QList<qreal> &data) {
    QBarSet *barSet = new QBarSet(name);
    barSet->append(data);
    bar_series_->append(barSet);
}

void BarChart::addLine(const QString &name, const QList<qreal> &data) {
    QLineSeries* line_series = new QLineSeries();
    line_series->setName(name);
    for (int i = 0; i < data.size(); i++) {
        line_series->append(i, data.at(i));
    }
    chart_->addSeries(line_series);
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

void BarChart::addBarSeries() {
    chart_->addSeries(bar_series_);
}

void BarChart::addStackedBarSeries() {
    chart_->addSeries(stacked_bar_series_);
}

void BarChart::show() {
    // Create auto scaled axis, mainly for y-axis.
    chart_->createDefaultAxes();
    // Delete the auto created x-axis.
    for (auto* x_axis : chart_->axes(Qt::Horizontal)) {
        chart_->removeAxis(x_axis);
    }

    // Use the own x-axis.
    chart_->addAxis(x_axis_, Qt::AlignBottom);
    for (auto* series : chart_->series()) {
        series->attachAxis(x_axis_);
    }

    // Fine tuning the y-axis.
    QValueAxis* y_axis = qobject_cast<QValueAxis *>(chart_->axes(Qt::Vertical).first());
    y_axis->setTitleText("US Dollar");
    y_axis->setLabelFormat("$%d");
    y_axis->applyNiceNumbers();

    chart_view_->setChart(chart_);
    QMainWindow::show();
}


