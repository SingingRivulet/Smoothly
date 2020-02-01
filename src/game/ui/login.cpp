#include "login.h"
#include "ui_login.h"
#include "../game.h"

login::login(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::login)
{
    ui->setupUi(this);
    ui->lineEdit_pwd->setEchoMode(QLineEdit::Password);
}

login::~login()
{
    delete ui;
}

void login::on_pushButton_login_clicked()
{
    QString host_q = ui->lineEdit_host->text();
    QString port_q = ui->lineEdit_port->text();
    QString user_q = ui->lineEdit_user->text();
    QString pwd_q  = ui->lineEdit_pwd->text();
    if(host_q.isEmpty() || port_q.isEmpty() || user_q.isEmpty() || pwd_q.isEmpty())
        return;
    this->hide();
    auto host_s = host_q.toStdString();
    auto port_s = port_q.toStdString();
    short port   = atoi(port_s.c_str());
    smoothly::game * game = new smoothly::game(host_s.c_str() , port);
    game->cmd_login(user_q.toStdString() , pwd_q.toStdString());
    while(game->gameLoop());
    delete game;
    this->close();
}
