#include "serviceitem.h"

#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QtGlobal>
#include <QObject>

namespace {
QString dbName()
{
    return QSqlDatabase::database().connectionName();
}

void showServiceError(const QString &message, const QSqlError &error)
{
    QMessageBox::critical(nullptr,
                          QObject::tr("Erreur SQL"),
                          message + "\n" + error.text());
}
}

ServiceItem::ServiceItem()
    : ServiceItem(0, QString(), QString(), qQNaN(), QDate(), QString())
{
}

ServiceItem::ServiceItem(int id, const QString &nom, const QString &typeService,
                         double prix, const QDate &dateService, const QString &status)
    : m_id(id),
      m_nom(nom.trimmed()),
      m_typeService(typeService.trimmed()),
      m_prix(prix),
      m_dateService(dateService),
      m_status(status.trimmed())
{
}

QVariant ServiceItem::prixVariant(double prix)
{
    return qIsFinite(prix) ? QVariant(prix) : QVariant(QVariant::Double);
}

bool ServiceItem::ajouter(bool utiliserSequence) const
{
    QSqlDatabase db = QSqlDatabase::database(dbName());
    QSqlQuery query(db);

    if (utiliserSequence) {
        query.prepare(QStringLiteral(
            "INSERT INTO SERVICE (ID_SERVICE, NOM, TYPE_SERVICE, PRIX, DATE_SERVICE, STATUS_SERVICE) "
            "VALUES (SERVICE_SEQ.NEXTVAL, :nom, :type, :prix, :date_service, :status)"));
    } else {
        query.prepare(QStringLiteral(
            "INSERT INTO SERVICE (ID_SERVICE, NOM, TYPE_SERVICE, PRIX, DATE_SERVICE, STATUS_SERVICE) "
            "VALUES (:id, :nom, :type, :prix, :date_service, :status)"));
        query.bindValue(":id", m_id);
    }

    query.bindValue(":nom", m_nom);
    query.bindValue(":type", m_typeService);
    query.bindValue(":prix", prixVariant(m_prix));
    query.bindValue(":date_service", m_dateService.isValid() ? QVariant(m_dateService) : QVariant(QVariant::Date));
    query.bindValue(":status", m_status);

    if (!query.exec()) {
        showServiceError(QObject::tr("Impossible d'ajouter le service."), query.lastError());
        return false;
    }

    QMessageBox::information(nullptr,
                             QObject::tr("Service ajouté"),
                             QObject::tr("Le service a été ajouté avec succès."));
    return true;
}

bool ServiceItem::modifier(int idReference) const
{
    QSqlDatabase db = QSqlDatabase::database(dbName());
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "UPDATE SERVICE "
        "SET NOM = :nom, TYPE_SERVICE = :type, PRIX = :prix, "
        "    DATE_SERVICE = :date_service, STATUS_SERVICE = :status "
        "WHERE ID_SERVICE = :id_reference"));

    query.bindValue(":nom", m_nom);
    query.bindValue(":type", m_typeService);
    query.bindValue(":prix", prixVariant(m_prix));
    query.bindValue(":date_service", m_dateService.isValid() ? QVariant(m_dateService) : QVariant(QVariant::Date));
    query.bindValue(":status", m_status);
    query.bindValue(":id_reference", idReference);

    if (!query.exec()) {
        showServiceError(QObject::tr("Impossible de modifier le service."), query.lastError());
        return false;
    }

    if (query.numRowsAffected() == 0) {
        QMessageBox::warning(nullptr,
                             QObject::tr("Aucune modification"),
                             QObject::tr("Aucun service ne correspond à l'identifiant fourni."));
        return false;
    }

    QMessageBox::information(nullptr,
                             QObject::tr("Service modifié"),
                             QObject::tr("Les informations du service ont été mises à jour."));
    return true;
}

bool ServiceItem::supprimer(int id)
{
    QSqlDatabase db = QSqlDatabase::database(dbName());
    QSqlQuery query(db);
    query.prepare(QStringLiteral("DELETE FROM SERVICE WHERE ID_SERVICE = :id"));
    query.bindValue(":id", id);

    if (!query.exec()) {
        showServiceError(QObject::tr("Impossible de supprimer le service."), query.lastError());
        return false;
    }

    if (query.numRowsAffected() == 0) {
        QMessageBox::warning(nullptr,
                             QObject::tr("Suppression"),
                             QObject::tr("Aucun service trouvé avec l'identifiant fourni."));
        return false;
    }

    QMessageBox::information(nullptr,
                             QObject::tr("Suppression réussie"),
                             QObject::tr("Le service a été supprimé."));
    return true;
}

QSqlQueryModel *ServiceItem::afficher()
{
    auto *model = new QSqlQueryModel();
    QSqlDatabase db = QSqlDatabase::database(dbName());
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT ID_SERVICE, NOM, TYPE_SERVICE, "
        "       NVL(PRIX, 0) AS PRIX, "
        "       DATE_SERVICE, STATUS_SERVICE "
        "FROM SERVICE ORDER BY NOM"));
    query.exec();
    model->setQuery(std::move(query));

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Identifiant"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Type"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Prix"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Date"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Statut"));

    return model;
}

QSqlQueryModel *ServiceItem::rechercher(const QString &texte)
{
    auto *model = new QSqlQueryModel();
    QSqlDatabase db = QSqlDatabase::database(dbName());
    QSqlQuery query(db);
    QString pattern = "%" + texte.trimmed().toUpper() + "%";
    query.prepare(QStringLiteral(
        "SELECT ID_SERVICE, NOM, TYPE_SERVICE, NVL(PRIX, 0) AS PRIX, DATE_SERVICE, STATUS_SERVICE "
        "FROM SERVICE "
        "WHERE UPPER(NOM) LIKE :pattern "
        "   OR UPPER(TYPE_SERVICE) LIKE :pattern "
        "   OR UPPER(STATUS_SERVICE) LIKE :pattern "
        "ORDER BY NOM"));
    query.bindValue(":pattern", pattern);
    query.exec();
    model->setQuery(std::move(query));

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Identifiant"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Type"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Prix"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Date"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Statut"));

    return model;
}
