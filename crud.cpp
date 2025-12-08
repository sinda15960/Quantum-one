#include "crud.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QDebug>
#include <QVariant>
#include <QMetaType>
#include <QWidget>
#include <QDialog>
#include <QApplication>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextTable>
#include <QTextCharFormat>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QDir>
#include <QTextLength>
#include <QTextFrameFormat>
#include <QPainter>
#include <QTextStream>
#include <QDesktopServices>
#include <QUrl>
#include <QPrinter>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextTable>
#include <QTextTableFormat>
#include <QTextCharFormat>
#include <QColor>
#include <QPageSize>


void AppartementCRUD::logSqlError(const QString &operation, const QSqlError &error)
{
    qCritical() << "========================================";
    qCritical() << "[AppartementCRUD::" << operation << "] SQL ERROR";
    qCritical() << "Error Type:" << error.type();
    qCritical() << "Error Text:" << error.text();
    qCritical() << "Driver Text:" << error.driverText();
    qCritical() << "Database Text:" << error.databaseText();
    qCritical() << "Native Error Code:" << error.nativeErrorCode();
    qCritical() << "========================================";
}

bool AppartementCRUD::insertAppartement(const AppartementRecord &record, QString *errorMsg)
{
    qDebug() << "========================================";
    qDebug() << "[AppartementCRUD::insertAppartement] INSERT OPERATION";
    qDebug() << "Room:" << record.room;
    qDebug() << "Floor:" << record.floor;
    qDebug() << "Condition:" << record.condition;
    qDebug() << "Status:" << record.status;
    qDebug() << "CIN:" << record.cin;

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        QString msg = "Database connection is not open";
        qCritical() << "[AppartementCRUD::insertAppartement]" << msg;
        if (errorMsg) *errorMsg = msg;
        return false;
    }

    if (!db.transaction()) {
        qWarning() << "[AppartementCRUD::insertAppartement] Transaction start failed:" << db.lastError().text();
    }

    QSqlQuery query(db);
    QString sql = "INSERT INTO APPARTEMENT (ID, ROOM, FLOOR, CONDITION, STATUS, CIN) "
                  "VALUES (APPARTEMENT_SEQ.NEXTVAL, ?, ?, ?, ?, ?)";

    qDebug() << "[AppartementCRUD::insertAppartement] SQL:" << sql;

    if (!query.prepare(sql)) {
        QString msg = "Query prepare failed: " + query.lastError().text();
        logSqlError("insertAppartement", query.lastError());
        db.rollback();
        if (errorMsg) *errorMsg = msg;
        return false;
    }

    query.addBindValue(record.room);
    query.addBindValue(record.floor);
    query.addBindValue(record.condition);
    query.addBindValue(record.status);
    query.addBindValue(record.cin.isEmpty() ? "" : record.cin);

    qDebug() << "[AppartementCRUD::insertAppartement] Executing with values:";
    qDebug() << "  room =" << record.room;
    qDebug() << "  floor =" << record.floor;
    qDebug() << "  condition =" << record.condition;
    qDebug() << "  status =" << record.status;
    qDebug() << "  cin =" << record.cin;

    if (!query.exec()) {
        QString msg = "Insert execution failed: " + query.lastError().text();
        logSqlError("insertAppartement", query.lastError());
        db.rollback();
        if (errorMsg) *errorMsg = msg;
        qDebug() << "========================================";
        return false;
    }

    if (!db.commit()) {
        QString msg = "Commit failed: " + db.lastError().text();
        qCritical() << "[AppartementCRUD::insertAppartement]" << msg;
        db.rollback();
        if (errorMsg) *errorMsg = msg;
        return false;
    }

    qDebug() << "[AppartementCRUD::insertAppartement] SUCCESS - Rows affected:" << query.numRowsAffected();
    qDebug() << "========================================";
    return true;
}

