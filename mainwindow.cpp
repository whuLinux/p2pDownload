#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_name_clicked()
{
    this->ctrl.local->setHostName(ui->lineEdit_name->text());
}

void MainWindow::on_pushButton_pwd_clicked()
{
    this->ctrl.local->setPwd(ui->lineEdit_pwd->text());
}

void MainWindow::on_pushButton_path_clicked()
{
    this->ctrl.local->downloadManager->setPath(ui->lineEdit_path->text());
}

void MainWindow::on_pushButton_url_clicked()
{
    this->ctrl.local->downloadManager->setUrl(ui->lineEdit_URL->text());
}

void MainWindow::on_pushButton_login_clicked()
{
    this->ctrl.local->regLocalClients();
}
