#ifndef CRUD_H
#define CRUD_H

#include "appartement.h"
#include <QString>
#include <QVector>
#include <QSqlError>
#include <QWidget>
#include <QSqlQueryModel>

using appartement::AppartementRecord;

class AppartementCRUD
{
public:
    static bool insertAppartement(const AppartementRecord &record, QString *errorMsg = nullptr);
    static QVector<AppartementRecord> selectAllAppartements(QString *errorMsg = nullptr);
    static bool fetchAppartementById(qint64 id, AppartementRecord &record, QString *errorMsg = nullptr);
    static bool updateAppartement(qint64 appartementId, const AppartementRecord &record, QString *errorMsg = nullptr);
    static bool deleteAppartement(qint64 appartementId, QString *errorMsg = nullptr);
    static qint64 getLastInsertedId();
    static void exporterListeAppartementsPDF(QWidget *parentWidget = nullptr);
    static QSqlQueryModel* searchAppartements(const QString &searchText, QString *errorMsg = nullptr);

private:
    static void logSqlError(const QString &operation, const QSqlError &error);
};

#endif // CRUD_H
