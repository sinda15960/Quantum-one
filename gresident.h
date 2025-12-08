#ifndef GRESIDENT_H
#define GRESIDENT_H

#include <QMainWindow>
#include <QTableView>
#include "employe.h"
#include "resident.h"
#include "crud.h"
#include "vehicle.h"
#include "smartavailability.h"
#include "statisticsdialog.h"
#include <QTimer>
#include "service.h"
#include "arduino.h"
namespace Ui {
class GResident;
}

class GResident : public QMainWindow
{
    Q_OBJECT

public:
    explicit GResident(QWidget *parent = nullptr);
    ~GResident();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;  // ✅ AJOUTER CETTE LIGNE

private slots:
    void addResident();
    void deleteResident();
    void modifyResident();
    void displayResidents();
    void on_ajouterpushbutton_2_clicked();
    void on_exportPdfBtn_clicked();
    void on_searchAppartementTextChanged(const QString &text);
    void on_searchAppartementDynamic();
    void on_searchRoomEditTextChanged(const QString &text);
    void on_sortBtn_clicked();
    void onStatsAppartementsOccupees();
        void on_searchRoomBtn3Clicked();

    // 👇 navigation helper
    void navigateToPage(int pageIndex);

    void on_ajouterpushbutton_clicked();

    void on_deleteButton_clicked();

    void on_employetableView_clicked(const QModelIndex &index);

    void on_modifierpushbutton_clicked();

    // ====== RESIDENT CRUD Slots ======
    void onAddResidentClicked();
    void onDeleteResidentClicked();
    void onModifyResidentClicked();
    void onSearchResidentClicked();
    void refreshResidentTable();

    // ====== SERVICE CRUD Slots ======

    void on_bt_ajouter_service_clicked();

    void on_bt_update_service_clicked();

    void on_tableView_service_clicked(const QModelIndex &index);

    void on_bt_delete_service_clicked();

    void on_btn_tri_clicked();

    void on_Search_service_textChanged(const QString &text);

    void on_bt_pdf_service_clicked();

    void on_bt_stat_service_clicked();

    void on_bt_code_promo_clicked();

    void on_bt_Code_Qr_clicked();


    void on_bt_Facture_clicked();

    // ====== APPARTEMENT CRUD Slots ======
    void onAddAppartementClicked();
    void onDeleteAppartementClicked();
    void onModifyAppartementClicked();
    void onSearchAppartementClicked();
    void refreshAppartementTable();
    void onSearchAppartementForModifyClicked();

    // ====== VEHICLE CRUD Slots ======
    void onAddVehicleClicked();
    void onDeleteVehicleClicked();
    void onModifyVehicleClicked();
    void onSearchVehicleClicked();
    void refreshVehicleTable();

    // ====== SMARTAVAILABILITY Slots ======
    void onSmartAvailabilityClicked();


    //arduino
    void read_arduino();
    void on_bt_quitter_appilication_clicked();

private:
    Ui::GResident *ui;
    QTimer *searchAppartementTimer;
    service se;
    arduino A;

};

#endif // GRESIDENT_H
