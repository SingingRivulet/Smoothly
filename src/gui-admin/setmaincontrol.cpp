#include "setmaincontrol.h"
#include "ui_setmaincontrol.h"

setMainControl::setMainControl(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::setMainControl)
{
    ui->setupUi(this);
}

setMainControl::~setMainControl()
{
    delete ui;
}

void setMainControl::on_buttonBox_accepted()
{
    auto u = ui->lineEdit_user->text();
    auto d = ui->lineEdit_uuid->text();
    if(u.isEmpty() || d.isEmpty())
        return;
    user = u.toStdString();
    uuid = d.toStdString();
    doActivity = true;
}
