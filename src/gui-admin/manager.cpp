#include "manager.h"
#include "ui_manager.h"

manager::manager(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::manager)
{
    ui->setupUi(this);
    setWindowTitle("管理");
    connection = NULL;
}

void manager::connectServer(const char *host, unsigned short port, const std::string &key)
{
    connection=RakNet::RakPeerInterface::GetInstance();
    RakNet::SocketDescriptor sd;
    connection->Startup(1, &sd, 1);
    if(connection->Connect(host,port,0,0)){
    }
    RakSleep(30);//sleep 30微秒后才能正常发送，原因未知
    adminKey = key.c_str();

    char buf[256];
    snprintf(buf,sizeof(buf),"[连接]%s %u",host,port);
    printStatus(buf);

    recvTimer = new QTimer(this);
    connect(recvTimer, &QTimer::timeout, this, &manager::recvHandle);
    recvTimer->start(1000);
}

manager::~manager()
{
    delete ui;
    if(connection){
        connection->Shutdown(300);
        RakNet::RakPeerInterface::DestroyInstance(connection);
        connection=NULL;
    }
}

#define makeHeader(a) \
    RakNet::BitStream bs; \
    bs.Write((RakNet::MessageID)(ID_USER_PACKET_ENUM+1)); \
    bs.Write((RakNet::MessageID)'a'); \
    bs.Write(adminKey); \
    bs.Write(RakNet::RakString(a));

void manager::on_commandLinkButton_setPwd_clicked()
{
    if(connection==NULL)
        return;
    setPwd s(this);
    s.doActivity = false;
    s.exec();
    if(!s.doActivity)
        return;
    makeHeader("setPwd");
    bs.Write(RakNet::RakString(s.user.c_str()));
    bs.Write(RakNet::RakString(s.pwd.c_str()));
    connection->Send( &bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true );
    printStatus("[执行]setPwd");
}

void manager::on_commandLinkButton_setMainControl_clicked()
{
    if(connection==NULL)
        return;
    setMainControl s(this);
    s.doActivity = false;
    s.exec();
    if(!s.doActivity)
        return;
    makeHeader("setMainControl");
    bs.Write(RakNet::RakString(s.user.c_str()));
    bs.Write(RakNet::RakString(s.uuid.c_str()));
    connection->Send( &bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true );
    printStatus("[执行]setMainControl");
}

void manager::on_commandLinkButton_addCharacter_clicked()
{
    if(connection==NULL)
        return;
    addCharacter s(this);
    s.doActivity = false;
    s.exec();
    if(!s.doActivity)
        return;
    makeHeader("addCharacter");

    bs.Write(s.user);
    bs.Write(s.id);
    bs.WriteVector(s.x , s.y , s.z);

    connection->Send( &bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true );

    printStatus("[执行]addCharacter");
}

void manager::on_commandLinkButton_removeCharacter_clicked()
{
    if(connection==NULL)
        return;
    removeCharacter s(this);
    s.doActivity = false;
    s.exec();
    if(!s.doActivity)
        return;

    makeHeader("removeCharacter");
    bs.Write(s.uuid);
    connection->Send( &bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true );
    printStatus("[执行]removeCharacter");
}

void manager::on_commandLinkButton_setPosition_clicked()
{
    if(connection==NULL)
        return;
    setPosition s(this);
    s.doActivity = false;
    s.exec();
    if(!s.doActivity)
        return;

    makeHeader("setPosition");
    bs.Write(s.uuid);
    bs.WriteVector(s.x , s.y , s.z);
    connection->Send( &bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true );
    printStatus("[执行]setPosition");
}

void manager::on_pushButton_db_get_clicked()
{
    if(connection==NULL)
        return;
    auto s = ui->lineEdit_db_key->text();
    if(!s.isEmpty()){
        makeHeader("getData");
        bs.Write(RakNet::RakString(s.toStdString().c_str()));
        connection->Send( &bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true );
    }
}

void manager::on_pushButton_db_put_clicked()
{
    if(connection==NULL)
        return;
    setData s(this);
    s.doActivity = false;
    s.exec();
    if(!s.doActivity)
        return;
    makeHeader("setData");
    bs.Write(RakNet::RakString(s.key.c_str()));
    bs.Write(RakNet::RakString(s.val.c_str()));
    connection->Send( &bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true );
    printStatus("[执行]插入数据库");
}

void manager::recvMessage()
{
    if(!connection)
        return;

    auto start = std::chrono::system_clock::now();

    while(1){
        auto end = std::chrono::system_clock::now();

        if((std::chrono::duration<double,std::milli>(end - start).count())>80)
            break;

        auto pk=connection->Receive();
        if(pk)
            onRecvMessage(pk);
        else
            break;
    }
}

void manager::onRecvMessage(RakNet::Packet *data)
{
    switch(data->data[0]){
        case (ID_USER_PACKET_ENUM+1):
            switch(data->data[1]){
                case 'a':
                    if(data->length<4){
                        break;
                    }
                    RakNet::BitStream bs(data->data,data->length,false);
                    bs.IgnoreBytes(2);

                    RakNet::RakString heads;
                    bs.Read(heads);
                    if(heads=="OK"){
                        RakNet::RakString act,u;
                        bs.Read(act);
                        bs.Read(u);
                        printStatus(QString("[ok]")+act+" "+u);
                    }else
                    if(heads=="addCharacter"){
                        RakNet::RakString r;
                        bs.Read(r);
                        if(r.IsEmpty())
                            printStatus("[addCharacter]添加失败");
                        else
                            printStatus(QString("[addCharacter]")+r);
                    }else
                    if(heads=="dbRes"){
                        RakNet::RakString k,v;
                        bs.Read(k);
                        bs.Read(v);
                        printStatus(QString("[数据库]")+k+" = "+v);
                    }

                break;
            }
        break;
    }
}

void manager::printStatus(const QString & str){
    ui->result->textCursor().insertText(str+"\n");
}

void manager::on_result_textChanged()
{
    ui->result->moveCursor(QTextCursor::End);
}

void manager::recvHandle()
{
    recvMessage();
}
