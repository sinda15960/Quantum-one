#ifndef SERVICEITEM_H
#define SERVICEITEM_H

#include <QDate>
#include <QSqlQueryModel>
#include <QString>
#include <QVariant>

class ServiceItem
{
public:
    ServiceItem();
    ServiceItem(int id, const QString &nom, const QString &typeService,
                double prix, const QDate &dateService, const QString &status);

    bool ajouter(bool utiliserSequence = true) const;
    bool modifier(int idReference) const;
    static bool supprimer(int id);

    static QSqlQueryModel *afficher();
    static QSqlQueryModel *rechercher(const QString &texte);

    int id() const { return m_id; }
    QString nom() const { return m_nom; }
    QString typeService() const { return m_typeService; }
    double prix() const { return m_prix; }
    QDate dateService() const { return m_dateService; }
    QString status() const { return m_status; }

    void setId(int id) { m_id = id; }
    void setNom(const QString &nom) { m_nom = nom; }
    void setTypeService(const QString &type) { m_typeService = type; }
    void setPrix(double prix) { m_prix = prix; }
    void setDateService(const QDate &date) { m_dateService = date; }
    void setStatus(const QString &status) { m_status = status; }

private:
    static QVariant prixVariant(double prix);

    int m_id;
    QString m_nom;
    QString m_typeService;
    double m_prix;
    QDate m_dateService;
    QString m_status;
};

#endif // SERVICEITEM_H
