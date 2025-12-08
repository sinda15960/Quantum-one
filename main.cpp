#include "gresident.h"
#include <QApplication>//cree une app
#include <QMessageBox>//pour les messages
#include "connexion.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv); //creation app

    Connection c; //creation objet
    bool test=c.createconnect(); //tentative de connexion a la base
    GResident w; //creation de la fenetre
    if(test)
    {w.show();
        QMessageBox::information(nullptr, QObject::tr("database is open"), //si connexion avec succé
                                 QObject::tr("connection successful.\n"
                                             "Click Cancel to exit."), QMessageBox::Cancel);

    }
    else
        QMessageBox::critical(nullptr, QObject::tr("database is not open"),//si non
                              QObject::tr("connection failed.\n"
                                          "Click Cancel to exit."), QMessageBox::Cancel);



    return a.exec();
}
