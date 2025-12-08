#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "residents.h"
#include "connection.h"
#include <QMessageBox>
#include <QPrinter>
#include <QTextDocument>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>
#include <QFile>
#include <QTextStream>
#include <QSqlQuery>
#include <QSqlError>
#include <QPushButton>
#include <QLineEdit>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsPathItem>
#include <QGraphicsDropShadowEffect>
#include <QtMath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    Connection c;
    if (!c.createconnect()) {
        QMessageBox::critical(this, "Erreur", "Impossible de se connecter à la base de données.");
    }

    refreshTable();

    // Connexions
    connect(ui->addButton, &QPushButton::clicked, this, &MainWindow::addResident);
    connect(ui->updateButton, &QPushButton::clicked, this, &MainWindow::updateResident);
    connect(ui->deleteButton, &QPushButton::clicked, this, &MainWindow::deleteResident);
    connect(ui->searchButton, &QPushButton::clicked, this, &MainWindow::searchResident);
    connect(ui->findButton, &QPushButton::clicked, this, &MainWindow::findResident);
    connect(ui->SMSButton, &QPushButton::clicked, this, &MainWindow::sendResidentSMS);
    connect(ui->exportPDFButton, &QPushButton::clicked, this, &MainWindow::exportResidentContractPDF);
    connect(ui->exportPDF2Button, &QPushButton::clicked, this, &MainWindow::exportResidentContractPDF2);
    connect(ui->residentFileButton, &QPushButton::clicked, this, &MainWindow::residentFile);
    connect(ui->residentListButton, &QPushButton::clicked, this, &MainWindow::exportResidentListToExcel);
    connect(ui->statistics_pushButton, &QPushButton::clicked, this, &MainWindow::showResidentStatistics);
    connect(ui->lineEdit_63, &QLineEdit::textChanged, this, [=]() {
        QString value = ui->lineEdit_63->text();
        QString type = ui->sortComboBox->currentText().trimmed().toUpper();
        refreshTable(value, type);
    });
    connect(ui->sortButton, &QPushButton::clicked, this, &MainWindow::sortResidents);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refreshTable(const QString &value, const QString &type)
{
    Residents r;
    ui->tableView->setModel(r.afficher(value, type));
}

// CRUD et recherches
void MainWindow::addResident()
{
    Residents r(ui->LineEdit_cin_add->text(), ui->LineEdit_nom_add->text(),
                ui->LineEdit_prenom_add->text(), ui->LineEdit_tel_add->text());
    if (r.ajouter()) {
        QMessageBox::information(this, "Succès", "Résident ajouté !");
        refreshTable();
    } else {
        QMessageBox::warning(this, "Erreur", "Ajout échoué !");
    }
}

void MainWindow::updateResident()
{
    Residents r(ui->LineEdit_cin_update->text(), ui->LineEdit_nom_update->text(),
                ui->LineEdit_prenom_update->text(), ui->LineEdit_tel_update->text());
    if (r.modifier()) {
        QMessageBox::information(this, "Succès", "Modification réussie !");
        refreshTable();
    } else {
        QMessageBox::warning(this, "Erreur", "Échec de modification !");
    }
}

void MainWindow::deleteResident()
{
    Residents r;
    r.setCin(ui->LineEdit_cin_delete->text());
    if (r.supprimer()) {
        QMessageBox::information(this, "Succès", "Résident supprimé !");
        refreshTable();
    } else {
        QMessageBox::warning(this, "Erreur", "Suppression échouée !");
    }
}

void MainWindow::searchResident()
{
    QString cin = ui->LineEdit_cin_update->text();
    Residents r;
    if (r.search(cin)) {
        ui->LineEdit_nom_update->setText(r.getNom());
        ui->LineEdit_prenom_update->setText(r.getPrenom());
        ui->LineEdit_tel_update->setText(r.getTelephone());
        QMessageBox::information(this, "Trouvé", "Résident trouvé !");
    } else {
        QMessageBox::warning(this, "Non trouvé", "Aucun résident !");
    }
}

void MainWindow::findResident()
{
    QString value = ui->lineEdit_63->text();
    QString type = ui->sortComboBox->currentText().trimmed().toUpper();
    refreshTable(value, type);
}