QVector<AppartementRecord> AppartementCRUD::selectAllAppartements(QString *errorMsg)
{
    qDebug() << "========================================";
    qDebug() << "[AppartementCRUD::selectAllAppartements] SELECT ALL OPERATION";

    QVector<AppartementRecord> records;
    QSqlDatabase db = QSqlDatabase::database();

    if (!db.isOpen()) {
        QString msg = "Database connection is not open";
        qCritical() << "[AppartementCRUD::selectAllAppartements]" << msg;
        if (errorMsg) *errorMsg = msg;
        return records;
    }

    QSqlQuery query(db);
    QString sql = "SELECT ID, ROOM, FLOOR, CONDITION, STATUS, CIN "
                  "FROM APPARTEMENT ORDER BY ID";

    qDebug() << "[AppartementCRUD::selectAllAppartements] SQL:" << sql;

    if (!query.exec(sql)) {
        QString msg = "Select execution failed: " + query.lastError().text();
        logSqlError("selectAllAppartements", query.lastError());
        if (errorMsg) *errorMsg = msg;
        qDebug() << "========================================";
        return records;
    }

    int count = 0;
    while (query.next()) {
        AppartementRecord record;
        record.id = query.value(0).toLongLong();
        record.room = query.value(1).toInt();
        record.floor = query.value(2).toInt();
        record.condition = query.value(3).toString();
        record.status = query.value(4).toString();
        record.cin = query.value(5).toString();
        records.append(record);
        count++;

        qDebug() << "[AppartementCRUD::selectAllAppartements] Record" << count << ":"
                 << "ID=" << record.id
                 << "Room=" << record.room
                 << "Floor=" << record.floor
                 << "Condition=" << record.condition
                 << "Status=" << record.status
                 << "CIN=" << record.cin;
    }

    qDebug() << "[AppartementCRUD::selectAllAppartements] SUCCESS - Total records:" << records.size();
    qDebug() << "========================================";
    return records;
}

bool AppartementCRUD::fetchAppartementById(qint64 id, AppartementRecord &record, QString *errorMsg)
{
    qDebug() << "========================================";
    qDebug() << "[AppartementCRUD::fetchAppartementById] FETCH BY ID OPERATION";
    qDebug() << "ID:" << id;

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        QString msg = "Database connection is not open";
        qCritical() << "[AppartementCRUD::fetchAppartementById]" << msg;
        if (errorMsg) *errorMsg = msg;
        return false;
    }

    QSqlQuery query(db);
    QString sql = QString("SELECT ID, ROOM, FLOOR, CONDITION, STATUS, CIN "
                          "FROM APPARTEMENT WHERE ID = %1").arg(id);

    qDebug() << "[AppartementCRUD::fetchAppartementById] SQL:" << sql;

    if (!query.exec(sql)) {
        QString msg = "Fetch execution failed: " + query.lastError().text();
        logSqlError("fetchAppartementById", query.lastError());
        if (errorMsg) *errorMsg = msg;
        qDebug() << "========================================";
        return false;
    }

    if (query.next()) {
        record.id = query.value(0).toLongLong();
        record.room = query.value(1).toInt();
        record.floor = query.value(2).toInt();
        record.condition = query.value(3).toString();
        record.status = query.value(4).toString();
        record.cin = query.value(5).toString();

        qDebug() << "[AppartementCRUD::fetchAppartementById] SUCCESS - Found:";
        qDebug() << "  Room:" << record.room;
        qDebug() << "  Floor:" << record.floor;
        qDebug() << "  Condition:" << record.condition;
        qDebug() << "  Status:" << record.status;
        qDebug() << "  CIN:" << record.cin;
        qDebug() << "========================================";
        return true;
    }

    QString msg = QString("Appartement ID %1 not found").arg(id);
    qWarning() << "[AppartementCRUD::fetchAppartementById]" << msg;
    if (errorMsg) *errorMsg = msg;
    qDebug() << "========================================";
    return false;
}

bool AppartementCRUD::updateAppartement(qint64 appartementId, const AppartementRecord &record, QString *errorMsg)
{
    qDebug() << "========================================";
    qDebug() << "[AppartementCRUD::updateAppartement] UPDATE OPERATION";
    qDebug() << "ID:" << appartementId;
    qDebug() << "New Room:" << record.room;
    qDebug() << "New Floor:" << record.floor;
    qDebug() << "New Condition:" << record.condition;
    qDebug() << "New Status:" << record.status;
    qDebug() << "New CIN:" << record.cin;

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        QString msg = "Database connection is not open";
        qCritical() << "[AppartementCRUD::updateAppartement]" << msg;
        if (errorMsg) *errorMsg = msg;
        return false;
    }

    QSqlQuery query(db);
    QString sql = QString("UPDATE APPARTEMENT SET ROOM = %1, FLOOR = %2, CONDITION = '%3', "
                          "STATUS = '%4' WHERE ID = %5")
                      .arg(record.room)
                      .arg(record.floor)
                      .arg(record.condition)
                      .arg(record.status)
                      .arg(appartementId);

    qDebug() << "[AppartementCRUD::updateAppartement] SQL:" << sql;

    if (!query.exec(sql)) {
        QString msg = "Update execution failed: " + query.lastError().text();
        qCritical() << "[AppartementCRUD::updateAppartement] EXEC FAILED";
        qCritical() << "  Error text:" << query.lastError().text();
        qCritical() << "  Driver text:" << query.lastError().driverText();
        qCritical() << "  Database text:" << query.lastError().databaseText();
        qCritical() << "  Native error:" << query.lastError().nativeErrorCode();
        logSqlError("updateAppartement", query.lastError());
        if (errorMsg) *errorMsg = msg;
        qDebug() << "========================================";
        return false;
    }

    int rowsAffected = query.numRowsAffected();
    qDebug() << "[AppartementCRUD::updateAppartement] SUCCESS - Rows affected:" << rowsAffected;

    if (rowsAffected == 0) {
        qWarning() << "[AppartementCRUD::updateAppartement] WARNING: No rows updated (ID may not exist)";
    }

    qDebug() << "========================================";
    return true;
}

