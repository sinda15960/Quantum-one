#include "connexion.h"

Connection::Connection()
{

}

bool Connection::createconnect()
{bool test=false;
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName("projet");//inserer le nom de la source de données
    db.setUserName("maissa");//inserer nom de l'utilisateur
    db.setPassword("maissa123");//inserer mot de passe de cet utilisateur


    if (db.open())
        test=true;





    return  test;
}
