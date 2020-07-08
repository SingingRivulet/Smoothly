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
#include <map>
#include <set>
#include <time.h>
#include <functional>
#include <vector>

namespace smoothly{
namespace client{
///////////////////////
#define makeHeader(c,a) \
    RakNet::BitStream bs; \
    bs.Write((RakNet::MessageID)(ID_USER_PACKET_ENUM+1)); \
    bs.Write((RakNet::MessageID)'.'); \
    bs.Write((RakNet::MessageID)c); \
    bs.Write((RakNet::MessageID)a);
class mapItem{
    public:
        int32_t id,index;
        inline bool operator==(const mapItem & i)const{
            return (id==i.id) && (index==i.index);
        }
        inline bool operator<(const mapItem & i)const{
            if(id<i.id)
                return true;
            else
            if(id==i.id){
                if(index<i.index)
                    return true;
            }
                return false;
        }
        inline mapItem & operator=(const mapItem & i){
            id=i.id;
            index=i.index;
            return *this;
        }
        inline mapItem(const mapItem & i){
            id=i.id;
            index=i.index;
        }
        inline mapItem(const int & ix , const int & iy){
            id=ix;
            index=iy;
        }
        inline mapItem(){
            id=0;
            index=0;
        }
};
///////////////////////
class connectionBase{
    RakNet::RakPeerInterface * connection;
    public:
        std::string myUUID;

        connectionBase(){
            lastHeartbeat = 0;
        }

