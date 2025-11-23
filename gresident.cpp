#include "gresident.h"
#include "ui_gresident.h"
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QInputDialog>
#include <QDebug>
#include <QRegularExpression>
#include <QMouseEvent>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QScrollBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
GResident::GResident(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::GResident),
    searchAppartementTimer(new QTimer(this))
{
    ui->setupUi(this);  // ✅ TOUJOURS EN PREMIER

    m_voiceToText = new VoiceToText(this);
    const QString initialApiKey = resolveHumeApiKey();
    if (!initialApiKey.isEmpty()) {
        m_voiceToText->setApiKey(initialApiKey);
    } else {
        // If env var not set, try a file in the app dir or user home (untracked by git)
        const QString fileKey = resolveHumeApiKeyFromFile();
        if (!fileKey.isEmpty()) {
            m_voiceToText->setApiKey(fileKey);
        }
    }

    connect(m_voiceToText, &VoiceToText::finalMessageReceived, this, &GResident::handleVoiceFinalMessage);
    connect(m_voiceToText, &VoiceToText::interimMessageReceived, this, &GResident::handleVoiceInterimMessage);
    connect(m_voiceToText, &VoiceToText::statusChanged, this, &GResident::handleVoiceStatusChanged);
    connect(m_voiceToText, &VoiceToText::errorOccurred, this, &GResident::handleVoiceError);

    if (ui->recordingstart) {
        connect(ui->recordingstart, &QPushButton::clicked, this, &GResident::onSpeechToTextButtonClicked);
    }
    handleVoiceStatusChanged(tr("Ready to record"));

    // Add Hume menu with a Set API Key action for safe local configuration
    QAction *setKeyAction = new QAction(tr("Set Hume API Key..."), this);
    connect(setKeyAction, &QAction::triggered, this, &GResident::setHumeApiKeyDialog);
    QMenu *humeMenu = menuBar()->addMenu(tr("Hume"));
    humeMenu->addAction(setKeyAction);

    // ====== TIMER Setup ======
    searchAppartementTimer = new QTimer(this);
    searchAppartementTimer->setSingleShot(true);
    searchAppartementTimer->setInterval(300);
    connect(searchAppartementTimer, &QTimer::timeout, this, &GResident::on_searchAppartementDynamic);

    // ====== NAVIGATION Setup ======
    connect(ui->home, &QPushButton::clicked, this, [this]() {
        navigateToPage(0);
    });
    connect(ui->resident, &QPushButton::clicked, this, [this]() {
        navigateToPage(1);
        refreshResidentTable();
    });
    connect(ui->employee, &QPushButton::clicked, this, [this]() {
        navigateToPage(2);
    });
    connect(ui->appartement, &QPushButton::clicked, this, [this]() {
        navigateToPage(3);
        refreshAppartementTable();
    });
    connect(ui->vehicles, &QPushButton::clicked, this, [this]() {
        navigateToPage(4);
    });
    connect(ui->services, &QPushButton::clicked, this, [this]() {
        navigateToPage(5);
        refreshServiceTable();
    });

    // ====== RECHERCHE DYNAMIQUE Setup ======
    connect(ui->lineEdit_63, &QLineEdit::textChanged, this, &GResident::on_searchAppartementTextChanged);
    // Also allow dynamic search from the search input in the Appartement modify tab
    // support both local apartment search input names: prefer `searchEdit` when present
    if (ui->searchEdit) {
        connect(ui->searchEdit, &QLineEdit::textChanged, this, &GResident::on_searchRoomEditTextChanged);
    }
    if (ui->searchRoomEdit) {
        connect(ui->searchRoomEdit, &QLineEdit::textChanged, this, &GResident::on_searchRoomEditTextChanged);
    }

    // ====== PDF Export ======
    connect(ui->exportPdfBtn, &QPushButton::clicked, this, &GResident::on_exportPdfBtn_clicked);

    // ====== EMPLOYEE Setup ======
    Employe e;
    ui->employetableView->setModel(e.afficher());

    // ====== RESIDENT Setup ======
    connect(ui->pushButton_9, &QPushButton::clicked, this, &GResident::onAddResidentClicked);

    if (ui->deletebtn) {
        connect(ui->deletebtn, &QPushButton::clicked, this, &GResident::onDeleteResidentClicked);
    } else {
        qDebug() << "Warning: deletebtn not found in UI";
    }

    if (ui->updateresident) {
        connect(ui->updateresident, &QPushButton::clicked, this, &GResident::onModifyResidentClicked);
    } else {
        qDebug() << "Warning: updateresident not found in UI";
    }

    if (ui->pushButton_39) {
        connect(ui->pushButton_39, &QPushButton::clicked, this, &GResident::onSearchResidentClicked);
    } else {
        qDebug() << "Warning: pushButton_39 not found in UI";
    }
    // Dans le constructeur GResident::GResident()
    // Ajoutez cette connexion :

    // ====== STATISTIQUES Setup ======
    // ====== STATISTIQUES Setup ======
    // ====== STATISTIQUES Setup ======
    if (ui->statisticsappartement) {
        qDebug() << "✅ Widget statisticsappartement trouvé - Installation EventFilter";

        // Installer l'event filter
        ui->statisticsappartement->installEventFilter(this);

        // Rendre le widget cliquable
        ui->statisticsappartement->setCursor(Qt::PointingHandCursor);
        ui->statisticsappartement->setToolTip("Cliquer pour voir les statistiques des appartements");

        // Donner un style visuel si c'est un QLabel
        QLabel *statsLabel = qobject_cast<QLabel*>(ui->statisticsappartement);
        if (statsLabel) {
            statsLabel->setText("📊 Statistiques Appartements");
            statsLabel->setAlignment(Qt::AlignCenter);
            statsLabel->setStyleSheet(
                "QLabel {"
                "    background-color: #2196F3;"
                "    color: white;"
                "    font-weight: bold;"
                "    border-radius: 8px;"
                "    padding: 10px;"
                "}"
                );
        }
    } else {
        qDebug() << "❌ Widget statisticsappartement non trouvé";

        // Créer un bouton de secours
        QPushButton *fallbackBtn = new QPushButton("📊 Stats Appartements", this);
        fallbackBtn->setObjectName("fallbackStatsBtn");
        fallbackBtn->setGeometry(50, 50, 200, 45);
        fallbackBtn->setStyleSheet(
            "QPushButton {"
            "    background-color: #4CAF50;"
            "    color: white;"
            "    font-weight: bold;"
            "    border-radius: 8px;"
            "    padding: 10px;"
            "}"
            );
        connect(fallbackBtn, &QPushButton::clicked, this, &GResident::onStatsAppartementsOccupees);
    }
    // ====== SERVICE CRUD Setup ======
    if (ui->pushButton_28) {
        connect(ui->pushButton_28, &QPushButton::clicked, this, &GResident::onAddServiceClicked);
    }
    if (ui->pushButton_27) {
        connect(ui->pushButton_27, &QPushButton::clicked, this, &GResident::onModifyServiceClicked);
    }
    if (ui->DeleteService) {
        connect(ui->DeleteService, &QPushButton::clicked, this, &GResident::onDeleteServiceClicked);
    }

    // ====== APPARTEMENT CRUD Setup ======
    if (ui->pushButton_18) {
        connect(ui->pushButton_18, &QPushButton::clicked, this, &GResident::onAddAppartementClicked);
    }
    if (ui->pushButton_19) {
        connect(ui->pushButton_19, &QPushButton::clicked, this, &GResident::onModifyAppartementClicked);
    }
    if (ui->pushButton_20) {
        connect(ui->pushButton_20, &QPushButton::clicked, this, &GResident::onDeleteAppartementClicked);
    }

    // Search buttons for MODIFY tabs
    if (ui->searchRoomBtn) {
        connect(ui->searchRoomBtn, &QPushButton::clicked, this, &GResident::onSearchAppartementForModifyClicked);
    }

    // ====== VEHICLE CRUD Setup ======
    if (ui->addVehicule) {
        connect(ui->addVehicule, &QPushButton::clicked, this, &GResident::onAddVehicleClicked);
    }
    if (ui->updatevehicule) {
        connect(ui->updatevehicule, &QPushButton::clicked, this, &GResident::onModifyVehicleClicked);
    }
    if (ui->deletevalider) {
        connect(ui->deletevalider, &QPushButton::clicked, this, &GResident::onDeleteVehicleClicked);
    }
    if (ui->pushButton_36) {
        connect(ui->pushButton_36, &QPushButton::clicked, this, &GResident::onSearchVehicleClicked);
    }
    if (ui->sortBtn) {
        connect(ui->sortBtn, &QPushButton::clicked, this, &GResident::on_sortBtn_clicked);

        // Connect the apartment search button to perform a search
        if (ui->searchRoomBtn_3) {
            // Run the apartment search when the user clicks the 'Search' button in the appartement page
            connect(ui->searchRoomBtn_3, &QPushButton::clicked, this, &GResident::on_searchRoomBtn3Clicked);
        }
    }

    // ====== SMARTAVAILABILITY Setup ======
    // Check if there's a smartavailability widget or button
    // Try to find a button or label in the apartement tab that can trigger SmartAvailability
    if (ui->smartAvailabilityBtn) {
        connect(ui->smartAvailabilityBtn, &QPushButton::clicked, this, &GResident::onSmartAvailabilityClicked);
        qDebug() << "✅ SmartAvailability button found and connected!";
    } else {
        qDebug() << "⚠️  SmartAvailability button NOT found!";
    }

    // Statistics button
    if (ui->statistic) {
        connect(ui->statistic, &QPushButton::clicked, this, &GResident::onStatsAppartementsOccupees);
        ui->statistic->setCursor(Qt::PointingHandCursor);
        ui->statistic->setStyleSheet(
            "QPushButton {"
            "    background-color: #FF9800;"
            "    color: white;"
            "    font-weight: bold;"
            "    border-radius: 8px;"
            "    font-size: 16px;"
            "}"
            "QPushButton:hover {"
            "    background-color: #F57C00;"
            "}"
            );
        qDebug() << "✅ Statistics button found and connected!";
    } else {
        qDebug() << "⚠️  Statistics button NOT found!";
    }

    if (ui->stackedWidget) {
        // We'll connect to page navigation instead
        connect(ui->appartement, &QPushButton::clicked, this, [this]() {
            // SmartAvailability can be launched manually from UI
        });
    }

    // ✅ Start on first page (index 0)
    ui->stackedWidget->setCurrentIndex(0);
}

