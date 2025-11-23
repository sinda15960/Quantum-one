#include "employe.h"

Employe::Employe() {} //constructeur vide

Employe::Employe(QString id, QString nom, QString prenom, QString poste, QString salaire) //constructeur avec parametre
{
    this->id_employe = id;
    this->nom = nom;
    this->prenom = prenom;
    this->poste = poste;
    this->salaire = salaire;
}

QString Employe::getId() const { return id_employe; }
QString Employe::getNom() const { return nom; }
QString Employe::getPrenom() const { return prenom; }
QString Employe::getPoste() const { return poste; }
QString Employe::getSalaire() const { return salaire; }

void Employe::setId(QString id) { id_employe = id; }
void Employe::setNom(QString n) { nom = n; }
void Employe::setPrenom(QString p) { prenom = p; }
void Employe::setPoste(QString ps) { poste = ps; }
void Employe::setSalaire(QString s) { salaire = s; }
QSqlQueryModel* Employe::afficher()
{
    QSqlQueryModel* model = new QSqlQueryModel();
    QSqlQuery query;
    query.exec("SELECT ID_EMPLOYE, NOM, PRENOM, POSTE, SALAIRE FROM EMPLOYE");
    model->setQuery(std::move(query));

    model->setHeaderData(0, Qt::Horizontal, "ID Employé");
    model->setHeaderData(1, Qt::Horizontal, "Nom");
    model->setHeaderData(2, Qt::Horizontal, "Prénom");
    model->setHeaderData(3, Qt::Horizontal, "Poste");
    model->setHeaderData(4, Qt::Horizontal, "Salaire");

    return model;
}

QSqlQueryModel* Employe::afficherPoste(QString poste)
{
    QSqlQueryModel *model = new QSqlQueryModel();

    QSqlQuery query;
    query.prepare("SELECT * FROM EMPLOYE WHERE LOWER(TRIM(POSTE)) LIKE LOWER(:poste)");
    query.bindValue(":poste", poste);

    query.exec();
    model->setQuery(std::move(query));

    return model;
}




bool Employe::ajouter()
{
    QSqlQuery query;
    query.prepare("INSERT INTO EMPLOYE (ID_EMPLOYE, NOM, PRENOM, POSTE, SALAIRE) "
                  "VALUES (:id, :nom, :prenom, :poste, :salaire)");

    query.bindValue(":id", id_employe);
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":poste", poste);
    query.bindValue(":salaire", salaire);

    return query.exec();
}

bool Employe::supprimer(QString id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM EMPLOYE WHERE TRIM(ID_EMPLOYE)=:id");
    query.bindValue(":id", id);

    if(!query.exec())
        return false; // Erreur de requête

    // ✅ IMPORTANT : vérifier si une ligne a réellement été supprimée
    return query.numRowsAffected() > 0;
}

bool Employe::existe(QString id)
{
    QSqlQuery query;
    query.prepare("SELECT ID_EMPLOYE FROM EMPLOYE WHERE TRIM(ID_EMPLOYE)=:id");
    query.bindValue(":id", id);
    query.exec();
    return query.next(); // true si résultat trouvé
}


bool Employe::modifier()
{
    QSqlQuery query;
    query.prepare("UPDATE EMPLOYE SET NOM=:nom, PRENOM=:prenom, POSTE=:poste, SALAIRE=:salaire "
                  "WHERE ID_EMPLOYE=:id");

    query.bindValue(":id", id_employe);
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":poste", poste);
    query.bindValue(":salaire", salaire);

    return query.exec();
}