// Export PDF
void MainWindow::exportResidentContractPDF()
{
    QString cin = ui->LineEdit_cin_add->text();
    QString nom = ui->LineEdit_nom_add->text();
    QString prenom = ui->LineEdit_prenom_add->text();
    QString telephone = ui->LineEdit_tel_add->text();
    if (cin.isEmpty()) return;
    QString fileName = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/Resident_Contract_" + cin + ".pdf";
    QTextDocument document;
    document.setHtml("<h2>Dossier résident</h2>"
                     "<p><b>Nom :</b> " + nom + " " + prenom + "</p>"
                                            "<p><b>CIN :</b> " + cin + "</p>"
                             "<p><b>Téléphone :</b> " + telephone + "</p>");
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    document.print(&printer);
    QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}

void MainWindow::exportResidentContractPDF2()
{
    QString cin = ui->LineEdit_cin_update->text();
    QString nom = ui->LineEdit_nom_update->text();
    QString prenom = ui->LineEdit_prenom_update->text();
    QString telephone = ui->LineEdit_tel_update->text();
    if (cin.isEmpty()) return;
    QString fileName = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/Resident_Updated_" + cin + ".pdf";
    QTextDocument document;
    document.setHtml("<h2>Dossier résident (Après Modification)</h2>"
                     "<p><b>Nom :</b> " + nom + " " + prenom + "</p>"
                                            "<p><b>CIN :</b> " + cin + "</p>"
                             "<p><b>Téléphone :</b> " + telephone + "</p>");
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    document.print(&printer);
    QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
    QMessageBox::information(this, "PDF Généré", "PDF du résident modifié généré sur le bureau !");
}

// Export TXT
void MainWindow::residentFile()
{
    QString cin = ui->LineEdit_cin_add->text();
    QString nom = ui->LineEdit_nom_add->text();
    QString prenom = ui->LineEdit_prenom_add->text();
    QString telephone = ui->LineEdit_tel_add->text();
    if (cin.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "CIN manquant !");
        return;
    }
    QString fileName = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/ResidentFile_" + cin + ".txt";
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Erreur", "Impossible de créer le fichier !");
        return;
    }
    QTextStream out(&file);
    out << "=== Suivi du dossier résident ===\n";
    out << "CIN : " << cin << "\n";
    out << "Nom : " << nom << "\n";
    out << "Prénom : " << prenom << "\n";
    out << "Téléphone : " << telephone << "\n\n";

    // Appartements
    out << "--- Historique des appartements ---\n";
    QSqlQuery q1;
    q1.prepare("SELECT NUM, ETAGE, TYPE FROM APPARTEMENTS WHERE CIN = :cin ORDER BY NUM");
    q1.bindValue(":cin", cin);
    if (!q1.exec()) out << "Erreur récupération appartements : " << q1.lastError().text() << "\n";
    else {
        bool any = false;
        while (q1.next()) {
            any = true;
            out << "Num: " << q1.value(0).toString() << ", Etage: " << q1.value(1).toString() << ", Type: " << q1.value(2).toString() << "\n";
        }
        if (!any) out << "Aucun appartement.\n";
    }

    // Véhicules
    out << "\n--- Véhicules associés ---\n";
    QSqlQuery q2;
    q2.prepare("SELECT MATRICULE, MARQUE, MODELE FROM VEHICULES WHERE CIN = :cin");
    q2.bindValue(":cin", cin);
    if (!q2.exec()) out << "Erreur récupération véhicules : " << q2.lastError().text() << "\n";
    else {
        bool any = false;
        while (q2.next()) {
            any = true;
            out << "Matricule: " << q2.value(0).toString() << ", Marque: " << q2.value(1).toString() << ", Modele: " << q2.value(2).toString() << "\n";
        }
        if (!any) out << "Aucun véhicule.\n";
    }

    // Services
    out << "\n--- Services souscrits ---\n";
    QSqlQuery q3;
    bool prepared3 = q3.prepare("SELECT ID_SERVICE, TYPE, \"DATE\" FROM SERVICES WHERE CIN = :cin ORDER BY \"DATE\"");
    if (!prepared3) q3.prepare("SELECT ID_SERVICE, TYPE, DATE FROM SERVICES WHERE CIN = :cin ORDER BY DATE");
    q3.bindValue(":cin", cin);
    if (!q3.exec()) out << "Erreur récupération services : " << q3.lastError().text() << "\n";
    else {
        bool any = false;
        while (q3.next()) {
            any = true;
            out << "ID: " << q3.value(0).toString() << ", Type: " << q3.value(1).toString() << ", Date: " << q3.value(2).toString() << "\n";
        }
        if (!any) out << "Aucun service.\n";
    }
    file.close();
    QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}

