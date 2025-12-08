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

#include <QMessageBox>
#include <QTextDocument>
#include <QPrinter>
#include <QFileDialog>
#include <QFileInfo>
#include <QTextStream>
#include <QPainter>
#include <QChartView>
#include <QPieSlice>
#include <QPieSeries>
#include <QDebug>
#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include "simpleqrcode.h"

GResident::GResident(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::GResident),
    searchAppartementTimer(new QTimer(this))
{
    ui->setupUi(this);  // ✅ TOUJOURS EN PREMIER

    //arduino
    if (A.connect_arduino() == 0) {

        QMessageBox::warning(this, "Port", A.getarduino_port_name());

    } else {

        QMessageBox::warning(this, "Erreur", "Échec de la connexion à l'Arduino.");

    }
    connect(A.get_serial(), &QSerialPort::readyRead, this, &GResident::read_arduino);






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
        connect(ui->appartement, &QPushButton::clicked, this, []() {
            // SmartAvailability can be launched manually from UI
        });
    }

    // ✅ Start on first page (index 0)
    ui->stackedWidget->setCurrentIndex(0);




    //Service
    if (ui->tableView_service) {
        ui->tableView_service->setModel(se.afficher());
    }
    if (ui->date_service) {
        ui->date_service->setDate(QDate::currentDate());
    }
    if (ui->date_service_mod) {
        ui->date_service_mod->setDate(QDate::currentDate());
    }

    QIntValidator *intValidator = new QIntValidator(0, 100000, this); // Only allows integers from 0 to 100

    if (ui->id_service) {
        ui->id_service->setValidator(intValidator);
    }
    if (ui->supprimer_id_service) {
        ui->supprimer_id_service->setValidator(intValidator);
    }

    if (ui->code_promo_mod) {
        ui->code_promo_mod->hide();
    }

}

GResident::~GResident()
{
    delete ui;
}

//arduino

void GResident::read_arduino()
{
    QByteArray data = A.read_from_arduino();
    QString uid = QString::fromUtf8(data).trimmed(); // Convert data to QString
    //qDebug()<<uid;
    if (!uid.isEmpty())
    {
        if (uid != "*")
        {
            A.code += uid; // Accumulate code
           // qDebug()<<A.code;
        }
        else
        {
            A.code.remove("+"); // Remove '/' from the accumulated code
            qDebug()<<A.code;
            QSqlQuery query;
            query.prepare("SELECT ID_EMPLOYE FROM EMPLOYE WHERE ID_EMPLOYE = :code");
            query.bindValue(":code", A.code);

            if (query.exec() && query.next())
            {
                int id = query.value(0).toInt(); // UID exists in the database
                if(id==0)
                {
                    QByteArray endByteArray = "#";
                    A.write_to_arduino(endByteArray); // Send the '/' character
                    qDebug()<<"#";

                }
                else
                {


                // Prepare query to count commandes for the supplier
                QSqlQuery countQuery;
                countQuery.prepare("UPDATE EMPLOYE SET PRESENCE='present'  WHERE ID_EMPLOYE = :id");
                countQuery.bindValue(":id", id);
                QSqlQuery nameQuery;
                nameQuery.prepare("SELECT NOM FROM EMPLOYE WHERE ID_EMPLOYE=:id");
                nameQuery.bindValue(":id",id);


                if (countQuery.exec() &&  nameQuery.exec() && nameQuery.next())
                {
                    QString name = nameQuery.value(0).toString(); // Get count
                    QByteArray dataToSend = name.toUtf8(); // Convert QString to QByteArray
                    qDebug() << name;
                    Employe e;
                    ui->employetableView->setModel(e.afficher());

                    // Send each character to the Arduino
                    for (char c : dataToSend)
                    {
                        QByteArray byteArray;
                        byteArray.append(c); // Create a QByteArray for the character
                        A.write_to_arduino(byteArray); // Send the character to Arduino
                    }

                    // Send the termination character '/'
                    QByteArray endByteArray = "/";
                    A.write_to_arduino(endByteArray); // Send the '/' character
                    qDebug()<<"/";
                }

                 }
            }
            else
            {
                QMessageBox::warning(this, "Erreur", "L'UID n'existe pas !");
            }

            // Reset code after processing
            A.code.clear();
        }
    }
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

    //fix it back when you add the smartAvailability
    qDebug() << "[onSmartAvailabilityClicked] Launching SmartAvailability dialog...";
   // SmartAvailability dialog(this);
    //  dialog.exec();
}