GResident::~GResident()
{
    if (m_voiceToText) {
        m_voiceToText->stop();
    }
    delete ui;
}
void GResident::navigateToPage(int pageIndex)
{
    ui->stackedWidget->setCurrentIndex(pageIndex);
}


// ========== EVENT FILTER IMPLÉMENTATION ==========

bool GResident::eventFilter(QObject *obj, QEvent *event)
{
    // Vérifier si l'événement concerne le widget statisticsappartement
    if (obj == ui->statisticsappartement) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                qDebug() << "✅ Clic détecté sur le widget statisticsappartement";
                onStatsAppartementsOccupees();
                return true; // Événement traité
            }
        }
    }

    // Passer l'événement à la classe parent pour les autres objets
    return QMainWindow::eventFilter(obj, event);
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

void GResident::on_ajouterpushbutton_clicked()
{
    QString id = ui->idLineEdit->text();
    QString nom = ui->nomLineEdit->text();
    QString prenom = ui->prenomLineEdit->text();
    QString poste = ui->posteLineEdit->text();
    QString salaire = ui->salaireLineEdit->text();

    if(id.isEmpty() || nom.isEmpty() || prenom.isEmpty() || poste.isEmpty() || salaire.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Tous les champs doivent être remplis !");
        return;
    }

    QRegularExpression regexNom("^[A-Za-z]+$");
    QRegularExpression regexPoste("^[A-Za-z]+$");

    // Vérifier le nom
    if(!regexNom.match(nom).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "Le nom doit contenir uniquement des lettres !");
        return;
    }

    // Vérifier le prénom
    if(!regexNom.match(prenom).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "Le prénom doit contenir uniquement des lettres !");
        return;
    }

    // Vérifier le poste
    if(!regexPoste.match(poste).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "Le poste doit contenir uniquement des lettres !");
        return;
    }

    // Vérifier ID numérique
    bool ok;
    id.toInt(&ok);
    if(!ok) {
        QMessageBox::warning(this, "Erreur", "L'ID doit être un nombre !");
        return;
    }

    // Vérifier salaire numérique
    salaire.toDouble(&ok);
    if(!ok) {
        QMessageBox::warning(this, "Erreur", "Le salaire doit être un nombre !");
        return;
    }

    Employe e(id, nom, prenom, poste, salaire);

    if(e.ajouter()) {
        ui->employetableView->setModel(e.afficher());
        QMessageBox::information(this, "Succès", "Employé ajouté !");
    } else {
        QMessageBox::critical(this, "Erreur", "Échec d'ajout !");
    }
}

void GResident::on_deleteButton_clicked()
{
    QString id = ui->idLineEditDelete->text();

    if (id.isEmpty()) {
        QMessageBox::warning(this, "Attention", "Veuillez entrer l'ID de l'employé à supprimer.");
        return;
    }

    Employe e;

    // ✅ Vérifier existence AVANT suppression
    if(!e.existe(id)) {
        QMessageBox::warning(this, "Erreur", "Cet employé n'existe pas !");
        return; // ✅ IMPORTANT : arrêter ici
    }

    // ✅ Ici c'est sûr que l'employé existe
    if ( e.supprimer(id)) {
        QMessageBox::information(this, "Succès", "Employé supprimé avec succès.");
        ui->employetableView->setModel(e.afficher()); // Refresh table
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de suppression.\nVérifiez l'ID.");
    }
}

void GResident::on_employetableView_clicked(const QModelIndex &index)
{
    int row = index.row();

    ui->idLineEditModif->setText(index.sibling(row, 0).data().toString());
    ui->nomLineEditModif->setText(index.sibling(row, 1).data().toString());
    ui->prenomLineEditModif->setText(index.sibling(row, 2).data().toString());
    ui->posteLineEditModif->setText(index.sibling(row, 3).data().toString());
    ui->salaireLineEditModif->setText(index.sibling(row, 4).data().toString());
    ui->ADD_2->setCurrentWidget(ui->tab_4);
}