// SMS
void MainWindow::sendResidentSMS()
{
    QString cin = ui->LineEdit_cin_add->text();
    QString nom = ui->LineEdit_nom_add->text();
    QString prenom = ui->LineEdit_prenom_add->text();
    QString telephone = ui->LineEdit_tel_add->text();
    if (cin.isEmpty() || nom.isEmpty() || prenom.isEmpty() || telephone.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez remplir tous les champs !");
        return;
    }

    QString sms;
    sms += "📩 SMS AUTOMATIQUE\n";
    sms += "Bonjour " + nom + " " + prenom + ",\n";
    sms += "Votre dossier a été mis à jour.\n\n";
    sms += "📌 CIN: " + cin + "\n";
    sms += "📞 Tel: " + telephone + "\n\n";

    // Appartements
    sms += "🏠 APPARTEMENTS :\n";
    QSqlQuery q1;
    q1.prepare("SELECT NUM, ETAGE, TYPE FROM APPARTEMENTS WHERE CIN = :cin ORDER BY NUM");
    q1.bindValue(":cin", cin);
    if (!q1.exec()) sms += " ⚠ Erreur récupération appartements.\n";
    else {
        bool any = false;
        while (q1.next()) {
            any = true;
            sms += " • Num: " + q1.value(0).toString() + " | Etage: " + q1.value(1).toString() + " | Type: " + q1.value(2).toString() + "\n";
        }
        if (!any) sms += " Aucun appartement.\n";
    }

    // Véhicules
    sms += "\n🚗 VÉHICULES :\n";
    QSqlQuery q2;
    q2.prepare("SELECT MATRICULE, MARQUE, MODELE FROM VEHICULES WHERE CIN = :cin");
    q2.bindValue(":cin", cin);
    if (!q2.exec()) sms += " ⚠ Erreur récupération véhicules.\n";
    else {
        bool any = false;
        while (q2.next()) {
            any = true;
            sms += " • " + q2.value(0).toString() + " | " + q2.value(1).toString() + " | " + q2.value(2).toString() + "\n";
        }
        if (!any) sms += " Aucun véhicule.\n";
    }

    // Services
    sms += "\n🛎 SERVICES :\n";
    QSqlQuery q3;
    bool prepared = q3.prepare("SELECT ID_SERVICE, TYPE, \"DATE\" FROM SERVICES WHERE CIN = :cin ORDER BY \"DATE\"");
    if (!prepared) q3.prepare("SELECT ID_SERVICE, TYPE, DATE FROM SERVICES WHERE CIN = :cin ORDER BY DATE");
    q3.bindValue(":cin", cin);
    if (!q3.exec()) sms += " ⚠ Erreur récupération services.\n";
    else {
        bool any = false;
        while (q3.next()) {
            any = true;
            sms += " • ID: " + q3.value(0).toString() + " | " + q3.value(1).toString() + " | Date: " + q3.value(2).toString() + "\n";
        }
        if (!any) sms += " Aucun service.\n";
    }
    QMessageBox::information(this, "SMS Résident", sms);
}

// Export CSV
void MainWindow::exportResidentListToExcel()
{
    QSqlQuery q;
    if (!q.exec("SELECT CIN, NOM, PRENOM, TELEPHONE FROM RESIDENTS ORDER BY CIN"))
        return;

    QString fileName = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/ResidentList.csv";
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out << "CIN,Nom,Prénom,Téléphone\n";
    while (q.next()) {
        out << q.value(0).toString() << "," << q.value(1).toString() << "," << q.value(2).toString() << "," << q.value(3).toString() << "\n";
    }
    file.close();
    QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}

// === Statistiques Résidents ===
void MainWindow::showResidentStatistics()
{
    qDebug() << "📊 Calcul des statistiques des résidents...";
    calculerStatistiquesResidents();
    afficherStatistiquesCirculaire();
    QMessageBox::information(this, "Statistiques Résidents", "Statistiques actualisées avec succès!");
}