void GResident::onStatsAppartementsOccupees()
{
    qDebug() << "[onStatsAppartementsOccupees] Showing apartment statistics with chart...";

    // Show the statistics dialog with chart
    StatisticsDialog statsDialog(this);
    statsDialog.exec();
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


// ========== SERVICE CRUD Implementations ==========


void GResident::on_bt_ajouter_service_clicked()
{
    int id_service=ui->id_service->text().toInt();
    QString name=ui->name_service->text();
    QString status=ui->status_service->currentText();
    QString type=ui->type_service->currentText();
    int prix=ui->prix_service->value();
    QDate date_service=ui->date_service->date();
    //Controle de Saisie !
    if(se.idExists(id_service))
    {
        QMessageBox::critical(nullptr, QObject::tr(""),
                    QObject::tr("ID deja existe  ."), QMessageBox::Cancel);
        return;
    }
    if(id_service==0 ||name==""|| prix==0)
    {
        QMessageBox::critical(nullptr, QObject::tr(""),
                    QObject::tr("Tu dois remplir tous les champs ."), QMessageBox::Cancel);
        return;
    }
    if(date_service<QDate::currentDate())
    {
        QMessageBox::critical(nullptr, QObject::tr(""),
                    QObject::tr("La date ne doit pas etre au passé."), QMessageBox::Cancel);
        return;
    }
    service e(id_service,name,status,type,prix,date_service);
    bool test=e.ajouter();
    if(test)
    {
        QMessageBox::information(nullptr, QObject::tr(""),
                    QObject::tr("Ajout avec succées ."), QMessageBox::Cancel);
        ui->tableView_service->setModel(se.afficher());
        ui->date_service->setDate(QDate::currentDate());
        ui->date_service_mod->setDate(QDate::currentDate());
        ui->id_service->clear();
        ui->name_service->clear();
        ui->prix_service->clear();
    }
    else
    {
        QMessageBox::critical(nullptr, QObject::tr(""),
                    QObject::tr("Ajout échoué ."), QMessageBox::Cancel);
    }

}

void GResident::on_bt_update_service_clicked()
{
    {
        int id_service=ui->id_service_mod->text().toInt();
        QString name=ui->name_service_mod->text();
        QString status=ui->status_service_mod->currentText();
        QString type=ui->type_service_mod->currentText();
        int prix=ui->prix_service_mod->value();
        QDate date_service=ui->date_service_mod->date();

        //Controle de Saisie !
        if(!se.idExists(id_service))
        {
            QMessageBox::critical(nullptr, QObject::tr(""),
                        QObject::tr("ID n'existe pas  ."), QMessageBox::Cancel);
            return;
        }
        if(id_service==0 ||name==""|| prix==0)
        {
            QMessageBox::critical(nullptr, QObject::tr(""),
                        QObject::tr("Tu dois remplir tous les champs ."), QMessageBox::Cancel);
            return;
        }


        service e(id_service,name,status,type,prix,date_service);
        bool test=e.modifier();
        if(test)
        {
            QMessageBox::information(nullptr, QObject::tr(""),
                        QObject::tr("Modification avec succées ."), QMessageBox::Cancel);
            ui->tableView_service->setModel(se.afficher());
            ui->date_service->setDate(QDate::currentDate());
            ui->date_service_mod->setDate(QDate::currentDate());
            ui->id_service_mod->clear();
            ui->name_service_mod->clear();
            ui->prix_service_mod->clear();
        }
        else
        {
            QMessageBox::critical(nullptr, QObject::tr(""),
                        QObject::tr("Modification échoué ."), QMessageBox::Cancel);
        }

    }

}

void GResident::on_tableView_service_clicked(const QModelIndex &index)
{
    QAbstractItemModel* model = ui->tableView_service->model();

        int row = index.row();



        QString ID_SERVICE = model->data(model->index(row, 0)).toString();
        QString NAME = model->data(model->index(row, 1)).toString();
        QString STATUS = model->data(model->index(row, 2)).toString();
        QString TYPE = model->data(model->index(row, 3)).toString();
        float PRIX = model->data(model->index(row, 4)).toFloat();
        QDate DATE_SERVICE = model->data(model->index(row, 5)).toDate();
        QString code_promo =model->data(model->index(row, 6)).toString();

ui->id_service_mod->setText(ID_SERVICE);
ui->name_service_mod->setText(NAME);
ui->status_service_mod->setCurrentText(STATUS);
ui->prix_service_mod->setValue(PRIX);
ui->date_service_mod->setDate(DATE_SERVICE);
ui->type_service_mod->setCurrentText(TYPE);
ui->code_promo_mod->setText(code_promo);



}

void GResident::on_bt_delete_service_clicked()
{

// Note: Changed from supprimer_id_service (QPushButton) to bt_delete_service (QLineEdit)
// If supprimer_id_service exists as QLineEdit in .ui, use that instead
int id=ui->supprimer_id_service->text().toInt();
if(!se.idExists(id))
{
    QMessageBox::critical(nullptr, QObject::tr(""),
                               QObject::tr("id n'existe pas ."), QMessageBox::Cancel);
}
else
{
bool test=se.supprimer(id);
 QMessageBox msgBox;
if(test)
{
    {QMessageBox::information(nullptr, QObject::tr(""),
                           QObject::tr("suppresion avec succes."), QMessageBox::Cancel);}
           ui->tableView_service->setModel(se.afficher());
           stat();
     }
     else
         {QMessageBox::critical(nullptr, QObject::tr(""),
                                QObject::tr("echec de suppresion."), QMessageBox::Cancel);}

}
}

void GResident::on_btn_tri_clicked()
{
QString choix=ui->choix_service->currentText();
QString ordre=ui->ordre_service->currentText();

if(ordre=="Tri: ↑")
{
    ordre="ASC";
}
else
    ordre="DESC";

ui->tableView_service->setModel(se.tri(choix,ordre));
}

void GResident::on_Search_service_textChanged(const QString &text)
{
    QString choix=ui->choix_service->currentText();
    ui->tableView_service->setModel(se.chercher(choix,text));


}

void GResident::on_bt_pdf_service_clicked()
{
    QString strStream;
    QTextStream out(&strStream);

    const int rowCount = ui->tableView_service->model()->rowCount();
    const int columnCount = ui->tableView_service->model()->columnCount();

    out << "<html>\n"
           "<head>\n"
           "<meta Content=\"Text/html; charset=Windows-1251\">\n"
           "<title>%1</title>\n"
           "<style>\n"
           "table {\n"
           "    width: 100%;\n"
           "    border-collapse: collapse;\n"
           "}\n"
           "th, td {\n"
           "    padding: 8px;\n"
           "    text-align: left;\n"
           "    border-bottom: 1px solid #ddd;\n"
           "}\n"
           "tr:nth-child(even) {\n"
           "    background-color: #f2f2f2;\n"
           "}\n"
           "</style>\n"
           "</head>\n"
           "<body bgcolor=#ffffff link=#5000A0>\n"
           "<center> <H1>Liste des Services</H1></center><br/><br/>\n"
           "<img src=\"path/to/your/image.jpg\" alt=\"Description of image\" style=\"max-width: 100%; height: auto;\">\n"
           "<table>\n";

    // headers
    out << "<thead><tr bgcolor=#f0f0f0> <th>Numero</th>";
    for (int column = 0; column < columnCount; column++)
    {
        if (!ui->tableView_service->isColumnHidden(column))
        {
            out << QString("<th>%1</th>").arg(ui->tableView_service->model()->headerData(column, Qt::Horizontal).toString());
        }
    }
    out << "</tr></thead>\n";

    // data table
    for (int row = 0; row < rowCount; row++)
    {
        out << "<tr> <td>" << row + 1 << "</td>";
        for (int column = 0; column < columnCount; column++)
        {
            if (!ui->tableView_service->isColumnHidden(column))
            {
                QString data = ui->tableView_service->model()->data(ui->tableView_service->model()->index(row, column)).toString().simplified();
                out << QString("<td>%1</td>").arg((!data.isEmpty()) ? data : QString("&nbsp;"));
            }
        }
        out << "</tr>\n";
    }



    QString fileName = QFileDialog::getSaveFileName((QWidget *)0, "Sauvegarder en PDF", QString(), "*.pdf");
    if (QFileInfo(fileName).suffix().isEmpty())
    {
        fileName.append(".pdf");
    }

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPageSize::A4);
    printer.setOutputFileName(fileName);

    QTextDocument doc;
    doc.setHtml(strStream);
    doc.print(&printer);

}
void GResident::on_bt_stat_service_clicked()
{
        QMap<QString, int> stats = se.statistiquesParType();

        QPieSeries *series = new QPieSeries();
        //QPieSlice
        for (auto it = stats.begin(); it != stats.end(); ++it) {
            series->append(it.key(), it.value());
        }

        QChart *chart = new QChart();
        chart->addSeries(series);
        chart->setTitle("Statistiques des Services par type");

        QChartView *chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);

        QDialog *chartDialog = new QDialog(this);
        chartDialog->setWindowTitle("Graphique des Statistiques");
        chartDialog->setFixedSize(480, 240);
        QVBoxLayout *layout = new QVBoxLayout();
        layout->addWidget(chartView);
        chartDialog->setLayout(layout);

        chartDialog->exec();
}

