#include "missioneditor.h"
#include "ui_missioneditor.h"
#include <QtEvents>

missionEditor::missionEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::missionEditor)
{
    ui->setupUi(this);
    setWindowTitle("添加任务");
}

missionEditor::~missionEditor(){
    delete ui;
}

void missionEditor::closeEvent(QCloseEvent * event){
    hide();
    event->ignore();
}


void missionEditor::on_pushButton_clicked(){
    auto title = ui->lineEdit_title->text();
    auto text  = ui->textEdit->toPlainText();
    auto showp = ui->checkBox_showPosi->isChecked();
    game->addMission(title.toStdString() , text.toStdString(),showp);
    ui->lineEdit_title->setText("");
    ui->textEdit->setPlainText("");
    hide();
}