void MainWindow::calculerStatistiquesResidents()
{
    statsResidents.clear();
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << "Base de données non connectée";
        QMessageBox::critical(this, "Erreur", "Connexion à la base de données échouée!");
        return;
    }

    QSqlQuery query(db);
    int totalResidents = 0;
    if (query.exec("SELECT COUNT(*) FROM RESIDENTS") && query.next()) {
        totalResidents = query.value(0).toInt();
    }

    const int maxResidents = 200;
    if (totalResidents == 0) {
        qDebug() << "Aucun résident trouvé pour les statistiques";
        statsResidents.append(StatResident("Aucun résident", 0, 0.0));
        return;
    }

    query.prepare("SELECT UPPER(SUBSTR(NOM,1,1)) as lettre, COUNT(*) "
                  "FROM RESIDENTS GROUP BY UPPER(SUBSTR(NOM,1,1)) "
                  "ORDER BY COUNT(*) DESC");
    if (query.exec()) {
        while (query.next()) {
            QString lettre = query.value(0).toString();
            int count = query.value(1).toInt();
            double percentage = (count * 100.0) / maxResidents;
            if (percentage > 100.0) percentage = 100.0;
            QString categorie = QString("Noms commençant par '%1'").arg(lettre);
            statsResidents.append(StatResident(categorie, count, percentage));
            qDebug() << "📈 Statistique -" << categorie << ":" << count << "résidents (" << percentage << "% par rapport à 200)";
        }
    } else {
        qDebug() << "❌ Erreur calcul statistiques:" << query.lastError().text();
        statsResidents.append(StatResident("Total résidents", totalResidents, (totalResidents * 100.0) / maxResidents));
    }

    if (statsResidents.size() > 8) {
        QList<StatResident> statsFiltrees;
        int autresCount = 0;
        double autresPercentage = 0.0;
        for (int i = 0; i < statsResidents.size(); ++i) {
            if (i < 7) statsFiltrees.append(statsResidents[i]);
            else {
                autresCount += statsResidents[i].count;
                autresPercentage += statsResidents[i].percentage;
            }
        }
        if (autresCount > 0) statsFiltrees.append(StatResident("Autres", autresCount, autresPercentage));
        statsResidents = statsFiltrees;
    }

    if (statsResidents.isEmpty()) {
        statsResidents.append(StatResident("Résidents", totalResidents, (totalResidents * 100.0) / maxResidents));
    }
}

void MainWindow::afficherStatistiquesCirculaire()
{
    if (statsResidents.isEmpty()) {
        QMessageBox::warning(this, "Statistiques", "Aucune donnée de résident disponible!");
        return;
    }

    QGraphicsScene *scene = new QGraphicsScene(this);
    QRectF zoneDiagramme(50,50,300,300);
    dessinerDiagrammeCirculaire(scene, statsResidents, zoneDiagramme);

    int totalResidents = 0;
    for (const StatResident &stat : std::as_const(statsResidents)) {
        totalResidents += stat.count;
    }

    QGraphicsTextItem *titre = scene->addText("📊 STATISTIQUES DES RÉSIDENTS");
    titre->setDefaultTextColor(Qt::darkBlue);
    titre->setFont(QFont("Arial", 14, QFont::Bold));
    titre->setPos(50, 10);

    QGraphicsTextItem *total = scene->addText(QString("Total: %1 résidents").arg(totalResidents));
    total->setDefaultTextColor(Qt::darkGreen);
    total->setFont(QFont("Arial", 11, QFont::Bold));
    total->setPos(50, 35);

    QList<QColor> colors = { QColor(255, 99, 132), QColor(54, 162, 235), QColor(255, 205, 86), QColor(75, 192, 192),
                            QColor(153, 102, 255), QColor(255, 159, 64), QColor(201, 203, 207), QColor(255, 99, 255),
                            QColor(50, 205, 50), QColor(70, 130, 180) };
    int xLegende = 400;
    int yLegende = 50;
    for (int i = 0; i < statsResidents.size(); ++i) {
        const StatResident &stat = statsResidents[i];
        QColor couleur = colors[i % colors.size()];
        QGraphicsRectItem *carre = scene->addRect(xLegende, yLegende, 15, 15);
        carre->setBrush(QBrush(couleur));
        carre->setPen(QPen(Qt::black,1));
        QString texteLegende = QString("%1: %2% (%3)")
                                   .arg(stat.categorie, QString::number(stat.percentage,'f',1), QString::number(stat.count));
        QGraphicsTextItem *legende = scene->addText(texteLegende);
        legende->setDefaultTextColor(Qt::black);
        legende->setFont(QFont("Arial",9));
        legende->setPos(xLegende+25,yLegende-3);
        yLegende += 25;
    }

    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setSceneRect(scene->itemsBoundingRect());
}