void GResident::on_bt_code_promo_clicked()
{
    int id_service = ui->id_service_mod->text().toInt();
    if(id_service==0)
    {
        QMessageBox::critical(nullptr, QObject::tr(""),
                    QObject::tr("Tu dois choisir une service d'apres le tableau  ."), QMessageBox::Cancel);
        return;
    }
    // Create a new dialog for input
    QDialog *inputDialog = new QDialog(this);
    inputDialog->setWindowTitle("Code Promo");
    inputDialog->setFixedSize(300, 150); // Set a fixed size if desired

    // Create a line edit
    QLineEdit *lineEdit = new QLineEdit(inputDialog);
    lineEdit->setPlaceholderText("Enter your code here");

    // Create buttons
    QPushButton *editButton = new QPushButton("Edit", inputDialog);
    QPushButton *deleteButton = new QPushButton("Delete", inputDialog);

    // Capture id_service and lineEdit in the lambda
    connect(editButton, &QPushButton::clicked, this, [this, lineEdit, id_service]() {
        QString text = lineEdit->text();
        if (!text.isEmpty()) {
            bool test = se.EditCodePromo(id_service, text);
            if (test) {
                QMessageBox::information(this, "Edit", "Edited: " + text);
            } else {
                QMessageBox::warning(this, "Edit Error", "Failed to edit the code promo.");
            }
            ui->tableView_service->setModel(se.afficher());
            lineEdit->clear();
        }
    });

    connect(deleteButton, &QPushButton::clicked, this, [this, lineEdit, id_service]() {
        QString text = lineEdit->text();
        if (!text.isEmpty()) {
            bool test = se.DeleteCodePromo(id_service);
            if (test) {
                QMessageBox::information(this, "Delete", "Deleted: " + text);
            } else {
                QMessageBox::warning(this, "Delete Error", "Failed to delete the code promo.");
            }
            ui->tableView_service->setModel(se.afficher());
            lineEdit->clear();
        }
    });

    // Create a layout for the dialog
    QVBoxLayout *layout = new QVBoxLayout(inputDialog);
    layout->addWidget(lineEdit);
    layout->addWidget(editButton);
    layout->addWidget(deleteButton);

    // Set layout to the dialog
    inputDialog->setLayout(layout);

    // Show the dialog
    inputDialog->exec();
}