void GResident::on_modifierpushbutton_clicked()
{
    QString id = ui->idLineEditModif->text();
    QString nom = ui->nomLineEditModif->text();
    QString prenom = ui->prenomLineEditModif->text();
    QString poste = ui->posteLineEditModif->text();
    QString salaire = ui->salaireLineEditModif->text();

    // 1. Vérifier que tous les champs sont remplis
    if(id.isEmpty() || nom.isEmpty() || prenom.isEmpty() || poste.isEmpty() || salaire.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Tous les champs doivent être remplis !");
        return;
    }

    // 2. Vérifier que l'ID est numérique
    bool ok;
    id.toInt(&ok);
    if(!ok) {
        QMessageBox::warning(this, "Erreur", "L'ID doit être un nombre !");
        return;
    }

    // 3. Vérifier que le salaire est numérique
    salaire.toDouble(&ok);
    if(!ok) {
        QMessageBox::warning(this, "Erreur", "Le salaire doit être un nombre !");
        return;
    }

    // 4. Créer l'objet Employe et modifier
    Employe e(id, nom, prenom, poste, salaire);
    if(e.modifier()) {
        QMessageBox::information(this, "Succès", "Employé modifié avec succès !");

        // ✅ IMPORTANT : rafraîchir APRÈS modification
        ui->employetableView->setModel(e.afficher());
    } else {
        QMessageBox::warning(this, "Erreur", "Échec de la modification !");
    }
}

void GResident::on_ajouterpushbutton_2_clicked()
{
    QString poste = ui->idLineEditModif_2->text();

    if(poste.isEmpty())
    {
        QMessageBox::warning(this, "Erreur", "Veuillez saisir un poste !");
        return;
    }

    Employe e;
    ui->employetableView->setModel(e.afficherPoste(poste));
}

// ========== RESIDENT CRUD Implementations ==========

void GResident::refreshResidentTable()
{
    if (!ui->residentTableView) return;

    QSqlQueryModel *model = Resident::afficher();
    ui->residentTableView->setModel(model);
    ui->residentTableView->resizeColumnsToContents();
}

void GResident::onAddResidentClicked()
{
    // Pour Resident ADD tab (page_6)
    QString cin = ui->lineEdit_6->text().trimmed();        // CIN (ID)
    QString nom = ui->lineEdit_7->text().trimmed();        // Nom (Name)
    QString prenom = ui->lineEdit_8->text().trimmed();     // Prénom (Last Name)
    QString type = ui->comboBox->currentText().trimmed();  // Type (Locataire/Propriétaire)

    if (cin.isEmpty() || nom.isEmpty() || prenom.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "CIN, Nom et Prénom sont obligatoires !");
        return;
    }

    // Validation : CIN format (ex: 10 caractères)
    if (cin.length() > 10) {
        QMessageBox::warning(this, "Erreur", "Le CIN ne doit pas dépasser 10 caractères !");
        return;
    }

    // Tel peut être vide pour Resident
    Resident r(cin, nom, prenom, "");
    if (r.ajouter()) {
        QMessageBox::information(this, "Succès", "Résident ajouté avec succès !");
        refreshResidentTable();
        // Clear fields
        ui->lineEdit_6->clear();
        ui->lineEdit_7->clear();
        ui->lineEdit_8->clear();
    } else {
        QMessageBox::warning(this, "Erreur", "Erreur lors de l'ajout du résident !");
    }
}

void GResident::onDeleteResidentClicked()
{
    // Pour Resident DELETE tab - read ID from DeleteRESIDENT field
    QString cin = ui->DeleteRESIDENT->text().trimmed();

    // Extract just the numbers/alphanumeric
    QString cleanCin;
    for (QChar c : cin) {
        if (c.isLetterOrNumber()) {
            cleanCin += c;
        }
    }
    if (!cleanCin.isEmpty()) {
        cin = cleanCin;
    }

    qDebug() << "[onDeleteResidentClicked] CIN after cleanup:" << cin;

    if (cin.isEmpty()) {
        QMessageBox::warning(this, "Attention", "Veuillez entrer le CIN/ID du résident à supprimer dans le champ.");
        return;
    }

    qDebug() << "[onDeleteResidentClicked] Deleting Resident CIN:" << cin;

    if (Resident::supprimer(cin)) {
        QMessageBox::information(this, "Succès", "Résident CIN " + cin + " supprimé avec succès !");
        refreshResidentTable();
        ui->DeleteRESIDENT->clear();
    } else {
        QMessageBox::critical(this, "Erreur", "Erreur lors de la suppression du résident CIN " + cin + " !");
    }
}

void GResident::onModifyResidentClicked()
{
    // Read ID from lineEdit_15
    QString cin = ui->lineEdit_15->text().trimmed();

    // Extract just the numbers/alphanumeric
    QString cleanCin;
    for (QChar c : cin) {
        if (c.isLetterOrNumber()) {
            cleanCin += c;
        }
    }
    if (!cleanCin.isEmpty()) {
        cin = cleanCin;
    }

    qDebug() << "[onModifyResidentClicked] CIN to modify:" << cin;

    if (cin.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "CIN/ID est obligatoire !");
        return;
    }

    // Read Nom from lineEdit_14 and Prenom from lineEdit_11
    QString nom = ui->lineEdit_14->text().trimmed();
    QString prenom = ui->lineEdit_11->text().trimmed();

    qDebug() << "[DEBUG] lineEdit_14 (Nom) text:" << ui->lineEdit_14->text();
    qDebug() << "[DEBUG] lineEdit_11 (Prenom) text:" << ui->lineEdit_11->text();
    qDebug() << "[DEBUG] After trimmed - Nom:" << nom << "Prenom:" << prenom;

    // Validate that at least one field is filled
    if (nom.isEmpty() && prenom.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Entrez au moins le Nom ou le Prénom !");
        return;
    }

    qDebug() << "[onModifyResidentClicked] Modifying Resident CIN:" << cin << "Nom:" << nom << "Prenom:" << prenom;

    // Create and modify resident
    Resident r(cin, nom, prenom, "");
    if (r.modifier(cin)) {
        QMessageBox::information(this, "Succès", "Résident CIN " + cin + " modifié avec succès !");
        refreshResidentTable();
        ui->lineEdit_15->clear();
        ui->lineEdit_14->clear();
        ui->lineEdit_11->clear();
    } else {
        QMessageBox::warning(this, "Erreur", "Erreur lors de la modification du résident CIN " + cin + " !");
    }
}

void GResident::onSearchResidentClicked()
{
    // Determine which search input to use: prefer the apartment-specific searchRoomEdit if not empty
    QString searchText;
    if (ui->searchRoomEdit && !ui->searchRoomEdit->text().trimmed().isEmpty()) {
        searchText = ui->searchRoomEdit->text().trimmed();
    } else {
        searchText = ui->lineEdit_63->text().trimmed();
    }

    if (searchText.isEmpty()) {
        refreshResidentTable();
        return;
    }

    QSqlQueryModel *model = Resident::rechercher(searchText);
    ui->residentTableView->setModel(model);
    ui->residentTableView->resizeColumnsToContents();
}

