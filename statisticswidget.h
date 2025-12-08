#ifndef STATISTICSWIDGET_H
#define STATISTICSWIDGET_H

#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QVBoxLayout>
#include <QLabel>
#include <QTextBrowser>

QT_CHARTS_USE_NAMESPACE

class StatisticsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StatisticsWidget(QWidget *parent = nullptr);
    ~StatisticsWidget();

    // Mise à jour des statistiques
    void updateStatistics(int totalResidents, int locataires, int proprietaires, 
                         int vehiclesWithCodes);
    void updateStatisticsServices(int totalServices, int activeServices);
    void updateStatisticsAppartements(int totalAppartements, int occupiedAppartements, 
                                      int emptyAppartements);

private:
    // Widgets
    QChartView *chartViewResidents;
    QChartView *chartViewServices;
    QChartView *chartViewAppartements;
    QTextBrowser *textBrowser;
    QLabel *titleLabel;
    
    // Méthodes pour créer les graphiques
    QChart* createResidentsChart(int total, int locataires, int proprietaires);
    QChart* createServicesChart(int total, int active);
    QChart* createAppartementsChart(int total, int occupied, int empty);
    
    // Données stockées
    int m_totalResidents;
    int m_locataires;
    int m_proprietaires;
    int m_vehiclesWithCodes;
    int m_totalServices;
    int m_activeServices;
    int m_totalAppartements;
    int m_occupiedAppartements;
    int m_emptyAppartements;
};

#endif // STATISTICSWIDGET_H
