//本库尽可能做到精简，便于其他引擎调用
#ifndef SMOOTHLY_CLIENT_CONNECTION
#define SMOOTHLY_CLIENT_CONNECTION
#include <raknet/RakPeerInterface.h>
#include <raknet/MessageIdentifiers.h>
#include <raknet/BitStream.h>
#include <raknet/RakNetTypes.h>
#include <raknet/RakSleep.h>
#include <chrono>   
#include <string>   
namespace smoothly{
namespace client{
///////////////////////
#define makeHeader(c,a) \
    RakNet::BitStream bs; \
    bs.Write((RakNet::MessageID)(ID_USER_PACKET_ENUM+1)); \
    bs.Write((RakNet::MessageID)'.'); \
    bs.Write((RakNet::MessageID)c); \
    bs.Write((RakNet::MessageID)a);
///////////////////////
class connection{
    public:
        RakNet::RakPeerInterface * connection;
        
        inline void connect(const char * addr,unsigned short port){
            connection->Connect(addr,port,0,0);
            RakSleep(30);//sleep 30微秒后才能正常发送，原因未知
        }
        inline void sendMessage(RakNet::BitStream * data){
            connection->Send( data, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true );  
        }
        inline void sendMessageU(RakNet::BitStream * data){
            connection->Send( data, HIGH_PRIORITY, RELIABLE, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true );  
        }
        inline void shutdown(){
            connection->Shutdown(300);
            RakNet::RakPeerInterface::DestroyInstance(connection);
            connection=NULL;
        }
        inline void recv(){
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
        inline void onRecvMessage(RakNet::Packet * data){
            switch(data->data[0]){
                case (ID_USER_PACKET_ENUM+1):
                    switch(data->data[1]){
                        case '.':
                            if(data->length<4){
                                break;
                            }
                            RakNet::BitStream bs(data->data,data->length,false);
                            bs.IgnoreBytes(4);
                            
                            switch(data->data[2]){
                                case 'R':
                                    switch(data->data[3]){
                                        case '+':
                                            ctl_addRemovedItem(&bs);
                                        break;
                                        case '=':
                                            ctl_setRemovedItem(&bs);
                                        break;
                                    }
                                break;
                                case 'B':
                                    switch(data->data[3]){
                                        case 'A':
                                            ctl_wearing_add(&bs);
                                        break;
                                        case 'R':
                                            ctl_wearing_remove(&bs);
                                        break;
                                        case 'G':
                                            ctl_wearing_set(&bs);
                                        break;
                                        case 'H':
                                            ctl_HPInc(&bs);
                                        break;
                                        case 'S':
                                            ctl_setStatus(&bs);
                                        break;
                                        case 'l':
                                            ctl_setLookAt(&bs);
                                        break;
                                        case 'p':
                                            ctl_setPosition(&bs);
                                        break;
                                        case 'r':
                                            ctl_setRotation(&bs);
                                        break;
                                        case 'i':
                                            ctl_interactive(&bs);
                                        break;
                                        case '-':
                                            ctl_removeBody(&bs);
                                        break;
                                        case '+':
                                            ctl_createBody(&bs);
                                        break;
                                        case '=':
                                            ctl_setBody(&bs);
                                        break;
                                    }
                                break;
                            }
                            
                        break;
                    }
                break;
            }
        }
        
        //cmds
        inline void cmd_addRemovedItem(int x,int y,int id,int index){
            makeHeader('R','+');
            bs.Write((int32_t)x);
            bs.Write((int32_t)y);
            bs.Write((int32_t)id);
            bs.Write((int32_t)index);
            sendMessageU(&bs);
        }
        inline void cmd_wearing_add(const std::string & uuid,int d){
            makeHeader('B','A');
            RakNet::RakString u=uuid.c_str();
            bs.Write(u);
            bs.Write((int32_t)d);
            sendMessage(&bs);
        }
        inline void cmd_wearing_remove(const std::string & uuid,int d){
            makeHeader('B','R');
            RakNet::RakString u=uuid.c_str();
            bs.Write(u);
            bs.Write((int32_t)d);
            sendMessage(&bs);
        }
        inline void cmd_wearing_get(const std::string & uuid){
            makeHeader('B','G');
            RakNet::RakString u=uuid.c_str();
            bs.Write(u);
            sendMessageU(&bs);
        }
        inline void cmd_HPInc(const std::string & uuid,int d){
            makeHeader('B','H');
            RakNet::RakString u=uuid.c_str();
            bs.Write(u);
            bs.Write((int32_t)d);
            sendMessage(&bs);
        }
        inline void cmd_setStatus(const std::string & uuid,int d){
            makeHeader('B','S');
            RakNet::RakString u=uuid.c_str();
            bs.Write(u);
            bs.Write((int32_t)d);
            sendMessage(&bs);
        }
        inline void cmd_setLookAt(const std::string & uuid,float x,float y,float z){
            makeHeader('B','l');
            RakNet::RakString u=uuid.c_str();
            bs.Write(u);
            bs.WriteVector(x,y,z);
            sendMessage(&bs);
        }
        inline void cmd_setPosition(const std::string & uuid,float x,float y,float z){
            makeHeader('B','p');
            RakNet::RakString u=uuid.c_str();
            bs.Write(u);
            bs.WriteVector(x,y,z);
            sendMessage(&bs);
        }
        inline void cmd_setRotation(const std::string & uuid,float x,float y,float z){
            makeHeader('B','r');
            RakNet::RakString u=uuid.c_str();
            bs.Write(u);
            bs.WriteVector(x,y,z);
            sendMessage(&bs);
        }
        inline void cmd_interactive(const std::string & uuid,const std::string & ss){
            makeHeader('B','i');
            RakNet::RakString u=uuid.c_str();
            RakNet::RakString s=ss.c_str();
            bs.Write(u);
            bs.Write(s);
            sendMessage(&bs);
        }
        
        //controllers
        virtual void ctl_addRemovedItem(RakNet::BitStream * data)=0;
        virtual void ctl_setRemovedItem(RakNet::BitStream * data)=0;
        
        virtual void ctl_wearing_add(RakNet::BitStream * data)=0;
        virtual void ctl_wearing_remove(RakNet::BitStream * data)=0;
        virtual void ctl_wearing_set(RakNet::BitStream * data)=0;
        virtual void ctl_HPInc(RakNet::BitStream * data)=0;
        virtual void ctl_setStatus(RakNet::BitStream * data)=0;
        virtual void ctl_setLookAt(RakNet::BitStream * data)=0;
        virtual void ctl_setPosition(RakNet::BitStream * data)=0;
        virtual void ctl_setRotation(RakNet::BitStream * data)=0;
        virtual void ctl_interactive(RakNet::BitStream * data)=0;
        virtual void ctl_removeBody(RakNet::BitStream * data)=0;
        virtual void ctl_createBody(RakNet::BitStream * data)=0;
        virtual void ctl_setBody(RakNet::BitStream * data)=0;
};
///////////////////////
}//////client
}//////smoothly
#endif