// ========== SERVICE CRUD Implementations ==========

void GResident::refreshServiceTable()
{
    if (!ui->serviceTableView) return;

    QSqlQueryModel *model = ServiceItem::afficher();
    ui->serviceTableView->setModel(model);
    ui->serviceTableView->resizeColumnsToContents();
}

void GResident::onAddServiceClicked()
{
    // Récupérer les valeurs depuis l'UI (Service ADD tab: page_5)
    QString nom = ui->lineEdit_86->text().trimmed();           // Service Name input
    QString type = ui->boxType_3->currentText().trimmed();     // Service Type dropdown
    double prix = ui->spinPrix_3->value();                     // Service Price spinbox - USE .value() not .text()!
    QDate dateService = ui->editDate_3->date();                // Service Date picker
    QString status = ui->boxStatut_3->currentText().trimmed(); // Service Status dropdown

    // Validation minimaliste - juste vérifier que les champs ne sont pas vides
    if (nom.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Le nom du service est obligatoire !");
        return;
    }

    qDebug() << "[onAddServiceClicked] Nom:" << nom << "Type:" << type << "Prix:" << prix << "Date:" << dateService << "Status:" << status;

    ServiceItem s(0, nom, type, prix, dateService, status);
    if (s.ajouter(true)) { // Use sequence for ID
        QMessageBox::information(this, "Succès", "Service ajouté avec succès !");
        refreshServiceTable();
        ui->lineEdit_86->clear();
        ui->spinPrix_3->setValue(0);
        ui->editDate_3->setDate(QDate::currentDate());
    } else {
        QMessageBox::critical(this, "Erreur", "Échec d'ajout du service !");
    }
}

void GResident::onDeleteServiceClicked()
{
    QString idStr = ui->lineEdit_70->text().trimmed(); // Get ID from DELETE tab input

    qDebug() << "[onDeleteServiceClicked] ID string raw:" << idStr;

    // Extract just the numbers if there's mixed text
    QString numericIdStr;
    for (QChar c : idStr) {
        if (c.isDigit()) {
            numericIdStr += c;
        }
    }
    if (!numericIdStr.isEmpty()) {
        idStr = numericIdStr;
    }

    qDebug() << "[onDeleteServiceClicked] ID string after cleanup:" << idStr;

    if (idStr.isEmpty() || idStr == "0") {
        QMessageBox::warning(this, "Attention", "Veuillez entrer l'ID du service à supprimer dans le champ.");
        return;
    }

    bool ok;
    int id = idStr.toInt(&ok);
    if (!ok) {
        id = 0; // Default to 0 if conversion fails
    }

    qDebug() << "[onDeleteServiceClicked] Deleting Service ID:" << id;

    if (ServiceItem::supprimer(id)) {
        QMessageBox::information(this, "Succès", "Service ID " + QString::number(id) + " supprimé avec succès !");
        refreshServiceTable();
        ui->lineEdit_70->clear();
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de suppression du service ID " + QString::number(id) + " !");
    }
}

void GResident::onModifyServiceClicked()
{
    QString idStr = ui->lineEdit_91->text().trimmed();           // Service ID input (the white input field)

    qDebug() << "[onModifyServiceClicked] ID string raw:" << idStr;

    // Extract just the numbers if there's mixed text
    QString numericIdStr;
    for (QChar c : idStr) {
        if (c.isDigit()) {
            numericIdStr += c;
        }
    }
    if (!numericIdStr.isEmpty()) {
        idStr = numericIdStr;
    }

    qDebug() << "[onModifyServiceClicked] ID string after cleanup:" << idStr;

    QString nom = ui->lineEdit_89->text().trimmed();             // Service Name input
    QString type = ui->boxType_4->currentText().trimmed();       // Service Type dropdown
    double prix = ui->spinPrix_4->value();                       // Service Price spinbox - USE .value() not .text()!
    QDate dateService = ui->editDate_4->date();                  // Service Date picker
    QString status = "PLANIFIE";                                 // Default status

    // Validation minimaliste - juste que l'ID n'est pas vide après cleanup
    if (idStr.isEmpty() || idStr == "0") {
        QMessageBox::warning(this, "Erreur", "L'ID du service est obligatoire ! Veuillez entrer l'ID dans le premier champ.");
        return;
    }

    // Essayer de convertir ID en nombre - si ce n'est pas un nombre, utiliser 0
    bool ok;
    int id = idStr.toInt(&ok);
    if (!ok) {
        id = 0; // Default to 0 if conversion fails
    }

    qDebug() << "[onModifyServiceClicked] ID converted to:" << id;
    qDebug() << "[onModifyServiceClicked] Updating Service ID:" << id << "with Nom:" << nom << "Prix:" << prix;

    ServiceItem s(id, nom, type, prix, dateService, status);

    if (s.modifier(id)) {
        QMessageBox::information(this, "Succès", "Service ID " + QString::number(id) + " modifié avec succès !");
        refreshServiceTable();
        ui->lineEdit_91->clear();
        ui->lineEdit_89->clear();
        ui->spinPrix_4->setValue(0);
        ui->editDate_4->setDate(QDate::currentDate());
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de la modification du service ID " + QString::number(id) + ". Vérifiez que l'ID existe dans la BD !");
    }
}

void GResident::onSearchServiceClicked()
{
    QString searchText = ui->lineEdit_63->text().trimmed();

    if (searchText.isEmpty()) {
        refreshServiceTable();
        return;
    }

    QSqlQueryModel *model = ServiceItem::rechercher(searchText);
    ui->serviceTableView->setModel(model);
    ui->serviceTableView->resizeColumnsToContents();
}

void GResident::onSearchServiceForModifyClicked()
{
    // Rechercher un service par ID pour le remplir dans le MODIFY tab
    QString idStr = ui->lineEdit_91->text().trimmed();

    qDebug() << "[onSearchServiceForModifyClicked] ID string raw:" << idStr;

    // Extract just the numbers
    QString numericIdStr;
    for (QChar c : idStr) {
        if (c.isDigit()) {
            numericIdStr += c;
        }
    }
    if (!numericIdStr.isEmpty()) {
        idStr = numericIdStr;
    }

    qDebug() << "[onSearchServiceForModifyClicked] ID string after cleanup:" << idStr;

    if (idStr.isEmpty()) {
        QMessageBox::warning(this, "Attention", "Veuillez entrer l'ID du service à chercher dans le premier champ!");
        return;
    }

    // Convertir l'ID
    bool ok;
    int id = idStr.toInt(&ok);
    if (!ok) {
        id = 0; // Default to 0 if not a number
    }

    qDebug() << "[onSearchServiceForModifyClicked] Searching for Service ID:" << id;

    // Récupérer les données du service depuis la BD
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);

    // Use string concatenation instead of bindValue for Oracle compatibility
    QString sql = QString("SELECT ID_SERVICE, NOM, TYPE_SERVICE, PRIX, DATE_SERVICE FROM SERVICE WHERE ID_SERVICE = %1").arg(id);
    qDebug() << "[onSearchServiceForModifyClicked] SQL:" << sql;

    if (query.exec(sql)) {
        if (query.next()) {
            // Remplir les champs avec les données trouvées
            ui->lineEdit_89->setText(query.value(1).toString());  // NOM
            ui->boxType_4->setCurrentText(query.value(2).toString()); // TYPE_SERVICE
            ui->spinPrix_4->setValue(query.value(3).toDouble());   // PRIX
            ui->editDate_4->setDate(query.value(4).toDate());      // DATE_SERVICE
            qDebug() << "[onSearchServiceForModifyClicked] Service found!";
            QMessageBox::information(this, "Succès", "Service ID " + QString::number(id) + " trouvé !");
        } else {
            qDebug() << "[onSearchServiceForModifyClicked] No service found with ID:" << id;
            QMessageBox::warning(this, "Erreur", "Aucun service trouvé avec l'ID: " + QString::number(id));
            ui->lineEdit_89->clear();
            ui->spinPrix_4->setValue(0);
            ui->editDate_4->setDate(QDate::currentDate());
        }
    } else {
        qDebug() << "[onSearchServiceForModifyClicked] Query failed:" << query.lastError().text();
        QMessageBox::critical(this, "Erreur BD", "Erreur lors de la recherche: " + query.lastError().text());
    }
}

