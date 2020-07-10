#include "manager.h"
#include "ui_manager.h"

manager::manager(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::manager),
    missionEditor(this)
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

    missionEditor.connection = connection;
    missionEditor.adminKey = adminKey;

    char buf[256];
    snprintf(buf,sizeof(buf),"[连接]%s %u",host,port);
    printStatus(buf);

    recvTimer = new QTimer(this);
    connect(recvTimer, &QTimer::timeout, this, &manager::recvHandle);
    recvTimer->start(200);
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

    bs.Write(RakNet::RakString(s.user.c_str()));
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
    bs.Write(RakNet::RakString(s.uuid.c_str()));
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
    bs.Write(RakNet::RakString(s.uuid.c_str()));
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

void manager::on_commandLinkButton_mission_clicked()
{
    missionEditor.exec();
}


void manager::on_pushButton_getNearUser_clicked()
{
    if(connection==NULL)
        return;
    int32_t x = ui->spinBox_chunk_x->value();
    int32_t y = ui->spinBox_chunk_y->value();
    makeHeader("getNearUsers");
    bs.Write(x);
    bs.Write(y);
    connection->Send( &bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true );
}

void manager::on_commandLinkButton_vlogin_clicked()
{
    if(connection==NULL)
        return;
    vlogin s(this);
    s.doActivity = false;
    s.exec();
    if(!s.doActivity)
        return;

    RakNet::BitStream bs;
    bs.Write((RakNet::MessageID)(ID_USER_PACKET_ENUM+1));
    bs.Write((RakNet::MessageID)'+');
    bs.Write(RakNet::RakString(s.user.c_str()));
    bs.Write(RakNet::RakString(s.pwd.c_str()));

    connection->Send( &bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true );

    printStatus("[执行]模拟登录");
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
    RakNet::BitStream bs;//心跳包
    bs.Write((RakNet::MessageID)(ID_USER_PACKET_ENUM+1));
    bs.Write((RakNet::MessageID)'~');
    connection->Send( &bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true );
}

