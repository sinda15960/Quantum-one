#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPushButton>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Initialisation : Le bouton de retour est caché sur la page principale.
    ui->btnback->hide();

    // 1. Définir la page principale (Index 0) comme vue initiale
    // NOTE : VOTRE UI N'A QUE DEUX PAGES. ASSUREZ-VOUS D'EN CRÉER 5 DANS QT DESIGNER !
    ui->stackedWidget->setCurrentIndex(0);

    // 2. Connexion manuelle du bouton 'Back' qui est unique dans le centralwidget
    connect(ui->btnback, &QPushButton::clicked, this, &MainWindow::on_btnback_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ------------------------------------------------------------------
// SLOTS pour les 4 boutons principaux (Navigation vers les formulaires)
// ------------------------------------------------------------------

void MainWindow::showActionPage(int index)
{
    // Affiche la page d'action demandée
    ui->stackedWidget->setCurrentIndex(index);
    // Affiche le bouton 'Back'
    ui->btnback->show();
}

void MainWindow::on_btn1_clicked() // Ajouter -> Page 1
{
    showActionPage(1);
}

void MainWindow::on_btn2_clicked() // Afficher -> Page 2
{
    showActionPage(2);
}

void MainWindow::on_btn3_clicked() // Supprimer -> Page 3
{
    showActionPage(3);
}

void MainWindow::on_btn4_clicked() // Modifier -> Page 4
{
    showActionPage(4);
}

// ------------------------------------------------------------------
// SLOT pour le bouton de retour (Navigation vers la page principale)
// ------------------------------------------------------------------

void MainWindow::on_btnback_clicked()
{
    // Revenir à la vue principale (Index 0)
    ui->stackedWidget->setCurrentIndex(0);
    // Cache le bouton 'Back'
    ui->btnback->hide();
}