// ========== APPARTEMENT CRUD Implementations ==========

void GResident::refreshAppartementTable()
{
    if (!ui->appartementTableView) return;

    // Delete old model if it exists
    if (ui->appartementTableView->model()) {
        delete ui->appartementTableView->model();
    }

    // Create a QSqlQueryModel to display the data
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    query.prepare("SELECT ID, ROOM, FLOOR, CONDITION, STATUS, CIN FROM APPARTEMENT ORDER BY ID");
    if (!query.exec()) {
        qDebug() << "[refreshAppartementTable] ERROR executing query:" << query.lastError().text();
    }
    model->setQuery(std::move(query));

    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Room");
    model->setHeaderData(2, Qt::Horizontal, "Floor");
    model->setHeaderData(3, Qt::Horizontal, "Condition");
    model->setHeaderData(4, Qt::Horizontal, "Status");
    model->setHeaderData(5, Qt::Horizontal, "CIN");

    ui->appartementTableView->setModel(model);
    ui->appartementTableView->resizeColumnsToContents();
}

void GResident::onAddAppartementClicked()
{
    // Get values from UI (Appartement ADD tab: page_2, tab_16)
    QString cinStr = ui->cinadd->text().trimmed();            // CIN de RESIDENT
    QString roomStr = ui->lineEdit_48->text().trimmed();      // Room/Number input (lineEdit_48 exists)
    QString condition = ui->lineEdit_103->text().trimmed();   // Condition input
    QString status = ui->addStatusCombo_2->currentText().trimmed(); // Status dropdown

    // Validation: CIN est obligatoire
    if (cinStr.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Le CIN du résident est obligatoire !");
        return;
    }

    // Validation: au moins room est obligatoire
    if (roomStr.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Le Room/Number est obligatoire !");
        return;
    }

    // Convertir room - si ce n'est pas un nombre, utiliser 0
    int room = 0;
    bool ok;
    room = roomStr.toInt(&ok);
    if (!ok) {
        room = 0; // Default to 0 if not valid
    }

    // Floor utilise la même valeur que room (car il n'y a pas de widget floor séparé)
    int floor = 0;

    AppartementRecord rec;
    rec.room = room;
    rec.floor = floor;
    rec.condition = condition;
    rec.status = status;
    rec.cin = cinStr;  // CIN du résident

    qDebug() << "[onAddAppartementClicked] Adding Appartement with CIN:" << cinStr << "Room:" << room << "Floor:" << floor;

    if (AppartementCRUD::insertAppartement(rec)) {
        QMessageBox::information(this, "Succès", "Appartement ajouté avec succès !");
        refreshAppartementTable();
        ui->cinadd->clear();
        ui->lineEdit_48->clear();
        ui->lineEdit_103->clear();
    } else {
        QMessageBox::critical(this, "Erreur", "Échec d'ajout de l'appartement !");
    }
}

void GResident::onDeleteAppartementClicked()
{
    QString idStr = ui->deleteSearchEdit->text().trimmed(); // Get ID from DELETE tab search field

    if (idStr.isEmpty()) {
        QMessageBox::warning(this, "Attention", "Veuillez entrer le numéro de l'appartement à supprimer.");
        return;
    }

    // Essayer de convertir ID - si ce n'est pas un nombre, utiliser 0
    bool ok;
    qint64 id = idStr.toLongLong(&ok);
    if (!ok) {
        id = 0; // Default to 0 if not valid
    }

    if (AppartementCRUD::deleteAppartement(id)) {
        QMessageBox::information(this, "Succès", "Appartement supprimé avec succès !");
        refreshAppartementTable();
        ui->deleteSearchEdit->clear();
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de suppression de l'appartement !");
    }
}

void GResident::onModifyAppartementClicked()
{
    // Get values from UI (Appartement MODIFY tab: page_2, tab_13)
    QString idStr = ui->searchRoomEdit->text().trimmed();         // Appartement ID (search field)
    QString cinStr = ui->cinmod->text().trimmed();                // CIN de RESIDENT
    QString roomStr = ui->lineEdit_51->text().trimmed();          // Room input
    QString floorStr = ui->lineEdit_55->text().trimmed();         // Floor input
    QString condition = ui->lineEdit_113->text().trimmed();       // Condition input
    QString status = ui->addStatusCombo_3->currentText().trimmed(); // Status dropdown

    // Validation: ID est obligatoire
    if (idStr.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "L'ID est obligatoire !");
        return;
    }

    // Validation: CIN est obligatoire
    if (cinStr.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Le CIN du résident est obligatoire !");
        return;
    }

    // Essayer de convertir ID - si ce n'est pas un nombre, utiliser 0
    bool ok;
    qint64 id = idStr.toLongLong(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Erreur", "L'ID \"" + idStr + "\" doit être un nombre !");
        return;
    }

    // Convertir room - si ce n'est pas un nombre, utiliser 0
    int room = 0;
    if (!roomStr.isEmpty()) {
        room = roomStr.toInt(&ok);
        if (!ok) {
            room = 0; // Default to 0 if not valid
        }
    }

    // Convertir floor - si ce n'est pas un nombre, utiliser 0
    int floor = 0;
    if (!floorStr.isEmpty()) {
        floor = floorStr.toInt(&ok);
        if (!ok) {
            floor = 0; // Default to 0 if not valid
        }
    }

    AppartementRecord rec;
    rec.id = id;
    rec.room = room;
    rec.floor = floor;
    rec.condition = condition;
    rec.status = status;
    rec.cin = cinStr;  // CIN du résident

    qDebug() << "[onModifyAppartementClicked] Updating Appartement ID:" << id << "with CIN:" << cinStr << "Room:" << room << "Floor:" << floor;

    if (AppartementCRUD::updateAppartement(id, rec)) {
        QMessageBox::information(this, "Succès", "Appartement ID " + QString::number(id) + " modifié avec succès !");
        refreshAppartementTable();
        ui->searchRoomEdit->clear();
        ui->cinmod->clear();
        ui->lineEdit_51->clear();
        ui->lineEdit_55->clear();
        ui->lineEdit_113->clear();
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de la modification de l'appartement ID " + QString::number(id) + ". Vérifiez que l'ID existe dans la BD !");
    }
}

