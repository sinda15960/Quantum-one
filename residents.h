#ifndef RESIDENTS_H
#define RESIDENTS_H

#include <QString>
#include <QSqlQueryModel>

class Residents {
public:
    Residents();
    Residents(QString c, QString n, QString p, QString t);

    QString getCin() const;
    QString getNom() const;
    QString getPrenom() const;
    QString getTelephone() const;

    void setCin(QString c);
    void setNom(QString n);
    void setPrenom(QString p);
    void setTelephone(QString t);

    bool ajouter();
    bool supprimer();
    bool modifier();
    QSqlQueryModel* afficher(const QString &value = "", const QString &type = "");
    bool search(QString cinToFind);

private:
    QString cin;
    QString nom;
    QString prenom;
    QString telephone;
};

#endif // RESIDENTS_H
