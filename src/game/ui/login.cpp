#include "login.h"
#include "ui_login.h"

login::login(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::login),
    medit(parent)
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

login::~login(){
    delete ui;
}

void login::on_pushButton_login_clicked(){
    QString host_q = ui->lineEdit_host->text();
    QString port_q = ui->lineEdit_port->text();
    QString user_q = ui->lineEdit_user->text();
    QString pwd_q  = ui->lineEdit_pwd->text();
    if(host_q.isEmpty() || port_q.isEmpty() || user_q.isEmpty() || pwd_q.isEmpty())
        return;
    this->hide();
    auto port_s = port_q.toStdString();
    short port   = atoi(port_s.c_str());
    if(ui->checkBox_saveUser->isChecked()){
        auto fp = fopen("user.txt","w");
        if(fp){
            fprintf(fp,"%s\n%s",user_q.toStdString().c_str() , pwd_q.toStdString().c_str());
            fclose(fp);
        }
    }
    auto th = new GameThread(NULL);
    connect(th, SIGNAL(exitSignal()), this, SLOT(gameOver()));
    connect(th, SIGNAL(missionEditSignal(smoothly::game *)), this, SLOT(missionEdit(smoothly::game *)));
    th->user = user_q.toStdString();
    th->host = host_q.toStdString();
    th->pwd  = pwd_q.toStdString();
    th->port = port;
    th->start();
}

void login::gameOver(){
    this->close();
}

void login::missionEdit(smoothly::game * g){
    medit.game = g;
    medit.show();
}

GameThread::GameThread(QObject *parent):QThread(parent){

}
void GameThread::run(){
    smoothly::game * game = new smoothly::game(host.c_str() , port);
    game->cmd_login(user.c_str() , pwd.c_str());
    while(game->gameLoop()){
        if(game->openMissionEditBox){
            emit missionEditSignal(game);
            game->openMissionEditBox = false;
        }
    }
    delete game;
    emit exitSignal();
}
