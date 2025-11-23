#ifndef APPARTEMENT_H
#define APPARTEMENT_H

#include <QString>
#include <cstdint>

namespace appartement {

struct AppartementRecord {
    qint64 id;
    int room;
    int floor;
    QString condition;
    QString status;
    QString cin;  // Foreign key to RESIDENT table (CHAR(10))
    
    AppartementRecord()
        : id(0), room(0), floor(0), condition(""), status(""), cin("") {}
};

}

#endif // APPARTEMENT_H
