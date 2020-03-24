#include "login.h"
#include "ui_login.h"
#include "../game.h"

login::login(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::login)
{
    ui->setupUi(this);
    ui->lineEdit_pwd->setEchoMode(QLineEdit::Password);
    auto fp = fopen("user.txt","r");
    if(fp){
        char buf[128];
        if(!feof(fp)){
            bzero(buf,sizeof(buf));
            fgets(buf,sizeof(buf),fp);
            ui->lineEdit_user->setText(QString(buf).trimmed());
        }
        if(!feof(fp)){
            bzero(buf,sizeof(buf));
            fgets(buf,sizeof(buf),fp);
            ui->lineEdit_pwd->setText(QString(buf).trimmed());
        }
        fclose(fp);
    }
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
    if(ui->checkBox_saveUser->isChecked()){
        auto fp = fopen("user.txt","w");
        if(fp){
            fprintf(fp,"%s\n%s",user_q.toStdString().c_str() , pwd_q.toStdString().c_str());
            fclose(fp);
        }
    }
    smoothly::game * game = new smoothly::game(host_s.c_str() , port);
    game->cmd_login(user_q.toStdString() , pwd_q.toStdString());
    while(game->gameLoop());
    delete game;
    this->close();
}
