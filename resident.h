#ifndef RESIDENT_H
#define RESIDENT_H

#include <QString>
#include <QSqlQueryModel>

class Resident
{
public:
    Resident();
    Resident(const QString &cin, const QString &nom, const QString &prenom, const QString &tel = QString());

    bool ajouter() const;
    bool modifier(const QString &cinReference) const;
    static bool supprimer(const QString &cin);

    static QSqlQueryModel *afficher();
    static QSqlQueryModel *rechercher(const QString &texte);

    QString getCin() const { return m_cin; }
    QString getNom() const { return m_nom; }
    QString getPrenom() const { return m_prenom; }
    QString getTel() const { return m_tel; }

    void setCin(const QString &cin) { m_cin = cin; }
    void setNom(const QString &nom) { m_nom = nom; }
    void setPrenom(const QString &prenom) { m_prenom = prenom; }
    void setTel(const QString &tel) { m_tel = tel; }

    QString m_cin;
    QString m_nom;
    QString m_prenom;
    QString m_tel;
};

#endif // RESIDENT_H