void MainWindow::dessinerDiagrammeCirculaire(QGraphicsScene *scene, const QList<StatResident> &stats, const QRectF &zone)
{
    if (stats.isEmpty()) return;
    QList<QColor> colors = { QColor(255, 99, 132), QColor(54, 162, 235), QColor(255, 205, 86), QColor(75, 192, 192),
                            QColor(153, 102, 255), QColor(255, 159, 64), QColor(201, 203, 207), QColor(255, 99, 255),
                            QColor(50, 205, 50), QColor(70, 130, 180) };
    double angleDepart = 0.0;
    double rayon = qMin(zone.width(), zone.height()) / 2.0;
    QPointF centre(zone.center());

    scene->addEllipse(zone, QPen(QColor(100,100,100),2), QBrush(QColor(240,240,240)));

    for (int i=0; i<stats.size(); ++i) {
        double angleSegment = (stats[i].percentage * 360.0) / 100.0;
        if (angleSegment <=0) continue;
        QPainterPath path;
        path.moveTo(centre);
        path.arcTo(zone, angleDepart, angleSegment);
        path.closeSubpath();
        QGraphicsPathItem *segment = scene->addPath(path);
        segment->setBrush(QBrush(colors[i % colors.size()]));
        segment->setPen(QPen(Qt::black,1));

        if (stats[i].percentage > 15) {
            QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
            shadow->setBlurRadius(10);
            shadow->setColor(QColor(0,0,0,100));
            shadow->setOffset(2,2);
            segment->setGraphicsEffect(shadow);
        }

        double angleMilieu = angleDepart + angleSegment/2.0;
        double angleRad = qDegreesToRadians(angleMilieu);
        double distanceTexte = rayon*0.6;
        QPointF posTexte(centre.x()+distanceTexte*qCos(angleRad), centre.y()+distanceTexte*qSin(angleRad));
        if (angleSegment>10) {
            QString textePourcentage = QString("%1%").arg(QString::number(stats[i].percentage,'f',1));
            QGraphicsTextItem *texte = scene->addText(textePourcentage);
            texte->setDefaultTextColor(Qt::black);
            texte->setFont(QFont("Arial",8,QFont::Bold));
            QRectF rectTexte = texte->boundingRect();
            texte->setPos(posTexte.x()-rectTexte.width()/2, posTexte.y()-rectTexte.height()/2);
        }
        angleDepart += angleSegment;
    }

    QRectF zoneCentre(centre.x()-rayon*0.3, centre.y()-rayon*0.3, rayon*0.6, rayon*0.6);
    QGraphicsEllipseItem *cercleCentre = scene->addEllipse(zoneCentre);
    cercleCentre->setBrush(QBrush(Qt::white));
    cercleCentre->setPen(QPen(Qt::black,1));

    int totalResidents = 0;
    for (const StatResident &stat : std::as_const(stats)) totalResidents += stat.count;
    QGraphicsTextItem *texteCentre = scene->addText(QString("%1 / 200").arg(totalResidents));
    texteCentre->setDefaultTextColor(Qt::darkBlue);
    texteCentre->setFont(QFont("Arial",16,QFont::Bold));
    QRectF rectTexteCentre = texteCentre->boundingRect();
    texteCentre->setPos(centre.x()-rectTexteCentre.width()/2, centre.y()-rectTexteCentre.height()/2);
}
// sort
void MainWindow::sortResidents()
{
    QString type = ui->sortComboBox->currentText().trimmed().toUpper();
    Residents r;

    // Pour afficher tous les résidents triés
    ui->tableView->setModel(r.afficher("", type));
}