        inline void connect(const char * addr,unsigned short port){
            connection=RakNet::RakPeerInterface::GetInstance();
            RakNet::SocketDescriptor sd;
            connection->Startup(1, &sd, 1);
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
    
            clock_t starts,ends;
            starts=clock();
            while(1){
                ends = clock();
                if((ends-starts)/(CLOCKS_PER_SEC/1000)>5)
                    break;
                
                auto pk=connection->Receive();
                if(pk)
                    onRecvMessage(pk);
                else
                    break;
            }

            auto ntm = time(0);
            if(ntm - lastHeartbeat>1){
                lastHeartbeat = ntm;
                RakNet::BitStream bs;//心跳包
                bs.Write((RakNet::MessageID)(ID_USER_PACKET_ENUM+1));
                bs.Write((RakNet::MessageID)'~');
                connection->Send( &bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true );
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
                                case '=':
                                    switch(data->data[3]){
                                        case 'r':
                                            ctl_setVisualRange(&bs);
                                        break;
                                    }
                                break;
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
                                        case '/':
                                            ctl_setMainControl(&bs);
                                        break;
                                    }
                                break;
                                case 'S':
                                    switch(data->data[3]){
                                        case 'A':
                                            ctl_fire(&bs);
                                        break;
                                    }
                                break;
                                case 'T':
                                    switch (data->data[3]) {
                                        case '+':
                                            ctl_addBuilding(&bs);
                                        break;
                                        case '-':
                                            ctl_removeBuilding(&bs);
                                        break;
                                        case '~':
                                            ctl_startChunk(&bs);
                                        break;
                                    }
                                break;
                                case 'P':
                                    switch (data->data[3]) {
                                        case '=':
                                            ctl_setBag(&bs);
                                        break;
                                        case '+':
                                            ctl_bag_tool_add(&bs);
                                        break;
                                        case '-':
                                            ctl_bag_tool_remove(&bs);
                                        break;
                                        case '<':
                                            ctl_bag_tool_use(&bs);
                                        break;
                                        case 'R':
                                            ctl_setBagResource(&bs);
                                        break;
                                        case 'G':
                                            ctl_setBagTool(&bs);
                                        break;
                                        case 'D':
                                            ctl_setBagToolDur(&bs);
                                        break;
                                    }
                                break;
                                case 'p':
                                    switch (data->data[3]) {
                                        case '+':
                                            ctl_package_add(&bs);
                                        break;
                                        case '-':
                                            ctl_package_remove(&bs);
                                        break;
                                    }
                                break;
                                case 't':
                                    switch (data->data[3]) {
                                        case 'u':
                                            ctl_unlockTech(&bs);
                                        break;
                                        case 't':
                                            ctl_techTarget(&bs);
                                        break;
                                    }
                                break;
                                case 'M':
                                    switch (data->data[3]) {
                                        case 's':
                                            ctl_makeStatus(&bs);
                                        break;
                                    }
                                break;
                                case 'G':
                                    switch (data->data[3]) {
                                        case 'a':
                                            ctl_chunkACL(&bs);
                                        break;
                                    }
                                break;
                                case 'I':
                                    switch (data->data[3]) {
                                        case 'm':
                                            ctl_setMission(&bs);
                                        break;
                                        case 'c':
                                            ctl_addMission(&bs);
                                        break;
                                        case 's':
                                            ctl_submitMissionStatus(&bs);
                                        break;
                                        case 'L':
                                            ctl_missionList(&bs);
                                        break;
                                        case 't':
                                            ctl_missionText(&bs);
                                        break;
                                    }
                                break;
                                case 'L':
                                    switch (data->data[3]) {
                                        case 'n':
                                            ctl_newMail(&bs);
                                        break;
                                        case 'm':
                                            ctl_mail(&bs);
                                        break;
                                        case 'u':
                                            ctl_mailPackagePickedUp(&bs);
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
        inline void cmd_addRemovedItem(int cx,int cy,float x,float y,int id,int index){
            makeHeader('R','+');
            bs.Write((int32_t)cx);
            bs.Write((int32_t)cy);
            bs.Write(x);
            bs.Write(y);
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
        inline void cmd_getBody(const std::string & uuid){
            makeHeader('B','?');
            RakNet::RakString u=uuid.c_str();
            bs.Write(u);
            sendMessage(&bs);
        }
        inline void cmd_login(const std::string & uuid,const std::string & pwd){
            myUUID = uuid;
            RakNet::BitStream bs;
            bs.Write((RakNet::MessageID)(ID_USER_PACKET_ENUM+1));
            bs.Write((RakNet::MessageID)'+');
            RakNet::RakString u=uuid.c_str();
            RakNet::RakString p=pwd.c_str();
            bs.Write(u);
            bs.Write(p);
            sendMessage(&bs);
        }
        inline void cmd_fire(const std::string & uuid,int id,float fx,float fy,float fz,float dx,float dy,float dz){
            makeHeader('S','A');
            RakNet::RakString u=uuid.c_str();
            bs.Write(u);
            bs.Write((int32_t)id);
            bs.WriteVector(fx ,fy ,fz);
            bs.WriteVector(dx ,dy ,dz);
            sendMessage(&bs);
        }
        inline void cmd_addBuilding(int id,float px,float py,float pz,float rx,float ry,float rz,std::function<void(RakNet::BitStream*)> callback){
            makeHeader('T','+');
            bs.Write((int32_t)id);
            bs.WriteVector(px ,py ,pz);
            bs.WriteVector(rx ,ry ,rz);
            callback(&bs);
            sendMessage(&bs);
        }
        inline void cmd_damageBuilding(const std::string & uuid,int dh){
            makeHeader('T','-');
            RakNet::RakString u=uuid.c_str();
            bs.Write(u);
            bs.Write((int32_t)dh);
            sendMessage(&bs);
        }
        inline void cmd_getBuilding(int32_t x , int32_t y){
            makeHeader('T','G');
            bs.Write(x);
            bs.Write(y);
            sendMessage(&bs);
        }
        inline void cmd_getBagTool(const char * uuid){
            makeHeader('P','G');
            RakNet::RakString u = uuid;
            bs.Write(u);
            sendMessage(&bs);
        }
        inline void cmd_useBagTool(const char * uuid,const char * tuuid){
            makeHeader('P','U');
            RakNet::RakString u = uuid;
            bs.Write(u);
            u = tuuid;
            bs.Write(u);
            sendMessage(&bs);
        }
        inline void cmd_reloadBagTool(const char * uuid,const char * tuuid){
            makeHeader('P','R');
            RakNet::RakString u = uuid;
            bs.Write(u);
            u = tuuid;
            bs.Write(u);
            sendMessage(&bs);
        }
        inline void cmd_putPackage(int32_t skin,float x,float y,float z,const char * bag,int32_t id,int32_t num){
            makeHeader('p','p');
            bool isResource = true;
            bs.Write(isResource);
            bs.Write(skin);
            bs.WriteVector(x,y,z);
            bs.Write(RakNet::RakString(bag));
            bs.Write(id);
            bs.Write(num);
            sendMessage(&bs);
        }
        inline void cmd_putPackage(int32_t skin,float x,float y,float z,const char * bag,const char * package){
            makeHeader('p','p');
            bool isResource = false;
            bs.Write(isResource);
            bs.Write(skin);
            bs.WriteVector(x,y,z);
            bs.Write(RakNet::RakString(bag));
            bs.Write(RakNet::RakString(package));
            sendMessage(&bs);
        }
        inline void cmd_pickupPackage(const char * bag,int32_t x,int32_t y,const char * package){
            makeHeader('p','u');
            bs.Write(RakNet::RakString(bag));
            bs.Write(x);
            bs.Write(y);
            bs.Write(RakNet::RakString(package));
            sendMessage(&bs);
        }
        inline void cmd_make(int32_t id,float x,float y,float z,const char * bag){
            makeHeader('M','m');
            bs.Write(id);
            bs.WriteVector(x,y,z);
            bs.Write(RakNet::RakString(bag));
            sendMessage(&bs);
        }
        inline void cmd_setTechTarget(int32_t target){
            makeHeader('t','t');
            bs.Write(target);
            sendMessage(&bs);
        }
        inline void cmd_getChunkACL(int32_t x,int32_t y){
            makeHeader('G','g');
            bs.Write(x);
            bs.Write(y);
            sendMessage(&bs);
        }
        inline void cmd_setChunkACL(int32_t x,int32_t y,bool b,bool c,bool t){
            makeHeader('G','s');
            bs.Write(x);
            bs.Write(y);
            bs.Write(b);
            bs.Write(c);
            bs.Write(t);
            sendMessage(&bs);
        }
        inline void cmd_getMission(const char * muuid){
            makeHeader('I','g');
            bs.Write(RakNet::RakString(muuid));
            sendMessage(&bs);
        }
        inline void cmd_getChunkMission(int32_t x,int32_t y){
            makeHeader('I','c');
            bs.Write(x);
            bs.Write(y);
            sendMessage(&bs);
        }
        inline void cmd_submitMission(const char * muuid,const char * body){
            makeHeader('I','s');
            bs.Write(RakNet::RakString(muuid));
            bs.Write(RakNet::RakString(body));
            sendMessage(&bs);
        }
        inline void cmd_addMission(float x,float y,float z,bool needArrive,bool showPosition,const char * parent,const char * des,const char * text){
            makeHeader('I','a');
            bs.WriteVector(x,y,z);
            bs.Write(needArrive);
            bs.Write(showPosition);
            bs.Write(RakNet::RakString(parent));
            bs.Write(RakNet::RakString(des));
            bs.Write(RakNet::RakString(text));
            sendMessage(&bs);
        }
        inline void cmd_goParentMission(){
            makeHeader('I','<');
            sendMessage(&bs);
        }
        inline void cmd_giveUpMission(){
            makeHeader('I','u');
            sendMessage(&bs);
        }

        inline void cmd_setChunkOwner(const char * buuid){
            makeHeader('G','+');
            bs.Write(RakNet::RakString(buuid));
            sendMessage(&bs);
        }
        inline void cmd_giveUpChunk(int32_t x , int32_t y){
            makeHeader('G','-');
            bs.Write(x);
            bs.Write(y);
            sendMessage(&bs);
        }
        inline void cmd_getMail(){
            makeHeader('L','g');
            sendMessage(&bs);
        }
        inline void cmd_pickupMailPackage(const char * buuid , const char * mpuuid){
            makeHeader('L','p');
            bs.Write(RakNet::RakString(buuid));
            bs.Write(RakNet::RakString(mpuuid));
            sendMessage(&bs);
        }
    private:
        //controllers
        inline void ctl_setVisualRange(RakNet::BitStream * data){
            int32_t r;
            data->Read(r);
            msg_setVisualRange(r);
        }
        inline void ctl_addRemovedItem(RakNet::BitStream * data){
            int32_t x,y,id,index;
            data->Read(x);
            data->Read(y);
            data->Read(id);
            data->Read(index);
            msg_addRemovedItem(x,y,id,index);
        }
        inline void ctl_setRemovedItem(RakNet::BitStream * data){
            std::set<mapItem> rmt;
            int32_t x,y,len,buf1,buf2;
            data->Read(x);
            data->Read(y);
            data->Read(len);
            for(int i=0;i<len;i++){
                data->Read(buf1);
                data->Read(buf2);
                rmt.insert(mapItem(buf1,buf2));
            }
            msg_setRemovedItem(x,y,rmt);
        }
        inline void ctl_wearing_add(RakNet::BitStream * data){
            RakNet::RakString u;
            int32_t d;
            data->Read(u);
            data->Read(d);
            msg_wearing_add(u.C_String(),d);
        }
        inline void ctl_wearing_remove(RakNet::BitStream * data){
            RakNet::RakString u;
            int32_t d;
            data->Read(u);
            data->Read(d);
            msg_wearing_remove(u.C_String(),d);
        }
        inline void ctl_wearing_set(RakNet::BitStream * data){
            RakNet::RakString u;
            int32_t len,buf;
            std::set<int> wearing;
            data->Read(u);
            data->Read(len);
            for(int i=0;i<len;i++){
                data->Read(buf);
                wearing.insert(buf);
            }
            msg_wearing_set(u.C_String(),wearing);
        }
        inline void ctl_HPInc(RakNet::BitStream * data){
            RakNet::RakString u;
            int32_t d;
            data->Read(u);
            data->Read(d);
            msg_HPInc(u.C_String(),d);
        }
        inline void ctl_setStatus(RakNet::BitStream * data){
            RakNet::RakString u;
            int32_t d;
            data->Read(u);
            data->Read(d);
            msg_setStatus(u.C_String(),d);
        }
        inline void ctl_setLookAt(RakNet::BitStream * data){
            float x,y,z;
            RakNet::RakString u;
            data->Read(u);
            data->ReadVector(x,y,z);
            msg_setLookAt(u.C_String(),x,y,z);
        }
        inline void ctl_setPosition(RakNet::BitStream * data){
            float x,y,z;
            RakNet::RakString u;
            data->Read(u);
            data->ReadVector(x,y,z);
            msg_setPosition(u.C_String(),x,y,z);
        }
        inline void ctl_setRotation(RakNet::BitStream * data){
            float x,y,z;
            RakNet::RakString u;
            data->Read(u);
            data->ReadVector(x,y,z);
            msg_setRotation(u.C_String(),x,y,z);
        }
        inline void ctl_interactive(RakNet::BitStream * data){
            RakNet::RakString u,s;
            data->Read(u);
            data->Read(s);
            msg_interactive(u.C_String() , s.C_String());
        }
        inline void ctl_removeBody(RakNet::BitStream * data){
            RakNet::RakString u;
            data->Read(u);
            msg_removeBody(u.C_String());
        }
        inline void ctl_createBody(RakNet::BitStream * data){
            RakNet::RakString u,owner;
            int32_t id,hp,status;
            float px,py,pz,
                  rx,ry,rz,
                  lx,ly,lz;
            data->Read(u);
            data->Read(id);
            data->Read(hp);
            data->Read(status);
            data->Read(owner);
            data->ReadVector(px,py,pz);
            data->ReadVector(rx,ry,rz);
            data->ReadVector(lx,ly,lz);
            msg_createBody(
                u.C_String(),id,hp,status,owner.C_String(),
                px,py,pz,
                rx,ry,rz,
                lx,ly,lz
            );
        }
        inline void ctl_setBody(RakNet::BitStream * data){
            RakNet::RakString u,owner;
            int32_t id,hp,status,len,buf;
            std::set<int> wearing;
            float px,py,pz,
                  rx,ry,rz,
                  lx,ly,lz;
            data->Read(u);
            data->Read(id);
            data->Read(hp);
            data->Read(status);
            data->Read(owner);
            data->ReadVector(px,py,pz);
            data->ReadVector(rx,ry,rz);
            data->ReadVector(lx,ly,lz);
            data->Read(len);
            for(int i=0;i<len;i++){
                data->Read(buf);
                wearing.insert(buf);
            }
            msg_setBody(
                u.C_String(),id,hp,status,owner.C_String(),
                px,py,pz,
                rx,ry,rz,
                lx,ly,lz,
                wearing
            );
        }
        inline void ctl_setMainControl(RakNet::BitStream * data){
            RakNet::RakString u;
            data->Read(u);
            msg_setMainControl(u.C_String());
        }
        inline void ctl_fire(RakNet::BitStream * data){
            RakNet::RakString u;
            int32_t id;
            float fX,fY,fZ,dX,dY,dZ;
            data->Read(u);
            data->Read(id);
            data->ReadVector(fX ,fY ,fZ);
            data->ReadVector(dX ,dY ,dZ);
            msg_fire(u.C_String(),id,fX,fY,fZ,dX,dY,dZ);
        }
        inline void ctl_addBuilding(RakNet::BitStream * data){
            RakNet::RakString u;
            int32_t id;
            float px,py,pz,
                  rx,ry,rz;
            data->Read(u);
            data->Read(id);
            data->ReadVector(px ,py ,pz);
            data->ReadVector(rx ,ry ,rz);
            msg_addBuilding(u.C_String(),id,px,py,pz,rx,ry,rz);
        }
        inline void ctl_removeBuilding(RakNet::BitStream * data){
            RakNet::RakString u;
            data->Read(u);
            msg_removeBuilding(u.C_String());
        }
        inline void ctl_startChunk(RakNet::BitStream * data){
            int32_t x,y;
            data->Read(x);
            data->Read(y);
            msg_startChunk(x,y);
        }
        inline void ctl_setBag(RakNet::BitStream * data){
            RakNet::RakString uuid,t;
            data->Read(uuid);
            data->Read(t);
            msg_setBag(uuid.C_String(),t.C_String());
        }
        inline void ctl_setBagResource(RakNet::BitStream * data){
            RakNet::RakString uuid;
            int32_t id,num;
            data->Read(uuid);
            data->Read(id);
            data->Read(num);
            msg_setBagResource(uuid.C_String(),id,num);
        }
        inline void ctl_setBagTool(RakNet::BitStream * data){
            RakNet::RakString uuid,str;
            data->Read(uuid);
            data->Read(str);
            msg_setBagTool(uuid.C_String(),str.C_String());
        }
        inline void ctl_setBagToolDur(RakNet::BitStream * data){
            RakNet::RakString uuid;
            int32_t dur,pwr;
            data->Read(uuid);
            data->Read(dur);
            data->Read(pwr);
            msg_setBagToolDur(uuid.C_String(),dur,pwr);
        }
        inline void ctl_bag_tool_add(RakNet::BitStream * data){
            RakNet::RakString uuid,tuuid;
            data->Read(uuid);
            data->Read(tuuid);
            msg_bag_tool_add(uuid.C_String(),tuuid.C_String());
        }
        inline void ctl_bag_tool_remove(RakNet::BitStream * data){
            RakNet::RakString uuid,tuuid;
            data->Read(uuid);
            data->Read(tuuid);
            msg_bag_tool_remove(uuid.C_String(),tuuid.C_String());
        }
        inline void ctl_bag_tool_use(RakNet::BitStream * data){
            RakNet::RakString uuid,tuuid;
            data->Read(uuid);
            data->Read(tuuid);
            msg_bag_tool_use(uuid.C_String(),tuuid.C_String());
        }
        inline void ctl_package_add(RakNet::BitStream * data){
            int32_t x,y;
            RakNet::RakString uuid,text;
            data->Read(x);
            data->Read(y);
            data->Read(uuid);
            data->Read(text);
            msg_package_add(x,y,uuid.C_String(),text.C_String());
        }
        inline void ctl_package_remove(RakNet::BitStream * data){
            int32_t x,y;
            RakNet::RakString uuid;
            data->Read(x);
            data->Read(y);
            data->Read(uuid);
            msg_package_remove(x,y,uuid.C_String());
        }
        inline void ctl_unlockTech(RakNet::BitStream * data){
            bool newTech;
            int32_t id;
            data->Read(newTech);
            data->Read(id);
            msg_unlockTech(newTech,id);
        }
        inline void ctl_techTarget(RakNet::BitStream * data){
            bool newTarget;
            int32_t target;
            data->Read(newTarget);
            data->Read(target);
            msg_techTarget(newTarget,target);
        }
        inline void ctl_makeStatus(RakNet::BitStream * data){
            int32_t id;
            bool status;
            data->Read(id);
            data->Read(status);
            msg_makeStatus(id,status);
        }
        inline void ctl_chunkACL(RakNet::BitStream * data){
            int32_t x,y;
            bool allowBuildingWrite;    //是否允许修改建筑物
            bool allowCharacterDamage;  //是否允许单位受到伤害
            bool allowTerrainItemWrite; //是否允许修改地形物体（如植被）
            RakNet::RakString owner;
            data->Read(x);
            data->Read(y);
            data->Read(allowBuildingWrite);
            data->Read(allowCharacterDamage);
            data->Read(allowTerrainItemWrite);
            data->Read(owner);
            msg_chunkACL(x,y,allowBuildingWrite,allowCharacterDamage,allowTerrainItemWrite,owner.C_String());
        }
        inline void ctl_addMission(RakNet::BitStream * data){
            RakNet::RakString uuid;
            float x,y,z;
            data->Read(uuid);
            data->ReadVector(x,y,z);
            msg_addMission(uuid.C_String() , x,y,z);
        }
        inline void ctl_setMission(RakNet::BitStream * data){
            RakNet::RakString uuid,text;
            data->Read(uuid);
            data->Read(text);
            msg_setMission(uuid.C_String() , text.C_String());
        }
        inline void ctl_submitMissionStatus(RakNet::BitStream * data){
            RakNet::RakString uuid;
            bool status;
            data->Read(uuid);
            data->Read(status);
            msg_submitMissionStatus(uuid.C_String() , status);
        }
        inline void ctl_missionList(RakNet::BitStream * data){
            int32_t len;
            std::vector<std::string> missions;
            data->Read(len);
            RakNet::RakString uuid;
            for(auto i=0;i<len;++i){
                if(data->Read(uuid)){
                    missions.push_back(uuid.C_String());
                }else{
                    break;
                }
            }
            msg_missionList(missions);
        }
        inline void ctl_missionText(RakNet::BitStream * data){
            RakNet::RakString uuid,text;
            data->Read(uuid);
            data->Read(text);
            msg_missionText(uuid.C_String() , text.C_String());
        }
        inline void ctl_newMail(RakNet::BitStream * ){
            msg_newMail();
        }
        inline void ctl_mail(RakNet::BitStream * data){
            RakNet::RakString text;
            data->Read(text);
            msg_mail(text.C_String());
        }
        inline void ctl_mailPackagePickedUp(RakNet::BitStream * data){
            msg_mailPackagePickedUp();
        }

        time_t lastHeartbeat;

    public:
        virtual void msg_setVisualRange(int v)=0;
        virtual void msg_addRemovedItem(int x,int y,int,int)=0;
        virtual void msg_setRemovedItem(int x,int y,const std::set<mapItem> &)=0;
        virtual void msg_wearing_add(const char*,int d)=0;
        virtual void msg_wearing_set(const char*,const std::set<int> &)=0;
        virtual void msg_wearing_remove(const char*,int d)=0;
        virtual void msg_HPInc(const char*,int d)=0;
        virtual void msg_setStatus(const char*,int d)=0;
        virtual void msg_setLookAt(const char*,float,float,float)=0;
        virtual void msg_setPosition(const char*,float,float,float)=0;
        virtual void msg_setRotation(const char*,float,float,float)=0;
        virtual void msg_interactive(const char*,const char*)=0;
        virtual void msg_removeBody(const char*)=0;
        virtual void msg_createBody(const char*,int,int,int,const char*,float,float,float,float,float,float,float,float,float)=0;
        virtual void msg_setBody(const char*,int,int,int,const char*,float,float,float,float,float,float,float,float,float,const std::set<int> &)=0;
        virtual void msg_setMainControl(const char *)=0;
        virtual void msg_fire(const char *,int,float,float,float,float,float,float)=0;
        virtual void msg_addBuilding(const char *,int,float,float,float,float,float,float)=0;
        virtual void msg_removeBuilding(const char *)=0;
        virtual void msg_startChunk(int,int)=0;
        virtual void msg_setBag(const char *,const char *)=0;
        virtual void msg_setBagResource(const char *,int,int)=0;
        virtual void msg_setBagTool(const char *,const char *)=0;
        virtual void msg_setBagToolDur(const char *,int,int)=0;
        virtual void msg_bag_tool_add(const char *,const char *)=0;
        virtual void msg_bag_tool_remove(const char *,const char *)=0;
        virtual void msg_bag_tool_use(const char *,const char *)=0;
        virtual void msg_package_add(int32_t x,int32_t y,const char * uuid,const char * text)=0;
        virtual void msg_package_remove(int32_t x,int32_t y,const char * uuid)=0;
        virtual void msg_unlockTech(bool newtech,int32_t id)=0;
        virtual void msg_techTarget(bool newtarget,int32_t target)=0;
        virtual void msg_makeStatus(int32_t id,bool status)=0;
        virtual void msg_chunkACL(int32_t x,int32_t y,bool b,bool c,bool t,const char * owner)=0;
        virtual void msg_addMission(const char * uuid , float x,float y,float z)=0;
        virtual void msg_setMission(const char * uuid , const char * text)=0;
        virtual void msg_submitMissionStatus(const char * uuid , bool status)=0;
        virtual void msg_missionList(const std::vector<std::string> & missions)=0;
        virtual void msg_missionText(const char * uuid , const char * text)=0;
        virtual void msg_newMail()=0;
        virtual void msg_mail(const char * text)=0;
        virtual void msg_mailPackagePickedUp()=0;
};
///////////////////////
}//////client
}//////smoothly
#endif
