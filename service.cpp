#include "service.h"
#include <QDebug>


service::service(int id, QString name,  QString status,  QString type, int prix,  QDate date_service)
{
    id_service=id;
    this->name=name;
    this->status=status;
    this->type=type;
    this->prix=prix;
    this->date_service=date_service;
}

bool service::ajouter()
{
    QSqlQuery query;

    query.prepare("INSERT INTO SERVICES (ID_SERVICE,NAME,STATUS,TYPE,PRIX,DATE_SERVICE) "
                  "VALUES (:ID_SERVICE,:NAME,:STATUS,:TYPE,:PRIX,:DATE_SERVICE)");

    query.bindValue(":ID_SERVICE", id_service);
    query.bindValue(":NAME", name);
    query.bindValue(":STATUS",status );
    query.bindValue(":TYPE", type);
    query.bindValue(":PRIX",prix);
    query.bindValue(":DATE_SERVICE",date_service);

if(query.exec()==true)
    return true;
else
    return false;
}


QSqlQueryModel* service::afficher()
{
QSqlQueryModel* model=new QSqlQueryModel();

     model->setQuery("SELECT * FROM SERVICES");
return model;

}


QSqlQueryModel* service::tri(QString column,QString choix)
{
QSqlQueryModel* model = new QSqlQueryModel();
model->setQuery("SELECT * FROM SERVICES ORDER BY "+column +" "+choix);


return model;
}

QSqlQueryModel* service::chercher(QString column,QString text)
{
QSqlQueryModel* model = new QSqlQueryModel();
model->setQuery("SELECT * FROM SERVICES WHERE "+column+" LIKE '%" + text + "%' ");


return model;
}




QMap<QString, int> service::statistiquesParType() {
    QMap<QString, int> TypeStats;

    QSqlQuery query;
    query.prepare("SELECT Type, COUNT(*) as count FROM SERVICES GROUP BY Type");

    if (query.exec()) {
        while (query.next()) {
            QString Type = query.value(0).toString();
            int count = query.value(1).toInt();
            TypeStats[Type] = count;
        }
    }

    return TypeStats;
}
//Maintenance 5
//Support 3
//

bool service::supprimer(int id)
{QSqlQuery query;
query.prepare("Delete FROM SERVICES where ID_SERVICE=:id");
query.bindValue(0,id);
return query.exec();
}

bool service::idExists(int id)
{
QSqlQuery query;
query.prepare("SELECT COUNT(*) FROM SERVICES WHERE ID_SERVICE = :id");
query.bindValue(":id", id);

if (query.exec() && query.next()) {
    int count = query.value(0).toInt();
    return count > 0;
}

return false;
}

bool service::modifier()
{
QSqlQuery query;


query.prepare("UPDATE SERVICES SET NAME=:NAME, STATUS=:STATUS  , TYPE=:TYPE , PRIX=:PRIX,DATE_SERVICE=:DATE_SERVICE  where ID_SERVICE=:ID_SERVICE");
query.bindValue(":ID_SERVICE", id_service);
query.bindValue(":NAME", name);
query.bindValue(":STATUS",status );
query.bindValue(":TYPE", type);
query.bindValue(":PRIX",prix);
query.bindValue(":DATE_SERVICE",date_service);


return query.exec();

}
bool service::EditCodePromo(int id_service ,QString CODE_PROMO)
{
    QSqlQuery query;


    query.prepare("UPDATE SERVICES SET CODE_PROMO=:CODE_PROMO where ID_SERVICE=:ID_SERVICE");
    query.bindValue(":ID_SERVICE", id_service);
    query.bindValue(":CODE_PROMO", CODE_PROMO);
    return query.exec();

}
bool service::DeleteCodePromo(int id_service)
{
    QSqlQuery query;


    query.prepare("UPDATE SERVICES SET CODE_PROMO='' where ID_SERVICE=:ID_SERVICE");
    query.bindValue(":ID_SERVICE", id_service);
    return query.exec();
}

int service::NbTotaleDuClientAyantScanne()
{
    QSqlQuery query;
int nb=0;

    query.prepare("SELECT COUNT(*) FROM RESIDENT_AVOIR_SERVICES WHERE  NB_PERSSONE_SCANNE=1");
    if(query.exec() && query.next())
    {
        nb=query.value(0).toInt();
    }
    return nb;

}


QMap<QString, float> service::PrixTotaleDuRemise()
{
    QSqlQuery query;
    float prixTotale = 0.0;
    QMap<QString, float> M;

    query.prepare("SELECT RS.ID_SERVICE, S.PRIX, S.NAME FROM RESIDENT_AVOIR_SERVICES RS "
                  "JOIN SERVICES S ON S.ID_SERVICE = RS.ID_SERVICE "
                  "WHERE RS.NB_PERSSONE_SCANNE = 1");

    if (query.exec())
    {
        while (query.next())
        {
            QString id_service = query.value(0).toString();
            float prix = query.value(1).toFloat();
            QString name = query.value(2).toString();

            float discountAmount = prix * 0.3;

            M[name] += discountAmount;

            prixTotale += discountAmount;
        }

        M["PrixTotale"] = prixTotale; //
    }

    return M;
}
