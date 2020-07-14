#include "mission.h"
#include "ui_mission.h"
#include "../utils/cJSON.h"
#include <QDebug>
#include <QMessageBox>

mission::mission(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::mission)
{
    ui->setupUi(this);
    setWindowTitle("任务编辑器");

    ui->tableWidget_need->setColumnCount(2);
    ui->tableWidget_need->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget_need->setColumnWidth(0,48);
    ui->tableWidget_need->setColumnWidth(1,48);
    ui->tableWidget_need->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->lineEdit_need_add_id->setValidator(new QRegExpValidator(QRegExp("[\\-0-9]*")));

    ui->tableWidget_reward->setColumnCount(2);
    ui->tableWidget_reward->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget_reward->setColumnWidth(0,48);
    ui->tableWidget_reward->setColumnWidth(1,48);
    ui->tableWidget_reward->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->lineEdit_reward_add_id->setValidator(new QRegExpValidator(QRegExp("[\\-0-9]*")));
}

mission::~mission()
{
    delete ui;
}

#define makeHeader(a) \
    RakNet::BitStream bs; \
    bs.Write((RakNet::MessageID)(ID_USER_PACKET_ENUM+1)); \
    bs.Write((RakNet::MessageID)'a'); \
    bs.Write(adminKey); \
    bs.Write(RakNet::RakString(a));

#define makeGameHeader(c,a) \
    RakNet::BitStream bs; \
    bs.Write((RakNet::MessageID)(ID_USER_PACKET_ENUM+1)); \
    bs.Write((RakNet::MessageID)'.'); \
    bs.Write((RakNet::MessageID)c); \
    bs.Write((RakNet::MessageID)a);

void mission::loadString(const char * s){
    auto json = cJSON_Parse(s);

    if(json){

        cJSON *c=json->child;
        while(c){
            if(c->type==cJSON_Number){
                if(strcmp(c->string,"x")==0){
                    //position.X = c->valuedouble;
                }else if(strcmp(c->string,"y")==0){
                    //position.Y = c->valuedouble;
                }else if(strcmp(c->string,"z")==0){
                    //position.Z = c->valuedouble;
                }else if(strcmp(c->string,"needArrive")==0){
                    //needArrive = (c->valueint!=0);
                }else if(strcmp(c->string,"showPosition")==0){
                    ui->checkBox_showPosition->setChecked((c->valueint!=0));
                }
            }else if(c->type==cJSON_String){
                if(strcmp(c->string,"parent")==0){
                    ui->lineEdit_parent_uuid->setText(c->valuestring);
                    ui->lineEdit_parent_uuid_2->setText(c->valuestring);
                }else if(strcmp(c->string,"description")==0){
                    ui->lineEdit_title->setText(c->valuestring);
                }else if(strcmp(c->string,"author")==0){
                    ui->label_author->setText(c->valuestring);
                }
            }else if(c->type==cJSON_Object){
                if(strcmp(c->string,"need")==0){
                    auto line = c->child;
                    while(line){
                        if(line->type == cJSON_Number){
                            int id = atoi(line->string);
                            int num = line->valueint;
                            int index = ui->tableWidget_need->rowCount();
                            ui->tableWidget_need->setRowCount(index + 1);
                            ui->tableWidget_need->setItem(index,0,new QTableWidgetItem(QString::number(id)));
                            ui->tableWidget_need->setItem(index,1,new QTableWidgetItem(QString::number(num)));
                            //need.push_back(resource_t(id,num));
                        }
                        line = line->next;
                    }
                }else if(strcmp(c->string,"reward")==0){
                    auto line = c->child;
                    while(line){
                        if(line->type == cJSON_Number){
                            int id = atoi(line->string);
                            int num = line->valueint;
                            int index = ui->tableWidget_reward->rowCount();
                            ui->tableWidget_reward->setRowCount(index + 1);
                            ui->tableWidget_reward->setItem(index,0,new QTableWidgetItem(QString::number(id)));
                            ui->tableWidget_reward->setItem(index,1,new QTableWidgetItem(QString::number(num)));
                            //reward.push_back(resource_t(id,num));
                        }
                        line = line->next;
                    }
                }
            }
            c = c->next;
        }

        cJSON_Delete(json);
    }
}

void mission::setUUID(const char * s)
{
    ui->lineEdit_uuid->setText(s);
}

void mission::addMission(const char * u)
{
    ui->listWidget_missions->addItem(new QListWidgetItem(u));
}

void mission::setText(const char * t)
{
    ui->textEdit_text->setText(t);
}

void mission::clear()
{
    ui->textEdit_text->clear();
    ui->lineEdit_uuid->clear();
    ui->tableWidget_need->clear();
    ui->tableWidget_need->setRowCount(0);
    ui->tableWidget_reward->clear();
    ui->tableWidget_reward->setRowCount(0);
    ui->label_author->clear();
    ui->lineEdit_title->clear();
    ui->lineEdit_parent_uuid_2->clear();
}

void mission::clearChildren()
{
    ui->listWidget_missions->clear();
}

