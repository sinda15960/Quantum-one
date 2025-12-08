#include "statisticsdialog.h"
#include <QVBoxLayout>
#include <QPainter>
#include <QColor>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QDebug>
#include <QLabel>
#include <QFont>

StatisticsDialog::StatisticsDialog(QWidget *parent)
    : QDialog(parent), chartView(nullptr), chart(nullptr)
{
    setWindowTitle("Statistics - Apartment Occupancy");
    setGeometry(100, 100, 800, 600);

    // Create the chart
    chart = new QChart();
    chart->setTitle("Apartment Occupancy Statistics");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // Create chart view
    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    // Load and display apartment statistics
    loadApartmentStatistics();

    // Create layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(chartView);
    setLayout(layout);
}

StatisticsDialog::~StatisticsDialog()
{
}

void StatisticsDialog::loadApartmentStatistics()
{
    QSqlQuery query(QSqlDatabase::database());

    // Get total apartments
    int total = 0;
    if (query.exec("SELECT COUNT(*) FROM APPARTEMENT") && query.next()) {
        total = query.value(0).toInt();
    }

    // Get vacant apartments (empty CIN or 'vacant' status)
    int vacant = 0;
    if (query.exec("SELECT COUNT(*) FROM APPARTEMENT WHERE (CIN IS NULL OR TRIM(CIN) = '' OR STATUS IS NULL OR TRIM(STATUS) = '' OR LOWER(STATUS) LIKE '%vac%')") && query.next()) {
        vacant = query.value(0).toInt();
    }

    int occupied = total - vacant;

    qDebug() << "[StatisticsDialog] Total:" << total << "Vacant:" << vacant << "Occupied:" << occupied;

    // Create bar series
    QBarSeries *series = new QBarSeries();

    // Create bar sets
    QBarSet *occupiedSet = new QBarSet("Occupied");
    *occupiedSet << occupied;
    occupiedSet->setColor(QColor(46, 204, 113)); // Green

    QBarSet *vacantSet = new QBarSet("Vacant");
    *vacantSet << vacant;
    vacantSet->setColor(QColor(231, 76, 60)); // Red

    series->append(occupiedSet);
    series->append(vacantSet);

    // Add series to chart
    chart->addSeries(series);

    // Create axes
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append("Apartments");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, total > 0 ? total + 5 : 10);
    axisY->setLabelFormat("%i");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // Add legend
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
}