void GResident::onSearchAppartementClicked()
{
    // Search not fully implemented yet - just refresh all
    refreshAppartementTable();
}

void GResident::onSearchAppartementForModifyClicked()
{
    // Rechercher un appartement par ID pour le remplir dans le MODIFY tab
    QString idStr = ui->searchRoomEdit->text().trimmed();

    if (idStr.isEmpty()) {
        QMessageBox::warning(this, "Attention", "Veuillez entrer l'ID de l'appartement à chercher.");
        return;
    }

    // Convertir l'ID
    bool ok;
    qint64 id = idStr.toLongLong(&ok);
    if (!ok) {
        id = 0; // Default to 0 if not a number
    }

    qDebug() << "[onSearchAppartementForModifyClicked] Searching for Appartement ID:" << id;

    // Récupérer les données de l'appartement depuis la BD
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);

    // Use string concatenation instead of bindValue for Oracle compatibility
    QString sql = QString("SELECT ID, ROOM, FLOOR, CONDITION, STATUS FROM APPARTEMENT WHERE ID = %1").arg(id);
    qDebug() << "[onSearchAppartementForModifyClicked] SQL:" << sql;

    if (query.exec(sql)) {
        if (query.next()) {
            // Remplir les champs avec les données trouvées
            ui->lineEdit_51->setText(query.value(1).toString());  // ROOM
            ui->lineEdit_55->setText(query.value(2).toString());  // FLOOR
            ui->lineEdit_113->setText(query.value(3).toString()); // CONDITION
            ui->addStatusCombo_3->setCurrentText(query.value(4).toString()); // STATUS
            qDebug() << "[onSearchAppartementForModifyClicked] Appartement found!";
            QMessageBox::information(this, "Succès", "Appartement ID " + idStr + " trouvé !");
        } else {
            qDebug() << "[onSearchAppartementForModifyClicked] No appartement found with ID:" << id;
            QMessageBox::warning(this, "Erreur", "Aucun appartement trouvé avec l'ID: " + idStr);
            ui->lineEdit_51->clear();
            ui->lineEdit_55->clear();
            ui->lineEdit_113->clear();
        }
    } else {
        qDebug() << "[onSearchAppartementForModifyClicked] Query failed:" << query.lastError().text();
        QMessageBox::critical(this, "Erreur BD", "Erreur lors de la recherche: " + query.lastError().text());
    }
}

// ========== VEHICLE CRUD Implementations ==========

void GResident::refreshVehicleTable()
{
    if (!ui->tableWidgetVehicule) return;

    // Block signals for faster update
    ui->tableWidgetVehicule->blockSignals(true);

    // Get model from Vehicle
    QSqlQueryModel *model = Vehicle::afficher();

    // Clear and setup
    ui->tableWidgetVehicule->setRowCount(0);
    ui->tableWidgetVehicule->setColumnCount(9);
    ui->tableWidgetVehicule->setHorizontalHeaderLabels(QStringList()
                                                       << "Matricule" << "CIN" << "Couleur" << "Type" << "Marque" << "Propriétaire"
                                                       << "Début" << "Fin" << "Badge");

    // Fill the table with data
    int rowCount = model->rowCount();
    ui->tableWidgetVehicule->setRowCount(rowCount);

    for (int row = 0; row < rowCount; ++row) {
        for (int col = 0; col < 9; ++col) {
            QString value = model->data(model->index(row, col)).toString();
            ui->tableWidgetVehicule->setItem(row, col, new QTableWidgetItem(value));
        }
    }

    // Re-enable signals
    ui->tableWidgetVehicule->blockSignals(false);
}

void GResident::onAddVehicleClicked()
{
    // Pour Vehicle ADD tab - read all fields
    QString matricule = ui->matricule->text().trimmed();
    QString cin = ui->cin->text().trimmed();
    QString couleur = ui->couleur->text().trimmed();
    QString typeVehicule = ui->type->text().trimmed();
    QString marque = ui->marque->text().trimmed();
    QString proprietaire = ui->proprietaire->text().trimmed();
    QDate dateDebut = ui->dateEdit_5->date();
    QDate dateFin = ui->badgedate->date();
    int badgeActif = ui->badge->isChecked() ? 1 : 0;

    Vehicle v(matricule, cin, couleur, typeVehicule, marque, proprietaire, dateDebut, dateFin, badgeActif);
    if (v.ajouter()) {
        QMessageBox::information(this, "Succès", "Véhicule ajouté !");
        refreshVehicleTable();
        ui->matricule->clear();
        ui->cin->clear();
        ui->couleur->clear();
        ui->type->clear();
        ui->marque->clear();
        ui->proprietaire->clear();
    } else {
        QMessageBox::warning(this, "Erreur", "Erreur lors de l'ajout !");
    }
}

void GResident::onDeleteVehicleClicked()
{
    QString matricule = ui->deletemat->text().trimmed();
    if (matricule.isEmpty()) {
        QMessageBox::warning(this, "Attention", "Entrez le matricule!");
        return;
    }

    if (Vehicle::supprimer(matricule)) {
        QMessageBox::information(this, "Succès", "Supprimé!");
        ui->deletemat->clear();
        // Only refresh if on DISPLAY tab to avoid lag
        if (ui->stackedWidget->currentIndex() == 4) {
            refreshVehicleTable();
        }
    }
}

void GResident::onModifyVehicleClicked()
{
    QString matricule = ui->modmatricule->text().trimmed();
    QString cin = ui->modcin->text().trimmed();
    QString couleur = ui->modcoleur->text().trimmed();
    QString typeVehicule = ui->modtype->text().trimmed();
    QString marque = ui->modmarque->text().trimmed();
    QString proprietaire = ui->modproprietaire->text().trimmed();
    QDate dateDebut = ui->dateEdit_11->date();
    QDate dateFin = ui->dateEdit_12->date();
    int badgeActif = ui->checkBox_4->isChecked() ? 1 : 0;

    Vehicle v(matricule, cin, couleur, typeVehicule, marque, proprietaire, dateDebut, dateFin, badgeActif);
    if (v.modifier(matricule)) {
        QMessageBox::information(this, "Succès", "Véhicule modifié !");
        if (ui->stackedWidget->currentIndex() == 4) {
            refreshVehicleTable();
        }
        ui->modmatricule->clear();
        ui->modcin->clear();
        ui->modcoleur->clear();
        ui->modtype->clear();
        ui->modmarque->clear();
        ui->modproprietaire->clear();
    } else {
        QMessageBox::warning(this, "Erreur", "Erreur lors de la modification !");
    }
}

