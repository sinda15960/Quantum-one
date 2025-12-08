#include "mainwindow.h"
#include "connection.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Connection c;
    if (!c.createconnect()) {
        QMessageBox::critical(nullptr, "Erreur", "Connexion à la base de données échouée !");
        return -1;
    }

    MainWindow w;
    w.show();
    return a.exec();
}
