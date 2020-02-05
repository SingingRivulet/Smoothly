#include "addcharacter.h"
#include "ui_addcharacter.h"

addCharacter::addCharacter(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::addCharacter)
{
    ui->setupUi(this);
}

addCharacter::~addCharacter()
{
    delete ui;
}

void addCharacter::on_buttonBox_accepted()
{
    auto u = ui->lineEdit_owner->text();
    if(u.isEmpty())
        return;

    id = ui->spinBox_id->value();
    x  = ui->doubleSpinBox_x->value();
    y  = ui->doubleSpinBox_y->value();
    z  = ui->doubleSpinBox_z->value();

    user = u.toStdString();
    doActivity = true;
}