void GResident::onSearchVehicleClicked()
{
    QString searchText = ui->lineEdit_63->text().trimmed();
    QSqlQueryModel *model = Vehicle::rechercher(searchText.isEmpty() ? "%" : searchText);

    ui->tableWidgetVehicule->setRowCount(0);
    ui->tableWidgetVehicule->setColumnCount(9);
    ui->tableWidgetVehicule->setHorizontalHeaderLabels(QStringList()
                                                       << "Matricule" << "CIN" << "Couleur" << "Type" << "Marque" << "Propriétaire"
                                                       << "Début" << "Fin" << "Badge");

    int rowCount = model->rowCount();
    ui->tableWidgetVehicule->setRowCount(rowCount);

    for (int row = 0; row < rowCount; ++row) {
        for (int col = 0; col < 9; ++col) {
            QModelIndex index = model->index(row, col);
            QString value = model->data(index).toString();
            ui->tableWidgetVehicule->setItem(row, col, new QTableWidgetItem(value));
        }
    }
}

void GResident::on_exportPdfBtn_clicked()
{
    qDebug() << "=== BOUTON EXPORT PDF CLIQUE ===";
    QMessageBox::information(this, "Debug", "Bouton PDF cliqué !");
    AppartementCRUD::exporterListeAppartementsPDF(this);
}

void GResident::on_sortBtn_clicked()
{
    // Demander le type de tri à l'utilisateur
    QStringList sortOptions;
    sortOptions << "Trier par ID" << "Trier par Étage" << "Trier par Numéro" << "Trier par Statut";

    bool ok;
    QString sortChoice = QInputDialog::getItem(this,
                                               "Choix du tri",
                                               "Comment voulez-vous trier les appartements ?",
                                               sortOptions, 0, false, &ok);

    if (!ok) {
        return; // L'utilisateur a annulé
    }

    // Déterminer l'ordre SQL en fonction du choix
    QString orderBy;
    if (sortChoice == "Trier par ID") {
        orderBy = "ID";
    } else if (sortChoice == "Trier par Étage") {
        orderBy = "FLOOR, ROOM";
    } else if (sortChoice == "Trier par Numéro") {
        orderBy = "ROOM, FLOOR";
    } else { // Trier par Statut
        orderBy = "STATUS, FLOOR, ROOM";
    }

    // Rafraîchir la table avec le tri choisi
    if (!ui->appartementTableView) return;

    // Supprimer l'ancien modèle
    if (ui->appartementTableView->model()) {
        delete ui->appartementTableView->model();
    }

    // Créer un nouveau modèle avec le tri
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);

    QString sql = QString("SELECT ID, ROOM, FLOOR, CONDITION, STATUS, CIN FROM APPARTEMENT ORDER BY %1").arg(orderBy);

    if (!query.exec(sql)) {
        qDebug() << "[on_sortBtn_clicked] ERROR executing query:" << query.lastError().text();
        delete model;
        return;
    }

    model->setQuery(std::move(query));

    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Room");
    model->setHeaderData(2, Qt::Horizontal, "Floor");
    model->setHeaderData(3, Qt::Horizontal, "Condition");
    model->setHeaderData(4, Qt::Horizontal, "Status");
    model->setHeaderData(5, Qt::Horizontal, "CIN");

    ui->appartementTableView->setModel(model);
    ui->appartementTableView->resizeColumnsToContents();

    QMessageBox::information(this, "Tri appliqué",
                             QString("Les appartements sont maintenant triés par: %1").arg(sortChoice));
}

// ========== SMARTAVAILABILITY Implementations ==========

void GResident::onSmartAvailabilityClicked()
{
    qDebug() << "[onSmartAvailabilityClicked] Launching SmartAvailability dialog...";
    SmartAvailability dialog(this);
    dialog.exec();
}

void GResident::onStatsAppartementsOccupees()
{
    qDebug() << "[onStatsAppartementsOccupees] Showing short apartment overview...";

    QSqlQuery query(QSqlDatabase::database());

    // Get totals
    int total = 0;
    if (query.exec("SELECT COUNT(*) FROM APPARTEMENT") && query.next()) {
        total = query.value(0).toInt();
    }

    // Get heuristic vacancies: check empty CIN or status containing 'vac'
    int vacant = 0;
    if (query.exec("SELECT COUNT(*) FROM APPARTEMENT WHERE (CIN IS NULL OR TRIM(CIN) = '' OR STATUS IS NULL OR TRIM(STATUS) = '' OR LOWER(STATUS) LIKE '%vac%')") && query.next()) {
        vacant = query.value(0).toInt();
    }

    int occupied = total - vacant;
    double occupancyRate = total > 0 ? (double)occupied / total * 100.0 : 0.0;

    // Small, clear overview for the STATISTICS button:
    // Only show: total, free (libre), active (occupés), occupancy rate — with emoji.
    QString summary;
    summary += QString("🏠 Total appartements: %1\n").arg(total);
    summary += QString("🟢 Libre: %1\n").arg(vacant);
    summary += QString("🔴 Occupés: %1\n").arg(occupied);
    summary += QString("📈 Taux d'occupation: %1%\n").arg(QString::number(occupancyRate, 'f', 1));

    // Short, clean UI — small overview message box with option to see more details
    QMessageBox msg(this);
    msg.setIcon(QMessageBox::Information);
    msg.setWindowTitle("STATISTICS — Aperçu");
    msg.setText(summary);
    QPushButton *detailsBtn = msg.addButton("Voir les détails", QMessageBox::ActionRole);
    msg.addButton(QMessageBox::Ok);
    msg.exec();

    if (msg.clickedButton() == detailsBtn) {
        // User wants the full SmartAvailability analysis
        SmartAvailability dialog(this);
        dialog.exec();
    }
}

// ========== RECHERCHE DYNAMIQUE APPARTEMENT Implementations ==========

// ========== RECHERCHE DYNAMIQUE APPARTEMENT - VERSION AMÉLIORÉE ==========

void GResident::on_searchAppartementTextChanged(const QString &text)
{
    Q_UNUSED(text)

    // Vérifier si nous sommes sur la page des appartements
    if (ui->stackedWidget->currentIndex() == 3) { // Page Appartement = index 3
        qDebug() << "[Recherche] Texte changé, démarrage du timer...";
        searchAppartementTimer->start();
    }
}
void GResident::on_searchAppartementDynamic()
{
    QString searchText = ui->lineEdit_63->text().trimmed();
    qDebug() << "[on_searchAppartementDynamic] Recherche dynamique avec:" << searchText;

    if (!ui->appartementTableView) {
        qDebug() << "[on_searchAppartementDynamic] ERREUR: appartementTableView non trouvé!";
        return;
    }

    // Utiliser AppartementCRUD pour la recherche
    QString errorMsg;
    QSqlQueryModel *model = AppartementCRUD::searchAppartements(searchText, &errorMsg);

    if (model) {
        // Supprimer l'ancien modèle pour éviter les fuites mémoire
        QAbstractItemModel *oldModel = ui->appartementTableView->model();
        if (oldModel && oldModel != model) {
            delete oldModel;
        }

        ui->appartementTableView->setModel(model);
        ui->appartementTableView->resizeColumnsToContents();

        qDebug() << "[on_searchAppartementDynamic] SUCCÈS -" << model->rowCount() << "résultats";
    } else {
        qDebug() << "[on_searchAppartementDynamic] ERREUR:" << errorMsg;
        QMessageBox::warning(this, "Erreur", "Erreur lors de la recherche: " + errorMsg);
    }
}

