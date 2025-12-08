#ifndef EMPLOYE_H
#define EMPLOYE_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>


class Employe
{
private:
    QString id_employe;
    QString nom;
    QString prenom;
    QString poste;
    QString salaire;

public:
    Employe();
    Employe(QString, QString, QString, QString, QString);

    // Getters
    QString getId() const;
    QString getNom() const;
    QString getPrenom() const;
    QString getPoste() const;
    QString getSalaire() const;

    // Setters
    void setId(QString);
    void setNom(QString);
    void setPrenom(QString);
    void setPoste(QString);
    void setSalaire(QString);
    // CRUD
    bool ajouter();
    QSqlQueryModel* afficher();
    QSqlQueryModel* afficherPoste(QString poste);
    bool supprimer(QString);
    bool existe(QString id);
    bool modifier();


};

#endif // EMPLOYE_H
