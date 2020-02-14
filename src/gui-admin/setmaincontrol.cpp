#include "setmaincontrol.h"
#include "ui_setmaincontrol.h"

setMainControl::setMainControl(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::setMainControl)
{
    ui->setupUi(this);
    setWindowTitle("设置第一人称控制");

    QRegExp rx = QRegExp("[0-9a-z\\-]*");
    QRegExpValidator *validator = new QRegExpValidator(rx);
    ui->lineEdit_uuid->setValidator(validator);
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
