#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include"mainctrl.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    mainctrl ctrl;
public:
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

private slots:
    void on_pushButton_name_clicked();

    void on_pushButton_pwd_clicked();

    void on_pushButton_path_clicked();

    void on_pushButton_url_clicked();

    void on_pushButton_login_clicked();

    void on_pushButton_create_clicked();

    void on_pushButton_mname_clicked();

    void on_pushButton_help_clicked();

    void on_pushButton_get_clients_clicked();

    void on_pushButton_punch_clicked();

    void on_pushButton_logout_clicked();

    void on_pushButton_downloadstart_clicked();

    void on_pushButton_debug_pause_clicked();

    void on_pushButton_debug_pause_false_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