bool AppartementCRUD::deleteAppartement(qint64 appartementId, QString *errorMsg)
{
    qDebug() << "========================================";
    qDebug() << "[AppartementCRUD::deleteAppartement] DELETE OPERATION";
    qDebug() << "ID to delete:" << appartementId;

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        QString msg = "Database connection is not open";
        qCritical() << "[AppartementCRUD::deleteAppartement]" << msg;
        if (errorMsg) *errorMsg = msg;
        return false;
    }

    if (!db.transaction()) {
        qWarning() << "[AppartementCRUD::deleteAppartement] Transaction start failed:" << db.lastError().text();
    }

    QSqlQuery query(db);
    QString sql = QString("DELETE FROM APPARTEMENT WHERE ID = %1").arg(appartementId);

    qDebug() << "[AppartementCRUD::deleteAppartement] SQL:" << sql;

    if (!query.exec(sql)) {
        QString msg = "Delete execution failed: " + query.lastError().text();
        logSqlError("deleteAppartement", query.lastError());
        db.rollback();
        if (errorMsg) *errorMsg = msg;
        qDebug() << "========================================";
        return false;
    }

    if (!db.commit()) {
        QString msg = "Commit failed: " + db.lastError().text();
        qCritical() << "[AppartementCRUD::deleteAppartement]" << msg;
        db.rollback();
        if (errorMsg) *errorMsg = msg;
        return false;
    }

    int rowsAffected = query.numRowsAffected();
    qDebug() << "[AppartementCRUD::deleteAppartement] SUCCESS - Rows affected:" << rowsAffected;

    if (rowsAffected == 0) {
        qWarning() << "[AppartementCRUD::deleteAppartement] WARNING: No rows deleted (ID may not exist)";
    }

    qDebug() << "========================================";
    return true;
}

qint64 AppartementCRUD::getLastInsertedId()
{
    QSqlQuery query("SELECT APPARTEMENT_SEQ.CURRVAL FROM DUAL");
    if (query.exec() && query.next()) {
        qint64 id = query.value(0).toLongLong();
        qDebug() << "[AppartementCRUD::getLastInsertedId] Last ID:" << id;
        return id;
    }
    qWarning() << "[AppartementCRUD::getLastInsertedId] Failed:" << query.lastError().text();
    return -1;
}



