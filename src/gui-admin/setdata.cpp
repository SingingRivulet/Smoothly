#include "setdata.h"
#include "ui_setdata.h"

setData::setData(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::setData)
{
    ui->setupUi(this);
    setWindowTitle("写入");
}

setData::~setData()
{
    delete ui;
}

void setData::on_buttonBox_accepted()
{
    auto k = ui->lineEdit_key->text();
    auto v = ui->textEdit_value->toPlainText();
    if(k.isEmpty() || v.isEmpty())
        return;
    key = k.toStdString();
    val = v.toStdString();
    doActivity = true;
}
