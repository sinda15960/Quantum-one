#include "resident.h"

#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <utility>

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

Resident::Resident()
    : m_cin(),
      m_nom(),
      m_prenom(),
      m_tel()
{
}

Resident::Resident(const QString &cin, const QString &nom, const QString &prenom, const QString &tel)
    : m_cin(cin.trimmed()),
      m_nom(nom.trimmed()),
      m_prenom(prenom.trimmed()),
      m_tel(tel.trimmed())
{
}

bool Resident::ajouter() const
{
    QSqlDatabase db = QSqlDatabase::database(connectionName());
    QSqlQuery query(db);
    
    // Simple INSERT - no telephone field anymore
    QString sql = QString(
        "INSERT INTO RESIDENT (CIN, NOM, PRENOM) "
        "VALUES ('%1', '%2', '%3')")
        .arg(m_cin, m_nom, m_prenom);

    if (!query.exec(sql)) {
        showSqlError(QObject::tr("Impossible d'ajouter le résident."), query.lastError());
        return false;
    }

    return true;
}

bool Resident::modifier(const QString &cinReference) const
{
    QSqlDatabase db = QSqlDatabase::database(connectionName());
    QSqlQuery query(db);
    
    // Simple UPDATE with string concatenation - accepts any input
    QString sql = QString(
        "UPDATE RESIDENT SET NOM = '%1', PRENOM = '%2' "
        "WHERE UPPER(CIN) = UPPER('%3')")
        .arg(m_nom, m_prenom, cinReference.trimmed());

    if (!query.exec(sql)) {
        showSqlError(QObject::tr("Impossible de modifier le résident."), query.lastError());
        return false;
    }

    if (query.numRowsAffected() == 0) {
        return false;
    }

    return true;
}

bool Resident::supprimer(const QString &cin)
{
    QSqlDatabase db = QSqlDatabase::database(connectionName());
    QSqlQuery query(db);
    
    // Simple DELETE with string concatenation - accepts any input
    QString sql = QString(
        "DELETE FROM RESIDENT WHERE UPPER(CIN) = UPPER('%1')")
        .arg(cin.trimmed());

    if (!query.exec(sql)) {
        showSqlError(QObject::tr("Impossible de supprimer le résident."), query.lastError());
        return false;
    }

    if (query.numRowsAffected() == 0) {
        return false;
    }

    return true;
}

QSqlQueryModel *Resident::afficher()
{
    auto *model = new QSqlQueryModel();
    QSqlDatabase db = QSqlDatabase::database(connectionName());
    QSqlQuery query(db);
    
    // Display only CIN, NOM, PRENOM - no telephone
    query.exec(QStringLiteral(
        "SELECT CIN, NOM, PRENOM "
        "FROM RESIDENT ORDER BY NOM, PRENOM"));
    model->setQuery(std::move(query));

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("CIN"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Prénom"));

    return model;
}

QSqlQueryModel *Resident::rechercher(const QString &texte)
{
    auto *model = new QSqlQueryModel();
    QSqlDatabase db = QSqlDatabase::database(connectionName());
    QSqlQuery query(db);
    
    // Search - no telephone
    QString pattern = "%" + texte.trimmed().toUpper() + "%";
    QString sql = QString(
        "SELECT CIN, NOM, PRENOM "
        "FROM RESIDENT "
        "WHERE UPPER(CIN) LIKE '%1' "
        "   OR UPPER(NOM) LIKE '%1' "
        "   OR UPPER(PRENOM) LIKE '%1' "
        "ORDER BY NOM, PRENOM")
        .arg(pattern);
    
    query.exec(sql);
    model->setQuery(std::move(query));

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("CIN"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Prénom"));

    return model;
}

