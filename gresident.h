#ifndef GRESIDENT_H
#define GRESIDENT_H

#include <QMainWindow>
#include <QTableView>
#include "employe.h"
#include "resident.h"
#include "serviceitem.h"
#include "crud.h"
#include "vehicle.h"
#include "smartavailability.h"
#include "voicetotext.h"
#include <QTimer>

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
    void onAddServiceClicked();
    void onDeleteServiceClicked();
    void onModifyServiceClicked();
    void onSearchServiceClicked();
    void refreshServiceTable();
    void onSearchServiceForModifyClicked();

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

    // ====== VOICE INPUT Slots ======
    void onSpeechToTextButtonClicked();
    void handleVoiceFinalMessage(const QString &role, const QString &text);
    void handleVoiceInterimMessage(const QString &text);
    void handleVoiceStatusChanged(const QString &status);
    void handleVoiceError(const QString &error);
    void setHumeApiKeyDialog();

private:
    Ui::GResident *ui;
    QTimer *searchAppartementTimer;
    VoiceToText *m_voiceToText{nullptr};
    QString m_pendingInterim;

    void appendTranscriptLine(const QString &prefix, const QString &text);
    QString resolveHumeApiKey() const;
    QString resolveHumeApiKeyFromFile() const;
};

#endif // GRESIDENT_H
