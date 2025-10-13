#ifndef GRESIDENT_H
#define GRESIDENT_H

#include <QMainWindow>

namespace Ui {
class GResident;
}

class GResident : public QMainWindow
{
    Q_OBJECT

public:
    explicit GResident(QWidget *parent = nullptr);
    ~GResident();

private slots:
    void addResident();
    void deleteResident();
    void modifyResident();
    void displayResidents();

    // 👇 navigation helper
    void navigateToPage(int pageIndex);

private:
    Ui::GResident *ui;
};

#endif // GRESIDENT_H
