#include "vlogin.h"
#include "ui_vlogin.h"

vlogin::vlogin(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::vlogin)
{
    ui->setupUi(this);
}

vlogin::~vlogin()
{
    delete ui;
}

void vlogin::on_buttonBox_accepted()
{
    auto u = ui->lineEdit_user->text();
    auto p = ui->lineEdit_pwd->text();
    if(u.isEmpty() || p.isEmpty())
        return;
    user = u.toStdString();
    pwd  = p.toStdString();
    doActivity = true;
}
