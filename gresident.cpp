#include "gresident.h"
#include "ui_gresident.h"
#include <QMessageBox>

GResident::GResident(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::GResident)
{
    ui->setupUi(this); // Load UI from Designer

    // ✅ Start on first page (index 0)
    ui->stackedWidget->setCurrentIndex(0);

    connect(ui->home, &QPushButton::clicked, this, [this]() {
        navigateToPage(0); // Go to page 1
    });

    // ✅ Connect your navigation buttons
    connect(ui->resident, &QPushButton::clicked, this, [this]() {
        navigateToPage(1); // Go to page 2
    });

    connect(ui->employee, &QPushButton::clicked, this, [this]() {
        navigateToPage(2); // Go to page 3
    });

    connect(ui->appartement, &QPushButton::clicked, this, [this]() {
        navigateToPage(3); // Go to page 4
    });

    connect(ui->vehicles, &QPushButton::clicked, this, [this]() {
        navigateToPage(4); // Go to page 5
    });

    connect(ui->services, &QPushButton::clicked, this, [this]() {
        navigateToPage(5); // Go to page 6
    });
}

GResident::~GResident()
{
    delete ui;
}

void GResident::navigateToPage(int pageIndex)
{
    ui->stackedWidget->setCurrentIndex(pageIndex);
}

// ------------------------------------------------------------
// Example placeholder logic for your resident actions
// ------------------------------------------------------------
void GResident::addResident()
{
    QMessageBox::information(this, "Ajout", "Résident ajouté avec succès !");
}

void GResident::deleteResident()
{
    QMessageBox::information(this, "Suppression", "Résident supprimé (simulation).");
}

void GResident::modifyResident()
{
    QMessageBox::information(this, "Modification", "Résident modifié (simulation).");
}

void GResident::displayResidents()
{
    QMessageBox::information(this, "Affichage", "Affichage des résidents (simulation).");
}
