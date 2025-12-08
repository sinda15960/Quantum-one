#ifndef STATISTICSDIALOG_H
#define STATISTICSDIALOG_H

#include <QDialog>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
class StatisticsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StatisticsDialog(QWidget *parent = nullptr);
    ~StatisticsDialog();

private:
    void setupChart();
    void loadApartmentStatistics();

    QChartView *chartView;
    QChart *chart;
};

#endif // STATISTICSDIALOG_H