void GResident::on_bt_Code_Qr_clicked()
{
    int id_service = ui->id_service_mod->text().toInt();
    if(id_service==0)
    {
        QMessageBox::critical(nullptr, QObject::tr(""),
                    QObject::tr("Tu dois choisir une service d'apres le tableau  ."), QMessageBox::Cancel);
        return;
    }

    QString name=ui->name_service_mod->text();
    QString status=ui->status_service_mod->currentText();
    QString type=ui->type_service_mod->currentText();
    int prix=ui->prix_service_mod->value();
    //QDate date_service=ui->date_service_mod->date();
    QString code_promo = ui->code_promo_mod->text();

// Using SimpleQRCode to create a QR code from Facture attributes
                 QString text = "name: " + name + "\n"
                                 "TYPE: " + type + "\n"
                                 "PRIX: " + QString::number(prix) + "\n"
                                 "CODE PROMO: " +code_promo + "\n";

                 // Generate QR code using simple method (works with Qt 5.9.9)
                 QImage qrImage = SimpleQRCode::generateQRCode(text, 200);

                 // Create a new dialog
                    QDialog *qrDialog = new QDialog(this);
                    qrDialog->setWindowTitle("QR Code");
                    qrDialog->setFixedSize(220, 220); // Set a fixed size for the dialog

                    // Create a label to hold the QR code image
                    QLabel *qrLabel = new QLabel(qrDialog);

                    // Set the pixmap from the QR code image
                    qrLabel->setPixmap(QPixmap::fromImage(qrImage.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation)));

                    // Create a layout for the dialog
                    QVBoxLayout *layout = new QVBoxLayout(qrDialog);
                    layout->addWidget(qrLabel);

                    // Set the layout to the dialog
                    qrDialog->setLayout(layout);

                    // Show the dialog
                    qrDialog->exec();

}

