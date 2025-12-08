#ifndef VEHICLE_H
#define VEHICLE_H

#include <QString>
#include <QSqlQueryModel>
#include <QDate>

class Vehicle
{
public:
    Vehicle();
    Vehicle(const QString &matricule, const QString &cin, const QString &couleur,
            const QString &typeVehicule, const QString &marque, const QString &proprietaire,
            const QDate &dateDebut, const QDate &dateFin, int badgeActif = 0);

    bool ajouter() const;
    bool modifier(const QString &matriculeReference) const;
    static bool supprimer(const QString &matricule);

    static QSqlQueryModel *afficher();
    static QSqlQueryModel *rechercher(const QString &texte);

    // Getters
    QString getMatricule() const { return m_matricule; }
    QString getCin() const { return m_cin; }
    QString getCouleur() const { return m_couleur; }
    QString getTypeVehicule() const { return m_typeVehicule; }
    QString getMarque() const { return m_marque; }
    QString getProprietaire() const { return m_proprietaire; }
    QDate getDateDebut() const { return m_dateDebut; }
    QDate getDateFin() const { return m_dateFin; }
    int getBadgeActif() const { return m_badgeActif; }

    // Setters
    void setMatricule(const QString &matricule) { m_matricule = matricule; }
    void setCin(const QString &cin) { m_cin = cin; }
    void setCouleur(const QString &couleur) { m_couleur = couleur; }
    void setTypeVehicule(const QString &typeVehicule) { m_typeVehicule = typeVehicule; }
    void setMarque(const QString &marque) { m_marque = marque; }
    void setProprietaire(const QString &proprietaire) { m_proprietaire = proprietaire; }
    void setDateDebut(const QDate &dateDebut) { m_dateDebut = dateDebut; }
    void setDateFin(const QDate &dateFin) { m_dateFin = dateFin; }
    void setBadgeActif(int badgeActif) { m_badgeActif = badgeActif; }

private:
    QString m_matricule;
    QString m_cin;
    QString m_couleur;
    QString m_typeVehicule;
    QString m_marque;
    QString m_proprietaire;
    QDate m_dateDebut;
    QDate m_dateFin;
    int m_badgeActif;
};

#endif // VEHICLE_H
