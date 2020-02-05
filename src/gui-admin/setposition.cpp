#include "setposition.h"
#include "ui_setposition.h"

setPosition::setPosition(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::setPosition)
{
    ui->setupUi(this);
    setWindowTitle("设置角色控制器位置");
}

setPosition::~setPosition()
{
    delete ui;
}

void setPosition::on_buttonBox_accepted()
{
    auto u = ui->lineEdit_uuid->text();
    if(u.isEmpty())
        return;
    x = ui->doubleSpinBox_x->value();
    y = ui->doubleSpinBox_y->value();
    z = ui->doubleSpinBox_z->value();
    uuid = u.toStdString();
    doActivity = true;
}
