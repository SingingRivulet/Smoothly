#include "utils.h"
#include <string>
#include <map>
#include <stdlib.h>
#include <math.h>
#include <time.h>

std::map<std::string,std::string> args;
RakNet::RakPeerInterface * connection;

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
void makeBSHeader(RakNet::BitStream * data){
    data->Write((RakNet::MessageID)smoothly::MESSAGE_GAME);
    data->Write((RakNet::MessageID)smoothly::M_ADMIN);
}

void makeAuth(RakNet::BitStream * data){
    RakNet::RakString u=args["--adminName"].c_str();
    RakNet::RakString p=args["--password"].c_str();
    data->Write(u);
    data->Write(p);
}

void sendMessage(RakNet::BitStream * data){
    connection->Send( data, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true );  
}
////////////////////////////////////////////////////////////////////////
int markedTime;
void markTime(){
    time_t t;
    t = time(NULL);
    markedTime = time(&t);
}
bool timeOut(){
    time_t t;
    t = time(NULL);
    int nt = time(&t);
    return abs(nt-markedTime)>5;
}

////////////////////////////////////////////////////////////////////////
//////////////////////////////operate///////////////////////////////////
////////////////////////////////////////////////////////////////////////
void createUser(){
    int64_t id=atoi(args["--nsid"].c_str());
    irr::core::vector3df position(
        atof(args["--X"].c_str()),
        atof(args["--Y"].c_str()),
        atof(args["--Z"].c_str())
    );
    const std::string & p=args["--npwd"];
    
    RakNet::BitStream bs;
    RakNet::RakString pwd=p.c_str();
    
    makeBSHeader(&bs);
    bs.Write((RakNet::MessageID)smoothly::A_CREATE_USER);
    makeAuth(&bs);
    
    bs.Write(id);
    bs.WriteVector(position.X,position.Y,position.Z);
    bs.Write(pwd);
    
    RakSleep(30);
    sendMessage(&bs);
    
    markTime();
    while(1){
        RakSleep(30);
        if(timeOut()){
            printf("[connect]time out\n");
            break;
        }
        auto pk=connection->Receive();
        if(pk){
            printf("[connect]received\n");
            if(
                pk->data[0]==smoothly::MESSAGE_GAME && 
                pk->data[1]==smoothly::M_ADMIN && 
                pk->data[2]==smoothly::A_SEND_USER_UUID
            ){
                RakNet::BitStream recv(pk->data,pk->length,false);
                recv.IgnoreBytes(3);
                RakNet::RakString uuid;
                recv.Read(uuid);
                printf("[createUser]%s\n",uuid.C_String());
                break;
            }
        }
    }
}

void teleport(){
    irr::core::vector3df position(
        atof(args["--X"].c_str()),
        atof(args["--Y"].c_str()),
        atof(args["--Z"].c_str())
    );
    const std::string & u=args["--substance"];
    
    RakNet::BitStream bs;
    RakNet::RakString uuid=u.c_str();
    
    makeBSHeader(&bs);
    bs.Write((RakNet::MessageID)smoothly::A_TELEPORT);
    makeAuth(&bs);
    
    bs.Write(uuid);
    bs.WriteVector(position.X,position.Y,position.Z);
    
    RakSleep(30);
    sendMessage(&bs);
}

void setOwner(){
    const std::string & u=args["--substance"];
    const std::string & o=args["--owner"];
    
    RakNet::BitStream bs;
    RakNet::RakString uuid =u.c_str();
    RakNet::RakString owner=o.c_str();
    
    makeBSHeader(&bs);
    bs.Write((RakNet::MessageID)smoothly::A_SET_OWNER);
    makeAuth(&bs);
    
    bs.Write(uuid);
    bs.Write(owner);
    
    RakSleep(30);
    sendMessage(&bs);
}

void setSubsStr(){
    const std::string & u=args["--substance"];
    const std::string & s=args["--json"];
    
    RakNet::BitStream bs;
    RakNet::RakString uuid =u.c_str();
    RakNet::RakString json =s.c_str();
    
    makeBSHeader(&bs);
    bs.Write((RakNet::MessageID)smoothly::A_SET_SUBS_STR);
    makeAuth(&bs);
    
    bs.Write(uuid);
    bs.Write(json);
    
    RakSleep(30);
    sendMessage(&bs);
}
///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////解析参数/////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
bool isKey(const char * str){
    return (str[0]=='-' && str[1]=='-');
}

void getArgs(int argc , char * argv[]){
    if(argc>1){
        for(int i=1;i<argc;i++){
            if(isKey(argv[i])){//这个参数是key
                if((i+1)<argc){//下一个参数存在
                    if(isKey(argv[i+1])){//下一个参数是key
                        args[argv[i]];
                    }else{
                        args[argv[i]]=argv[i+1];
                        ++i;
                    }
                }else{
                    args[argv[i]];
                }
            }
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc , char * argv[]){
    getArgs(argc,argv);
    
    printf(
        "Smoothly Admin Client\n"
        "A Open Source First Person Sandbox Strategy game\n\n"
        "Singing Rivulet Project(https://github.com/SingingRivulet)\n"
        "By cgoxopx<cgoxopx@qq.com>\n\n"
    );
    
    {
        std::string address="127.0.0.1";
        unsigned short port=39065;
        auto addrit=args.find("--server");
        if(addrit!=args.end()){
            address=addrit->second;
        }
        auto portit=args.find("--port");
        if(portit!=args.end()){
            port=atoi(portit->second.c_str());
        }
        connection=RakNet::RakPeerInterface::GetInstance();
        RakNet::SocketDescriptor sd;
        connection->Startup(1,&sd,1);
        connection->Connect(address.c_str(),port,0,0);
        printf("[connect]%s:%d\n",address.c_str(),port);
    }
    
    if(!connection->IsActive()){
        printf("[connect]fail\n");
        return 1;
    }
    
    {
        auto it=args.find("--mode");
        if(it!=args.end()){
            if(it->second=="createUser"){
                printf("[mode]createUser\n");
                createUser();
            }else
            if(it->second=="teleport"){
                printf("[mode]teleport\n");
                teleport();
            }else
            if(it->second=="setOwner"){
                printf("[mode]setOwner\n");
                setOwner();
            }else
            if(it->second=="setSubsStr"){
                printf("[mode]setSubsStr\n");
                setSubsStr();
            }
        }
    }
    
    printf("[connect]disconnect\n");
    connection->Shutdown(300);
    RakNet::RakPeerInterface::DestroyInstance(connection);
    return 0;
}