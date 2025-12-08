#include "residents.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVariant>

Residents::Residents()
    : cin(""), nom(""), prenom(""), telephone("")
{}

Residents::Residents(QString c, QString n, QString p, QString t)
    : cin(c), nom(n), prenom(p), telephone(t)
{}

QString Residents::getCin() const { return cin; }
QString Residents::getNom() const { return nom; }
QString Residents::getPrenom() const { return prenom; }
QString Residents::getTelephone() const { return telephone; }

void Residents::setCin(QString c) { cin = c; }
void Residents::setNom(QString n) { nom = n; }
void Residents::setPrenom(QString p) { prenom = p; }
void Residents::setTelephone(QString t) { telephone = t; }

bool Residents::ajouter() {
    QSqlQuery query;
    query.prepare("INSERT INTO RESIDENTS (CIN, NOM, PRENOM, TELEPHONE) "
                  "VALUES (:cin, :nom, :prenom, :telephone)");
    query.bindValue(":cin", cin);
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":telephone", telephone);

    if (!query.exec()) {
        qDebug() << "Erreur ajout:" << query.lastError().text();
        return false;
    }
    return true;
}

bool Residents::supprimer() {
    QSqlQuery query;
    query.prepare("DELETE FROM RESIDENTS WHERE CIN = :cin");
    query.bindValue(":cin", cin);

    if (!query.exec()) {
        qDebug() << "Erreur suppression:" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

bool Residents::modifier() {
    QSqlQuery query;
    query.prepare("UPDATE RESIDENTS SET NOM = :nom, PRENOM = :prenom, TELEPHONE = :telephone "
                  "WHERE CIN = :cin");
    query.bindValue(":cin", cin);
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":telephone", telephone);

    if (!query.exec()) {
        qDebug() << "Erreur modification:" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

QSqlQueryModel* Residents::afficher(const QString &value, const QString &type) {
    QSqlQueryModel* model = new QSqlQueryModel();
    QSqlQuery query;
    QString t = type.toUpper();

    if (value.isEmpty()) {
        if (t == "CIN") {
            query.prepare("SELECT CIN, NOM, PRENOM, TELEPHONE FROM RESIDENTS ORDER BY CIN ASC");
        } else if (t == "NOM") {
            query.prepare("SELECT CIN, NOM, PRENOM, TELEPHONE FROM RESIDENTS ORDER BY NOM ASC");
        } else {
            query.prepare("SELECT CIN, NOM, PRENOM, TELEPHONE FROM RESIDENTS ORDER BY CIN ASC");
        }
    } else {
        if (t == "CIN") {
            query.prepare("SELECT CIN, NOM, PRENOM, TELEPHONE FROM RESIDENTS WHERE CIN LIKE :value ORDER BY CIN ASC");
            query.bindValue(":value", "%" + value + "%");
        } else if (t == "NOM") {
            query.prepare("SELECT CIN, NOM, PRENOM, TELEPHONE FROM RESIDENTS WHERE UPPER(NOM) LIKE UPPER(:value) ORDER BY NOM ASC");
            query.bindValue(":value", "%" + value + "%");
        }
    }

    if (!query.exec()) {
        qDebug() << "Erreur affichage:" << query.lastError().text();
    }

    model->setQuery(std::move(query));
    return model;
}


bool Residents::search(QString cinToFind) {
    QSqlQuery query;
    query.prepare("SELECT NOM, PRENOM, TELEPHONE FROM RESIDENTS WHERE CIN = :cin");
    query.bindValue(":cin", cinToFind);

    if (!query.exec()) {
        qDebug() << "Erreur recherche:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        nom = query.value(0).toString();
        prenom = query.value(1).toString();
        telephone = query.value(2).toString();
        return true;
    }
    return false;
}
