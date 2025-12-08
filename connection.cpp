#include "connection.h"

Connection::Connection() {
    db = QSqlDatabase::addDatabase("QODBC");
}

bool Connection::createconnect() {
    db.setDatabaseName("projet");
    db.setUserName("sindalazaar"); // Remplacer par ton username
    db.setPassword("1234567890");   // Remplacer par ton password

    if (!db.open()) {
        qDebug() << "Erreur connexion Oracle:" << db.lastError().text();
        return false;
    }
    qDebug() << "Connexion Oracle réussie!";
    return true;
}

void Connection::closeConnection() {
    db.close();
}
