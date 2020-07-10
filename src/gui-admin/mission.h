#ifndef MISSION_H
#define MISSION_H

#include <QDialog>
#include <raknet/RakPeerInterface.h>
#include <raknet/MessageIdentifiers.h>
#include <raknet/BitStream.h>
#include <raknet/RakNetTypes.h>
#include <raknet/RakSleep.h>
#include <QListWidget>
#include <QTableWidget>

namespace Ui {
class mission;
}

class mission : public QDialog
{
        Q_OBJECT

    public:
        explicit mission(QWidget *parent = 0);
        ~mission();

        RakNet::RakPeerInterface * connection;
        RakNet::RakString adminKey;

    private:
        Ui::mission *ui;

    public:
        void loadString(const char * s);
        void setUUID(const char * s);
        void addMission(const char * u);
        void setText(const char * t);
        void clear();
        void clearChildren();

    private slots:
        void on_pushButton_remove_clicked();
        void on_listWidget_missions_itemDoubleClicked(QListWidgetItem *item);
        void on_pushButton_load_uuid_clicked();
        void on_pushButton_save_clicked();
        void on_pushButton_load_chunk_clicked();
        void on_pushButton_load_children_clicked();
        void on_pushButton_need_add_submit_clicked();
        void on_pushButton_reward_add_submit_clicked();
        void on_tableWidget_need_itemDoubleClicked(QTableWidgetItem *item);
        void on_tableWidget_reward_itemDoubleClicked(QTableWidgetItem *item);
        void on_pushButton_submit_clicked();
};

#endif // MISSION_H
