#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("登录");
    QRegExp rx = QRegExp("[0-9]{0,5}");
    QRegExpValidator *validator = new QRegExpValidator(rx);
    ui->lineEdit_port->setValidator(validator);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_login_clicked()
{
    auto host_s = ui->lineEdit_host->text();
    auto port_s = ui->lineEdit_port->text();
    auto key_s  = ui->lineEdit_key->text();
    if(host_s.isEmpty() || port_s.isEmpty() || key_s.isEmpty())
        return;
    auto  host = host_s.toStdString();
    short port = atoi(port_s.toStdString().c_str());
    auto  key  = key_s.toStdString();
    this->hide();
    manager * m = new manager(this);
    m->connectServer(host.c_str() , port , key);
    m->show();
}
