#include "vehicle.h"

#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

namespace {
QString connectionName()
{
    return QSqlDatabase::database().connectionName();
}

void showSqlError(const QString &title, const QSqlError &error)
{
    QMessageBox::critical(nullptr,
                          QObject::tr("Erreur SQL"),
                          title + "\n" + error.text());
}
}

Vehicle::Vehicle()
    : m_matricule(),
      m_cin(),
      m_couleur(),
      m_typeVehicule(),
      m_marque(),
      m_proprietaire(),
      m_dateDebut(QDate::currentDate()),
      m_dateFin(QDate::currentDate()),
      m_badgeActif(0)
{
}

Vehicle::Vehicle(const QString &matricule, const QString &cin, const QString &couleur,
                 const QString &typeVehicule, const QString &marque, const QString &proprietaire,
                 const QDate &dateDebut, const QDate &dateFin, int badgeActif)
    : m_matricule(matricule.trimmed()),
      m_cin(cin.trimmed()),
      m_couleur(couleur.trimmed()),
      m_typeVehicule(typeVehicule.trimmed()),
      m_marque(marque.trimmed()),
      m_proprietaire(proprietaire.trimmed()),
      m_dateDebut(dateDebut),
      m_dateFin(dateFin),
      m_badgeActif(badgeActif)
{
}

bool Vehicle::ajouter() const
{
    QSqlDatabase db = QSqlDatabase::database(connectionName());
    QSqlQuery query(db);
    
    // INSERT with all 9 fields INCLUDING CIN (CIN is NOT NULL in database)
    QString sql = QString(
        "INSERT INTO VEHICULE (MATRICULE, CIN, COULEUR, TYPE_VEHICULE, MARQUE, PROPRIETAIRE, DATE_DEBUT, DATE_FIN, BADGE_ACTIF) "
        "VALUES ('%1', '%2', '%3', '%4', '%5', '%6', TO_DATE('%7', 'YYYY-MM-DD'), TO_DATE('%8', 'YYYY-MM-DD'), %9)")
        .arg(m_matricule,
             m_cin,
             m_couleur,
             m_typeVehicule,
             m_marque,
             m_proprietaire,
             m_dateDebut.toString("yyyy-MM-dd"),
             m_dateFin.toString("yyyy-MM-dd"),
             QString::number(m_badgeActif));

    qDebug() << "[Vehicle::ajouter] SQL:" << sql;
    
    if (!query.exec(sql)) {
        qDebug() << "[Vehicle::ajouter] Error:" << query.lastError().text();
        return false;
    }

    qDebug() << "[Vehicle::ajouter] SUCCESS";
    return true;
}

bool Vehicle::modifier(const QString &matriculeReference) const
{
    QSqlDatabase db = QSqlDatabase::database(connectionName());
    QSqlQuery query(db);
    
    // Update all fields including CIN (CIN is NOT NULL in database)
    QString sql = QString(
        "UPDATE VEHICULE SET "
        "CIN = '%1', COULEUR = '%2', TYPE_VEHICULE = '%3', MARQUE = '%4', PROPRIETAIRE = '%5', "
        "DATE_DEBUT = TO_DATE('%6', 'YYYY-MM-DD'), DATE_FIN = TO_DATE('%7', 'YYYY-MM-DD'), BADGE_ACTIF = %8 "
        "WHERE UPPER(MATRICULE) = UPPER('%9')")
        .arg(m_cin,
             m_couleur,
             m_typeVehicule,
             m_marque,
             m_proprietaire,
             m_dateDebut.toString("yyyy-MM-dd"),
             m_dateFin.toString("yyyy-MM-dd"),
             QString::number(m_badgeActif),
             matriculeReference.trimmed());

    qDebug() << "[Vehicle::modifier] SQL:" << sql;

    if (!query.exec(sql)) {
        qDebug() << "[Vehicle::modifier] Error:" << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qDebug() << "[Vehicle::modifier] No rows found";
        return false;
    }

    qDebug() << "[Vehicle::modifier] SUCCESS";
    return true;
}

bool Vehicle::supprimer(const QString &matricule)
{
    QSqlDatabase db = QSqlDatabase::database(connectionName());
    QSqlQuery query(db);
    
    // Simple DELETE with string concatenation - accepts any input
    QString sql = QString(
        "DELETE FROM VEHICULE WHERE UPPER(MATRICULE) = UPPER('%1')")
        .arg(matricule.trimmed());

    if (!query.exec(sql)) {
        showSqlError(QObject::tr("Impossible de supprimer le véhicule."), query.lastError());
        return false;
    }

    if (query.numRowsAffected() == 0) {
        return false;
    }

    return true;
}

QSqlQueryModel *Vehicle::afficher()
{
    auto *model = new QSqlQueryModel();
    QSqlDatabase db = QSqlDatabase::database(connectionName());
    QSqlQuery query(db);
    
    // Display all VEHICULE columns
    query.exec(QStringLiteral(
        "SELECT MATRICULE, CIN, COULEUR, TYPE_VEHICULE, MARQUE, PROPRIETAIRE, DATE_DEBUT, DATE_FIN, BADGE_ACTIF "
        "FROM VEHICULE ORDER BY MATRICULE"));
    
    model->setQuery(std::move(query));

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Matricule"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("CIN"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Couleur"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Type Véhicule"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Marque"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Propriétaire"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Date Début"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("Date Fin"));
    model->setHeaderData(8, Qt::Horizontal, QObject::tr("Badge Actif"));

    return model;
}

QSqlQueryModel *Vehicle::rechercher(const QString &texte)
{
    auto *model = new QSqlQueryModel();
    QSqlDatabase db = QSqlDatabase::database(connectionName());
    QSqlQuery query(db);
    
    // Search across all text fields
    QString pattern = "%" + texte.trimmed().toUpper() + "%";
    QString sql = QString(
        "SELECT MATRICULE, CIN, COULEUR, TYPE_VEHICULE, MARQUE, PROPRIETAIRE, DATE_DEBUT, DATE_FIN, BADGE_ACTIF "
        "FROM VEHICULE "
        "WHERE UPPER(MATRICULE) LIKE '%1' "
        "   OR UPPER(CIN) LIKE '%1' "
        "   OR UPPER(COULEUR) LIKE '%1' "
        "   OR UPPER(TYPE_VEHICULE) LIKE '%1' "
        "   OR UPPER(MARQUE) LIKE '%1' "
        "   OR UPPER(PROPRIETAIRE) LIKE '%1' "
        "ORDER BY MATRICULE")
        .arg(pattern);
    
    query.exec(sql);
    model->setQuery(std::move(query));

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Matricule"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("CIN"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Couleur"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Type Véhicule"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Marque"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Propriétaire"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Date Début"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("Date Fin"));
    model->setHeaderData(8, Qt::Horizontal, QObject::tr("Badge Actif"));

    return model;
}
