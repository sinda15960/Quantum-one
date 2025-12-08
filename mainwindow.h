#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlQuery>
#include <QList>
#include <QGraphicsScene>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct StatResident {
    QString categorie;
    int count;
    double percentage;
    StatResident() : categorie(""), count(0), percentage(0.0) {}
    StatResident(QString c, int ct, double p) : categorie(c), count(ct), percentage(p) {}
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void refreshTable(const QString &value = "", const QString &type = "");
    // CRUD et recherche
    void addResident();
    void updateResident();
    void deleteResident();
    void searchResident();
    void findResident();
    // Export
    void exportResidentContractPDF();
    void exportResidentContractPDF2();
    void residentFile();
    void exportResidentListToExcel();
    // SMS
    void sendResidentSMS();
    // Statistiques
    void showResidentStatistics();
    void calculerStatistiquesResidents();
    void afficherStatistiquesCirculaire();
    void dessinerDiagrammeCirculaire(QGraphicsScene *scene, const QList<StatResident> &stats, const QRectF &zone);
    // sort
    void sortResidents();

private:
    Ui::MainWindow *ui;
    // Données pour statistiques
    QList<StatResident> statsResidents;
};

#endif // MAINWINDOW_H