void manager::onRecvMessage(RakNet::Packet *data)
{
    switch(data->data[0]){
        case (ID_USER_PACKET_ENUM+1):
            switch(data->data[1]){
                case 'a':
                {
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
                    }else
                    if(heads=="nearUsers"){
                        int32_t x,y;
                        RakNet::RakString u;
                        bs.Read(x);
                        bs.Read(y);
                        bs.Read(u);
                        printStatus(QString("[")+"("+QString::number(x)+","+QString::number(y)+")"+"附近用户]"+u);
                    }
                }
                break;
                case '.':
                    {
                        if(data->length<4){
                            break;
                        }
                        RakNet::BitStream bs(data->data,data->length,false);
                        bs.IgnoreBytes(4);

                        switch(data->data[2]){
                            case 'R':
                                switch(data->data[3]){
                                    case '+':
                                        {
                                            int32_t x,y,id,index;
                                            bs.Read(x);
                                            bs.Read(y);
                                            bs.Read(id);
                                            bs.Read(index);
                                            printStatus(QString("[登录响应:addRemovedItem](")+
                                                        QString::number(x)+","+
                                                        QString::number(y)+","+
                                                        QString::number(id)+","+
                                                        QString::number(index)+")");
                                        }
                                        break;
                                    case '=':
                                        {
                                            int32_t x,y,len;
                                            bs.Read(x);
                                            bs.Read(y);
                                            bs.Read(len);
                                            printStatus(QString("[登录响应:setRemovedItem](")+
                                                        QString::number(x)+","+
                                                        QString::number(y)+") 数量:"+
                                                        QString::number(len));
                                        }
                                        break;
                                }
                                break;
                            case 'B':
                                switch(data->data[3]){
                                    case 'A':
                                        {
                                            RakNet::RakString u;
                                            int32_t d;
                                            bs.Read(u);
                                            bs.Read(d);
                                            printStatus(QString("[登录响应:wearing_add]")+u.C_String()+" => "+QString::number(d));
                                        }
                                        break;
                                    case 'R':
                                        {
                                            RakNet::RakString u;
                                            int32_t d;
                                            bs.Read(u);
                                            bs.Read(d);
                                            printStatus(QString("[登录响应:wearing_remove]")+u.C_String()+" => "+QString::number(d));
                                        }
                                        break;
                                    case 'G':
                                        {
                                            RakNet::RakString u;
                                            int32_t len;
                                            bs.Read(u);
                                            bs.Read(len);
                                            printStatus(QString("[登录响应:wearing_set]")+u.C_String()+" 数量:"+QString::number(len));
                                        }
                                        break;
                                    case 'H':
                                        {
                                            RakNet::RakString u;
                                            int32_t d;
                                            bs.Read(u);
                                            bs.Read(d);
                                            printStatus(QString("[登录响应:HPInc]")+u.C_String()+" => "+QString::number(d));
                                        }
                                        break;
                                    case 'S':
                                        {
                                            RakNet::RakString u;
                                            int32_t d;
                                            bs.Read(u);
                                            bs.Read(d);
                                            printStatus(QString("[登录响应:setStatus]")+u.C_String()+" => "+QString::number(d));
                                        }
                                        break;
                                    case 'l':
                                        {
                                            float x,y,z;
                                            RakNet::RakString u;
                                            bs.Read(u);
                                            bs.ReadVector(x,y,z);
                                            printStatus(QString("[登录响应:setLookAt]")+
                                                        u.C_String()+" => ("+
                                                        QString::number(x)+","+
                                                        QString::number(y)+","+
                                                        QString::number(z)+")");
                                        }
                                        break;
                                    case 'p':
                                        {
                                            float x,y,z;
                                            RakNet::RakString u;
                                            bs.Read(u);
                                            bs.ReadVector(x,y,z);
                                            printStatus(QString("[登录响应:setPosition]")+
                                                        u.C_String()+" => ("+
                                                        QString::number(x)+","+
                                                        QString::number(y)+","+
                                                        QString::number(z)+")");
                                        }
                                        break;
                                    case 'r':
                                        {
                                            float x,y,z;
                                            RakNet::RakString u;
                                            bs.Read(u);
                                            bs.ReadVector(x,y,z);
                                            printStatus(QString("[登录响应:setRotation]")+
                                                        u.C_String()+" => ("+
                                                        QString::number(x)+","+
                                                        QString::number(y)+","+
                                                        QString::number(z)+")");
                                        }
                                        break;
                                    case 'i':
                                        {
                                            RakNet::RakString u,s;
                                            bs.Read(u);
                                            bs.Read(s);
                                            printStatus(QString("[登录响应:interactive]")+u.C_String()+" => "+s.C_String());
                                        }
                                        break;
                                    case '-':
                                        {
                                            RakNet::RakString u;
                                            bs.Read(u);
                                            printStatus(QString("[登录响应:removeBody]")+u.C_String());
                                        }
                                        break;
                                    case '+':
                                        {
                                            RakNet::RakString u,owner;
                                            int32_t id,hp,status;
                                            float px,py,pz,
                                                    rx,ry,rz,
                                                    lx,ly,lz;
                                            bs.Read(u);
                                            bs.Read(id);
                                            bs.Read(hp);
                                            bs.Read(status);
                                            bs.Read(owner);
                                            bs.ReadVector(px,py,pz);
                                            bs.ReadVector(rx,ry,rz);
                                            bs.ReadVector(lx,ly,lz);
                                            printStatus(QString("[登录响应:createBody]")+
                                                        "\n uuid:"+u.C_String()+
                                                        "\n owner:"+owner.C_String()+
                                                        "\n id:"+QString::number(id)+
                                                        "\n hp:"+QString::number(hp)+
                                                        "\n status:"+QString::number(status)+
                                                        "\n position:("+QString::number(px)+","+QString::number(py)+","+QString::number(pz)+")"+
                                                        "\n rotation:("+QString::number(rx)+","+QString::number(ry)+","+QString::number(rz)+")"+
                                                        "\n lookAt:("+QString::number(lx)+","+QString::number(ly)+","+QString::number(lz)+")");
                                        }
                                        break;
                                    case '=':
                                        {
                                            RakNet::RakString u,owner;
                                            int32_t id,hp,status,len;
                                            float px,py,pz,
                                                    rx,ry,rz,
                                                    lx,ly,lz;
                                            bs.Read(u);
                                            bs.Read(id);
                                            bs.Read(hp);
                                            bs.Read(status);
                                            bs.Read(owner);
                                            bs.ReadVector(px,py,pz);
                                            bs.ReadVector(rx,ry,rz);
                                            bs.ReadVector(lx,ly,lz);
                                            bs.Read(len);
                                            printStatus(QString("[登录响应:setBody]")+
                                                        "\n uuid:"+u.C_String()+
                                                        "\n owner:"+owner.C_String()+
                                                        "\n id:"+QString::number(id)+
                                                        "\n hp:"+QString::number(hp)+
                                                        "\n status:"+QString::number(status)+
                                                        "\n position:("+QString::number(px)+","+QString::number(py)+","+QString::number(pz)+")"+
                                                        "\n rotation:("+QString::number(rx)+","+QString::number(ry)+","+QString::number(rz)+")"+
                                                        "\n lookAt:("+QString::number(lx)+","+QString::number(ly)+","+QString::number(lz)+")"+
                                                        "\n wearing数量:"+QString::number(len));
                                        }
                                        break;
                                    case '/':
                                        {
                                            RakNet::RakString u;
                                            bs.Read(u);
                                            printStatus(QString("[登录响应:setMainControl]")+u.C_String());
                                        }
                                        break;
                                }
                                break;
                            case 'S':
                                switch(data->data[3]){
                                    case 'A':
                                        {
                                            RakNet::RakString u;
                                            int32_t id;
                                            float fX,fY,fZ,dX,dY,dZ;
                                            bs.Read(u);
                                            bs.Read(id);
                                            bs.ReadVector(fX ,fY ,fZ);
                                            bs.ReadVector(dX ,dY ,dZ);
                                        }
                                        //ctl_fire(&bs);
                                        break;
                                }
                                break;
                            case 'I':
                                switch(data->data[3]){
                                    case 'm':
                                        {
                                            RakNet::RakString uuid,text;
                                            bs.Read(uuid);
                                            bs.Read(text);
                                            missionEditor.setUUID(uuid.C_String());
                                            missionEditor.loadString(text.C_String());
                                        }
                                        break;
                                    case 'L':
                                        {
                                            int32_t len;
                                            bs.Read(len);
                                            RakNet::RakString uuid;
                                            missionEditor.clearChildren();
                                            for(auto i=0;i<len;++i){
                                                if(bs.Read(uuid)){
                                                    missionEditor.addMission(uuid.C_String());
                                                }else{
                                                    break;
                                                }
                                            }
                                        }
                                        break;
                                    case 't':
                                        {
                                            RakNet::RakString uuid,text;
                                            bs.Read(uuid);
                                            bs.Read(text);
                                            missionEditor.setText(text.C_String());
                                        }
                                        break;
                                    case 'c':
                                        {
                                            RakNet::RakString uuid;
                                            float x,y,z;
                                            bs.Read(uuid);
                                            bs.ReadVector(x,y,z);
                                            missionEditor.addMission(uuid.C_String());
                                        }
                                        break;
                                }
                                break;
                        }
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
