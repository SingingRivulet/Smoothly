#ifndef MANAGER_H
#define MANAGER_H

#include <QMainWindow>
#include <raknet/RakPeerInterface.h>
#include <raknet/MessageIdentifiers.h>
#include <raknet/BitStream.h>
#include <raknet/RakNetTypes.h>
#include <raknet/RakSleep.h>
#include <QTimer>

#include "setpwd.h"
#include "setmaincontrol.h"
#include "addcharacter.h"
#include "removecharacter.h"

namespace Ui {
class manager;
}

class manager : public QMainWindow
{
    Q_OBJECT

public:
    explicit manager(QWidget *parent = 0);
    void connectServer(const char * host , unsigned short port , const std::string & key);
    ~manager();

private slots:
    void on_commandLinkButton_setPwd_clicked();

    void on_commandLinkButton_setMainControl_clicked();

    void on_commandLinkButton_addCharacter_clicked();

    void on_commandLinkButton_removeCharacter_clicked();

    void on_result_textChanged();

    void recvHandle();

private:
    Ui::manager *ui;
    RakNet::RakPeerInterface * connection;
    RakNet::RakString adminKey;

    QTimer * recvTimer;
    void recvMessage();
    void onRecvMessage(RakNet::Packet * data);
    void printStatus(const QString & str);
};

#endif // MANAGER_H