void GResident::on_bt_Facture_clicked()
{
    int nb = se.NbTotaleDuClientAyantScanne();
    QMap<QString, float> remiseMap = se.PrixTotaleDuRemise();

    // Get the total price from the map
    float prixTotale = remiseMap.value("PrixTotale", 0); // Default to 0 if not found

    QString strStream;
    QTextStream out(&strStream);

    // Start HTML structure
    out << "<html>\n"
           "<head>\n"
           "<meta Content=\"Text/html; charset=Windows-1251\">\n"
           "<title>Facture</title>\n"
           "<style>\n"
           "body { font-family: Arial, sans-serif; margin: 20px; }\n"
           ".header { text-align: left; }\n"
           ".header h1 { margin: 0; }\n"
           ".invoice-details { margin: 20px 0; }\n"
           "table { width: 100%; border-collapse: collapse; margin: 20px 0; }\n"
           "th, td { padding: 10px; text-align: left; border: 1px solid #ddd; }\n"
           "th { background-color: #f2f2f2; }\n"
           ".total { font-weight: bold; font-size: 1.2em; }\n"
           ".footer { margin-top: 20px; text-align: left; font-size: 0.9em; }\n"
           "</style>\n"
           "</head>\n"
           "<body>\n";

    // Company Information
    out << "<div class='header'>\n"
           "<h1>Mon Entreprise</h1>\n"
           "<p>123 Rue de France</p>\n"
           "<p>75000 Paris, France</p>\n"
           "</div>\n";

    // Invoice Number and Date
    out << "<div class='invoice-details'>\n"
           "<p>Date de la facture: " << QDate::currentDate().toString("dd/MM/yyyy") << "</p>\n"
           "<p>Facture No: 2018-08-003</p>\n"  // Replace with dynamic invoice number if needed
           "<p>Reference de la facture: 2018-08-003</p>\n"
           "</div>\n";

    // Service Discounts Table
    out << "<table>\n"
           "<thead>\n"
           "<tr><th>Designation des produits</th><th>Quantites</th><th>Unites</th><th>Prix Unitaire HT</th><th>Total TTC</th></tr>\n"
           "</thead>\n"
           "<tbody>\n";

    // Iterate through the map to populate the table
    for (auto it = remiseMap.constBegin(); it != remiseMap.constEnd(); ++it) {
        if (it.key() != "PrixTotale") { // Skip the total price entry
            out << QString("<tr><td>%1</td><td>1</td><td>pc.</td><td>%2</td><td>%3</td></tr>\n")
                   .arg(it.key())
                   .arg(QString::number(it.value(), 'f', 2)) // Unit Price
                   .arg(QString::number(it.value(), 'f', 2)); // Total (same as unit price for 1 quantity)
        }
    }

    out << "</tbody>\n</table>\n";
    //Total Client scanne
    out << "<div class='total'>\n"
           "<p>Total Client Scanne: " << QString::number(nb) << " </p>\n"
           "</div>\n";
    // Total Price
    out << "<div class='total'>\n"
           "<p>Total Prix Remise: " << QString::number(prixTotale, 'f', 2) << " EUR</p>\n"
           "</div>\n";

    // Additional Information
    out << "<div class='footer'>\n"
           "<p>TVA non applicable, article 293 B du CGI</p>\n"
           "<p>Merci pour votre confiance!</p>\n"
           "</div>\n";

    out << "</body>\n</html>";

    // Save PDF
    QString fileName = QFileDialog::getSaveFileName((QWidget *)0, "Sauvegarder la facture", QString(), "*.pdf");
    if (QFileInfo(fileName).suffix().isEmpty())
    {
        fileName.append(".pdf");
    }

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPageSize::A4);
    printer.setOutputFileName(fileName);

    QTextDocument doc;
    doc.setHtml(strStream);
    doc.print(&printer);
}



void GResident::on_bt_quitter_appilication_clicked()
{
    // Create a message box for confirmation
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmation",
                                  "Do you really want to quit?",
                                  QMessageBox::Yes | QMessageBox::No);

    // Check the user's response
    if (reply == QMessageBox::Yes) {
        QSqlQuery query;
        query.prepare("UPDATE EMPLOYE SET PRESENCE='absent'");
        query.exec();
        qApp->quit();
    } else {

    }
}
