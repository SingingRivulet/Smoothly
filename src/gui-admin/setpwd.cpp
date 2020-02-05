#include "setpwd.h"
#include "ui_setpwd.h"

setPwd::setPwd(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::setPwd)
{
    ui->setupUi(this);
    setWindowTitle("设置用户密码");
}

setPwd::~setPwd()
{
    delete ui;
}

void setPwd::on_buttonBox_accepted()
{
    auto u = ui->lineEdit_user->text();
    auto p = ui->lineEdit_pwd->text();
    if(u.isEmpty() || p.isEmpty())
        return;
    user = u.toStdString();
    pwd  = p.toStdString();
    doActivity = true;
}
