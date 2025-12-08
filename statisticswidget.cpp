#include "statisticswidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFont>

StatisticsWidget::StatisticsWidget(QWidget *parent)
    : QWidget(parent),
      m_totalResidents(0), m_locataires(0), m_proprietaires(0),
      m_vehiclesWithCodes(0), m_totalServices(0), m_activeServices(0),
      m_totalAppartements(0), m_occupiedAppartements(0), m_emptyAppartements(0)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Titre principal
    titleLabel = new QLabel("STATISTIQUES");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);
    
    // Layout pour les graphiques circulaires
    QHBoxLayout *chartsLayout = new QHBoxLayout();
    
    // Graphique Résidents
    chartViewResidents = new QChartView();
    chartViewResidents->setRenderHint(QPainter::Antialiasing);
    chartsLayout->addWidget(chartViewResidents);
    
    // Graphique Services
    chartViewServices = new QChartView();
    chartViewServices->setRenderHint(QPainter::Antialiasing);
    chartsLayout->addWidget(chartViewServices);
    
    // Graphique Appartements
    chartViewAppartements = new QChartView();
    chartViewAppartements->setRenderHint(QPainter::Antialiasing);
    chartsLayout->addWidget(chartViewAppartements);
    
    mainLayout->addLayout(chartsLayout);
    
    // Text browser pour les détails
    textBrowser = new QTextBrowser();
    mainLayout->addWidget(textBrowser);
    
    setLayout(mainLayout);
}

StatisticsWidget::~StatisticsWidget()
{
}

void StatisticsWidget::updateStatistics(int totalResidents, int locataires, 
                                        int proprietaires, int vehiclesWithCodes)
{
    m_totalResidents = totalResidents;
    m_locataires = locataires;
    m_proprietaires = proprietaires;
    m_vehiclesWithCodes = vehiclesWithCodes;
    
    // Créer et afficher le graphique
    QChart *chart = createResidentsChart(totalResidents, locataires, proprietaires);
    chartViewResidents->setChart(chart);
    
    // Mettre à jour le texte
    QString html = "<h2>Statistiques Résidents</h2>";
    html += QString("<p><b>Total:</b> %1</p>").arg(totalResidents);
    html += QString("<p><b>Locataires:</b> %1</p>").arg(locataires);
    html += QString("<p><b>Propriétaires:</b> %1</p>").arg(proprietaires);
    html += QString("<p><b>Véhicules avec codes:</b> %1</p>").arg(vehiclesWithCodes);
    
    textBrowser->setHtml(html);
}

void StatisticsWidget::updateStatisticsServices(int totalServices, int activeServices)
{
    m_totalServices = totalServices;
    m_activeServices = activeServices;
    
    int inactiveServices = totalServices - activeServices;
    
    QChart *chart = createServicesChart(totalServices, activeServices);
    chartViewServices->setChart(chart);
}

void StatisticsWidget::updateStatisticsAppartements(int totalAppartements, 
                                                    int occupiedAppartements, 
                                                    int emptyAppartements)
{
    m_totalAppartements = totalAppartements;
    m_occupiedAppartements = occupiedAppartements;
    m_emptyAppartements = emptyAppartements;
    
    QChart *chart = createAppartementsChart(totalAppartements, occupiedAppartements, emptyAppartements);
    chartViewAppartements->setChart(chart);
}

QChart* StatisticsWidget::createResidentsChart(int total, int locataires, int proprietaires)
{
    QPieSeries *series = new QPieSeries();
    
    if (total > 0) {
        // Slice pour les locataires
        if (locataires > 0) {
            QPieSlice *slice1 = series->append("Locataires", locataires);
            slice1->setColor(QColor(76, 175, 80)); // Vert
            slice1->setLabelVisible(true);
            slice1->setLabelPosition(QPieSlice::LabelInsideHorizontal);
        }
        
        // Slice pour les propriétaires
        if (proprietaires > 0) {
            QPieSlice *slice2 = series->append("Propriétaires", proprietaires);
            slice2->setColor(QColor(33, 150, 243)); // Bleu
            slice2->setLabelVisible(true);
            slice2->setLabelPosition(QPieSlice::LabelInsideHorizontal);
        }
    } else {
        // Si aucun résident
        QPieSlice *slice = series->append("Aucun résident", 1);
        slice->setColor(QColor(200, 200, 200)); // Gris
        slice->setLabelVisible(true);
    }
    
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Distribution des Résidents");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    
    return chart;
}

QChart* StatisticsWidget::createServicesChart(int total, int active)
{
    QPieSeries *series = new QPieSeries();
    
    if (total > 0) {
        // Services actifs
        if (active > 0) {
            QPieSlice *slice1 = series->append("Actifs", active);
            slice1->setColor(QColor(76, 175, 80)); // Vert
            slice1->setLabelVisible(true);
            slice1->setLabelPosition(QPieSlice::LabelInsideHorizontal);
        }
        
        // Services inactifs
        int inactive = total - active;
        if (inactive > 0) {
            QPieSlice *slice2 = series->append("Inactifs", inactive);
            slice2->setColor(QColor(244, 67, 54)); // Rouge
            slice2->setLabelVisible(true);
            slice2->setLabelPosition(QPieSlice::LabelInsideHorizontal);
        }
    } else {
        QPieSlice *slice = series->append("Aucun service", 1);
        slice->setColor(QColor(200, 200, 200));
        slice->setLabelVisible(true);
    }
    
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("État des Services");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    
    return chart;
}

QChart* StatisticsWidget::createAppartementsChart(int total, int occupied, int empty)
{
    QPieSeries *series = new QPieSeries();
    
    if (total > 0) {
        // Appartements occupés
        if (occupied > 0) {
            QPieSlice *slice1 = series->append("Occupés", occupied);
            slice1->setColor(QColor(76, 175, 80)); // Vert
            slice1->setLabelVisible(true);
            slice1->setLabelPosition(QPieSlice::LabelInsideHorizontal);
        }
        
        // Appartements vides
        if (empty > 0) {
            QPieSlice *slice2 = series->append("Vides", empty);
            slice2->setColor(QColor(255, 193, 7)); // Orange
            slice2->setLabelVisible(true);
            slice2->setLabelPosition(QPieSlice::LabelInsideHorizontal);
        }
    } else {
        QPieSlice *slice = series->append("Aucun appartement", 1);
        slice->setColor(QColor(200, 200, 200));
        slice->setLabelVisible(true);
    }
    
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Occupation des Appartements");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    
    return chart;
}