void mission::on_pushButton_remove_clicked()
{

    if(QMessageBox::information(this, "删除", "确实要删除吗?",QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes){
        makeHeader("removeMission");
        bs.Write(RakNet::RakString(ui->lineEdit_uuid->text().toStdString().c_str()));
        clear();
        connection->Send( &bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true );
        QMessageBox::information(this, "成功", "删除成功");
    }
}

void mission::on_listWidget_missions_itemDoubleClicked(QListWidgetItem *item)
{
    ui->lineEdit_uuid->setText(item->text());
}

void mission::on_pushButton_load_uuid_clicked()
{
    auto u = RakNet::RakString(ui->lineEdit_uuid->text().toStdString().c_str());
    clear();
    {
        makeGameHeader('I','g');
        bs.Write(u);
        connection->Send( &bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true );
    }
    {
        makeGameHeader('I','t');
        bs.Write(u);
        connection->Send( &bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true );
    }
}

void mission::on_pushButton_save_clicked()
{
    auto uuid = ui->lineEdit_uuid->text();

    cJSON * json = cJSON_CreateObject();

    cJSON * res = cJSON_CreateObject();
    cJSON_AddItemToObject(json,"need",res);
    int len = ui->tableWidget_need->rowCount();
    for (int i=0;i<len;i++){
        auto id = ui->tableWidget_need->item(i,0)->text();
        int num = ui->tableWidget_need->item(i,1)->text().toInt();
        cJSON_AddNumberToObject(res , id.toStdString().c_str() , num);
    }

    res = cJSON_CreateObject();
    cJSON_AddItemToObject(json,"reward",res);
    len = ui->tableWidget_reward->rowCount();
    for (int i=0;i<len;i++){
        auto id = ui->tableWidget_reward->item(i,0)->text();
        int num = ui->tableWidget_reward->item(i,1)->text().toInt();
        cJSON_AddNumberToObject(res , id.toStdString().c_str() , num);
    }

    char * pp = cJSON_PrintUnformatted(json);
    if(pp){
        makeHeader("setMission");
        bs.Write(RakNet::RakString(uuid.toStdString().c_str()));
        bs.Write(RakNet::RakString(pp));
        connection->Send( &bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true );
        free(pp);
    }
    cJSON_Delete(json);
}

void mission::on_pushButton_load_chunk_clicked()
{
    clearChildren();
    int32_t x = ui->lineEdit_x->text().toInt();
    int32_t y = ui->lineEdit_y->text().toInt();
    makeGameHeader('I','c');
    bs.Write(x);
    bs.Write(y);
    connection->Send( &bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true );
}

void mission::on_pushButton_load_children_clicked()
{
    auto par = ui->lineEdit_parent_uuid->text();
    makeGameHeader('I','n');
    bs.Write(RakNet::RakString(par.toStdString().c_str()));
    connection->Send( &bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true );
}

void mission::on_pushButton_need_add_submit_clicked()
{
    int index = ui->tableWidget_need->rowCount();
    ui->tableWidget_need->setRowCount(index + 1);
    ui->tableWidget_need->setItem(index,0,new QTableWidgetItem(ui->lineEdit_need_add_id->text()));
    ui->tableWidget_need->setItem(index,1,new QTableWidgetItem(QString::number(ui->spinBox_need_add_num->value())));
}

void mission::on_pushButton_reward_add_submit_clicked()
{
    int index = ui->tableWidget_reward->rowCount();
    ui->tableWidget_reward->setRowCount(index + 1);
    ui->tableWidget_reward->setItem(index,0,new QTableWidgetItem(ui->lineEdit_reward_add_id->text()));
    ui->tableWidget_reward->setItem(index,1,new QTableWidgetItem(QString::number(ui->spinBox_reward_add_num->value())));
}

void mission::on_tableWidget_need_itemDoubleClicked(QTableWidgetItem *item)
{
    int rowIndex = item->row();
    if (rowIndex != -1)
        ui->tableWidget_need->removeRow(rowIndex);
}

void mission::on_tableWidget_reward_itemDoubleClicked(QTableWidgetItem *item)
{
    int rowIndex = item->row();
    if (rowIndex != -1)
        ui->tableWidget_reward->removeRow(rowIndex);
}

void mission::on_pushButton_submit_clicked()
{
    auto uuid = RakNet::RakString(ui->lineEdit_uuid->text().toStdString().c_str());

    cJSON * json = cJSON_CreateObject();

    cJSON_AddStringToObject(json,"description" ,ui->lineEdit_title->text().toStdString().c_str());
    cJSON_AddNumberToObject(json,"showPosition",ui->checkBox_showPosition->isChecked());

    char * pp = cJSON_PrintUnformatted(json);
    if(pp){
        {
            makeHeader("setMission");
            bs.Write(uuid);
            bs.Write(RakNet::RakString(pp));
            connection->Send( &bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true );
        }
        {
            makeHeader("setMissionText");
            bs.Write(uuid);
            bs.Write(RakNet::RakString(ui->textEdit_text->toPlainText().toStdString().c_str()));
            connection->Send( &bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true );
        }
        free(pp);
    }
    cJSON_Delete(json);
}