void AppartementCRUD::exporterListeAppartementsPDF(QWidget *parentWidget)
{
    qDebug() << "=== DÉBUT EXPORT PDF SIMPLE ===";

    // Récupérer les données
    QSqlQuery query;
    QString sql = "SELECT ID, ROOM, FLOOR, CONDITION, STATUS, CIN FROM APPARTEMENT ORDER BY FLOOR, ROOM";

    if (!query.exec(sql)) {
        QMessageBox::critical(parentWidget, "Erreur", "Erreur lors de la récupération: " + query.lastError().text());
        return;
    }

    // Fichier de sortie
    QString fileName = QFileDialog::getSaveFileName(parentWidget,
                                                    "Exporter en PDF",
                                                    QDir::homePath() + "/liste_appartements.pdf",
                                                    "PDF Files (*.pdf)");

    if (fileName.isEmpty()) return;

    // Créer le PDF
    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize(QPageSize::A4));
    // Créer le document
    QTextDocument document;
    QString html;

    // Générer le contenu HTML simple
    html += "<h1>LISTE DES APPARTEMENTS</h1>";
    html += "<h2>Smart City - Gestion Immobilière</h2>";
    html += "<p>Généré le: " + QDateTime::currentDateTime().toString("dd/MM/yyyy à HH:mm") + "</p>";
    html += "<table border='1' style='width:100%; border-collapse:collapse;'>";
    html += "<tr style='background-color:#3498db; color:white;'>";
    html += "<th>ID</th><th>Numéro</th><th>Étage</th><th>Condition</th><th>Statut</th><th>CIN Résident</th>";
    html += "</tr>";

    int totalAppartements = 0;
    int occupeCount = 0;
    int libreCount = 0;

    while (query.next()) {
        totalAppartements++;
        QString status = query.value(4).toString();

        if (status.toUpper().contains("ACTIVE") || status.toUpper().contains("OCCUP")) {
            occupeCount++;
        } else {
            libreCount++;
        }

        html += "<tr>";
        for (int i = 0; i < 6; i++) {
            html += "<td>" + query.value(i).toString() + "</td>";
        }
        html += "</tr>";
    }

    html += "</table>";

    // Résumé
    double tauxOccupation = totalAppartements > 0 ? (occupeCount * 100.0) / totalAppartements : 0.0;

    html += "<h3>RÉSUMÉ</h3>";
    html += "<p>Total des appartements: " + QString::number(totalAppartements) + "</p>";
    html += "<p>Appartements occupés: " + QString::number(occupeCount) + "</p>";
    html += "<p>Appartements libres: " + QString::number(libreCount) + "</p>";
    html += "<p>Taux d'occupation: " + QString::number(tauxOccupation, 'f', 1) + "%</p>";

    document.setHtml(html);
    document.print(&printer);

    QMessageBox::information(parentWidget, "Succès",
                             QString("PDF créé avec %1 appartements").arg(totalAppartements));
}
QSqlQueryModel* AppartementCRUD::searchAppartements(const QString &searchText, QString *errorMsg)
{
    qDebug() << "========================================";
    qDebug() << "[AppartementCRUD::searchAppartements] SEARCH OPERATION";
    qDebug() << "Search text:" << searchText;

    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlDatabase db = QSqlDatabase::database();

    if (!db.isOpen()) {
        QString msg = "Database connection is not open";
        qCritical() << "[AppartementCRUD::searchAppartements]" << msg;
        if (errorMsg) *errorMsg = msg;
        delete model;
        return nullptr;
    }

    QSqlQuery query(db);
    QString sql;

    if (searchText.isEmpty()) {
        sql = "SELECT ID, ROOM, FLOOR, CONDITION, STATUS, CIN FROM APPARTEMENT ORDER BY ID";
    } else {
        // Parameterized substring search to match any column containing the text
        sql = QString(
                  "SELECT ID, ROOM, FLOOR, CONDITION, STATUS, CIN FROM APPARTEMENT "
                  "WHERE UPPER(ID) LIKE UPPER(:p) "
                  "OR TO_CHAR(ROOM) LIKE :p "
                  "OR TO_CHAR(FLOOR) LIKE :p "
                  "OR UPPER(CONDITION) LIKE UPPER(:p) "
                  "OR UPPER(STATUS) LIKE UPPER(:p) "
                  "OR UPPER(CIN) LIKE UPPER(:p) "
                  "ORDER BY ID"
                  );
    }

    qDebug() << "[AppartementCRUD::searchAppartements] SQL:" << sql;

    if (searchText.isEmpty()) {
        // For empty search do a direct exec
        if (!query.exec(sql)) {
            QString msg = "Search execution failed: " + query.lastError().text();
            qCritical() << "[AppartementCRUD::searchAppartements]" << msg;
            if (errorMsg) *errorMsg = msg;
            delete model;
            qDebug() << "========================================";
            return nullptr;
        }
    } else {
        // For non-empty search, use a parameterized query to avoid errors and allow special chars
        if (!query.prepare(sql)) {
            QString msg = "Failed to prepare search query: " + query.lastError().text();
            qCritical() << "[AppartementCRUD::searchAppartements]" << msg;
            if (errorMsg) *errorMsg = msg;
            delete model;
            return nullptr;
        }
        // Bind the same pattern for all :p usages
        QString pattern = "%" + searchText + "%";
        qDebug() << "[AppartementCRUD::searchAppartements] pattern=" << pattern;
        query.bindValue(":p", pattern);
        if (!query.exec()) {
            QString msg = "Search execution failed: " + query.lastError().text();
            qCritical() << "[AppartementCRUD::searchAppartements]" << msg;
            if (errorMsg) *errorMsg = msg;
            delete model;
            qDebug() << "========================================";
            return nullptr;
        }
    }

    // no extra sanity checks here; return the model

    model->setQuery(std::move(query));

    // Définir les en-têtes
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Room");
    model->setHeaderData(2, Qt::Horizontal, "Floor");
    model->setHeaderData(3, Qt::Horizontal, "Condition");
    model->setHeaderData(4, Qt::Horizontal, "Status");
    model->setHeaderData(5, Qt::Horizontal, "CIN");

    qDebug() << "[AppartementCRUD::searchAppartements] SUCCESS - Results found:" << model->rowCount();
    qDebug() << "========================================";

    return model;
}

