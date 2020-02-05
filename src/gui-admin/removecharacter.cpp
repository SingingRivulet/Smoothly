#include "removecharacter.h"
#include "ui_removecharacter.h"

removeCharacter::removeCharacter(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::removeCharacter)
{
    ui->setupUi(this);
    setWindowTitle("删除角色控制器");
}

removeCharacter::~removeCharacter()
{
    delete ui;
}

void removeCharacter::on_buttonBox_accepted()
{
    auto u = ui->lineEdit_uuid->text();
    if(u.isEmpty())
        return;
    uuid = u.toStdString();
    doActivity = true;
}
