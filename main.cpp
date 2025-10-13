#include <QApplication>
#include "gresident.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    GResident w;
    w.setWindowTitle("Gestion des Résidents");
    w.resize(1600, 1000);
    w.show();

    return app.exec();
}
