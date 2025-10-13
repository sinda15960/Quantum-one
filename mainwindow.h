#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Slots for the 4 action buttons to navigate TO the forms
    void on_btn1_clicked();
    void on_btn2_clicked();
    void on_btn3_clicked();
    void on_btn4_clicked();

    // Common slot for the 'Back' button
    void on_btnback_clicked();

private:
    Ui::MainWindow *ui;
    // Helper to connect the 'btnback' on all form pages
    void connectBackButtons();
};

#endif // MAINWINDOW_H