// Triggered by the apartments-specific search input in the modify tab
void GResident::on_searchRoomEditTextChanged(const QString &text)
{
    Q_UNUSED(text)
    // Debounce like the main search field
    if (ui->stackedWidget->currentIndex() == 3) {
        searchAppartementTimer->start();
    }
}

void GResident::on_searchRoomBtn3Clicked()
{
    // Read the search text from the apartment-specific search field
    // Prefer the general search input `searchEdit`, then `searchRoomEdit`, then fallback to the global lineEdit_63
    QString searchText;
    if (ui->searchEdit && !ui->searchEdit->text().trimmed().isEmpty()) {
        searchText = ui->searchEdit->text().trimmed();
    } else if (ui->searchRoomEdit && !ui->searchRoomEdit->text().trimmed().isEmpty()) {
        searchText = ui->searchRoomEdit->text().trimmed();
    } else {
        searchText = ui->lineEdit_63 ? ui->lineEdit_63->text().trimmed() : QString();
    }

    // If empty, use the global apartment search field
    if (searchText.isEmpty()) {
        searchText = ui->lineEdit_63 ? ui->lineEdit_63->text().trimmed() : QString();
    }

    // Navigate to the apartments display page (index 3)
    navigateToPage(3);

    // If no search text, just refresh the table and exit
    if (searchText.isEmpty()) {
        refreshAppartementTable();
        return;
    }

    // Do the search and show results
    QString errorMsg;
    qDebug() << "[on_searchRoomBtn3Clicked] searchText:" << searchText;
    QSqlQueryModel *model = AppartementCRUD::searchAppartements(searchText, &errorMsg);
    if (model) {
        QAbstractItemModel *oldModel = ui->appartementTableView->model();
        if (oldModel && oldModel != model) delete oldModel;
        ui->appartementTableView->setModel(model);
        ui->appartementTableView->resizeColumnsToContents();
        qDebug() << "[on_searchRoomBtn3Clicked] Results found:" << model->rowCount();
        if (model->rowCount() == 0) {
            QMessageBox::information(this, "Recherche", "Aucun appartement trouvé pour: " + searchText);
        }
    } else {
        QMessageBox::warning(this, "Recherche", "Aucun résultat: " + errorMsg);
    }
}

// ========== VOICE INPUT IMPLEMENTATION ==========

void GResident::onSpeechToTextButtonClicked()
{
    if (!m_voiceToText) {
        return;
    }

    if (!m_voiceToText->hasApiKey()) {
        const QString apiKey = resolveHumeApiKey();
        if (!apiKey.isEmpty()) {
            m_voiceToText->setApiKey(apiKey);
        } else {
            const QString fileKey = resolveHumeApiKeyFromFile();
            if (!fileKey.isEmpty()) {
                m_voiceToText->setApiKey(fileKey);
            }
        }
    }

    if (!m_voiceToText->hasApiKey()) {
        QMessageBox::warning(this,
                             tr("Hume API Key"),
                             tr("Aucune clé API Hume trouvée. Définissez la variable d'environnement HUME_API_KEY."));
        return;
    }

    if (m_voiceToText->isRecording()) {
        m_voiceToText->stop();
    } else {
        if (!m_voiceToText->start()) {
            QMessageBox::warning(this, tr("Enregistrement"), tr("Impossible de démarrer l'enregistrement."));
        }
    }
}

void GResident::handleVoiceFinalMessage(const QString &role, const QString &text)
{
    m_pendingInterim.clear();
    const QString prefix = role.compare(QStringLiteral("assistant"), Qt::CaseInsensitive) == 0
                               ? tr("Assistant")
                               : tr("Utilisateur");
    appendTranscriptLine(prefix, text);
}

void GResident::handleVoiceInterimMessage(const QString &text)
{
    m_pendingInterim = text;
    if (ui->voiceChatLabel_2) {
        ui->voiceChatLabel_2->setText(tr("Écoute… %1").arg(text));
    }
}

void GResident::handleVoiceStatusChanged(const QString &status)
{
    if (ui->voiceChatLabel_2) {
        ui->voiceChatLabel_2->setText(status);
    }

    if (ui->recordingstart) {
        const bool recording = m_voiceToText && m_voiceToText->isRecording();
        ui->recordingstart->setText(recording ? tr("Stop recording") : tr("Start recording"));
    }
}

void GResident::handleVoiceError(const QString &error)
{
    if (ui->voiceChatLabel_2) {
        ui->voiceChatLabel_2->setText(error);
    }
    QMessageBox::warning(this, tr("Hume"), error);
}

void GResident::appendTranscriptLine(const QString &prefix, const QString &text)
{
    if (!ui->speechtotext) {
        return;
    }

    const QString sanitized = text.trimmed();
    if (sanitized.isEmpty()) {
        return;
    }

    ui->speechtotext->append(QStringLiteral("[%1] %2").arg(prefix, sanitized));
    if (ui->speechtotext->verticalScrollBar()) {
        ui->speechtotext->verticalScrollBar()->setValue(ui->speechtotext->verticalScrollBar()->maximum());
    }
}

void GResident::setHumeApiKeyDialog()
{
    bool ok = false;
    QString key = QInputDialog::getText(this,
                                        tr("Set Hume API Key"),
                                        tr("Enter your Hume API key (app will save it to ~/.hume_api_key):"),
                                        QLineEdit::Password,
                                        QString(),
                                        &ok);
    if (!ok || key.isEmpty()) return;

    // Set to runtime instance
    if (m_voiceToText) {
        m_voiceToText->setApiKey(key.trimmed());
    }

    // Save to file for persistent local usage - do not commit this file!
    QString path = QCoreApplication::applicationDirPath() + "/.hume_api_key";
    QFile f(path);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&f);
        out << key.trimmed() << Qt::endl;
        f.close();
        QMessageBox::information(this, tr("Saved"), tr("Hume API key stored in %1").arg(path));
    } else {
        // try in home dir as fallback
        QString homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.hume_api_key";
        QFile f2(homePath);
        if (f2.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&f2);
            out << key.trimmed() << Qt::endl;
            f2.close();
            QMessageBox::information(this, tr("Saved"), tr("Hume API key stored in %1").arg(homePath));
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Could not save the key locally. Please set the HUME_API_KEY env var."));
        }
    }
}

QString GResident::resolveHumeApiKey() const
{
    QString key = qEnvironmentVariable("HUME_API_KEY");
    if (key.isEmpty()) {
        key = qEnvironmentVariable("HUME_API_TOKEN");
    }
    return key.trimmed();
}

// Try additional secure local fallback (file not tracked by git). This helps
// development without committing secrets. The file is searched in the app
// directory first, then user home (e.g., ~/.hume_api_key).
QString GResident::resolveHumeApiKeyFromFile() const
{
    const QString candidateFiles[] = {
        QCoreApplication::applicationDirPath() + "/.hume_api_key",
        QCoreApplication::applicationDirPath() + "/hume_api_key",
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.hume_api_key",
    };

    for (const QString &fname : candidateFiles) {
        QFile f(fname);
        if (!f.exists()) continue;
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) continue;
        QTextStream in(&f);
        const QString key = in.readLine().trimmed();
        f.close();
        if (!key.isEmpty()) return key;
    }

    return QString();
}

