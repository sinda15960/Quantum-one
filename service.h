#ifndef SERVICE_H
#define SERVICE_H

#include <QString>
#include <QDate>
#include <QSqlQuery>
#include <QSqlQueryModel>

class service
{
public:
    // Constructor
    service()
        : id_service(0), name(""), status(""), type(""), prix(0.0f), date_service(QDate()) {}

    // Parameterized Constructor
    service(int id, QString name,  QString status,  QString type, int prix,  QDate date_service );

    // Getters
    int getIdService() const { return id_service; }
    QString getName() const { return name; }
    QString getStatus() const { return status; }
    QString getType() const { return type; }
    int getPrix() const { return prix; }
    QDate getDateService() const { return date_service; }

    // Setters
    void setIdService(int id) { id_service = id; }
    void setName(const QString& n) { name = n; }
    void setStatus(const QString& s) { status = s; }
    void setType(const QString& t) { type = t; }
    void setPrix(int price) { prix = price; }
    void setDateService(const QDate& date) { date_service = date; }

    // CRUD methods
    bool ajouter();
    bool modifier();
    bool supprimer(int id);
    QSqlQueryModel* afficher();
    bool idExists(int id);

    //metier
    QSqlQueryModel*  tri(QString column,QString choix);
    QSqlQueryModel* chercher(QString column,QString text);
    QMap<QString, int> statistiquesParType() ;
    //metier avance
    bool EditCodePromo(int id_service ,QString CODE_PROMO);
    bool DeleteCodePromo(int id_service);
    int NbTotaleDuClientAyantScanne();
    QMap<QString,float> PrixTotaleDuRemise();

private:
    int id_service;
    QString name;
    QString status;
    QString type;
    int prix;
    QDate date_service;
};

#endif // SERVICE_H
