#ifndef SMOOTHLY_UTILS
#define SMOOTHLY_UTILS
#include <irrlicht/irrlicht.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <raknet/RakPeerInterface.h>
#include <raknet/MessageIdentifiers.h>
#include <raknet/BitStream.h>
#include <raknet/RakNetTypes.h>
#include <raknet/RakSleep.h>
#include <leveldb/db.h>
#include <list>
#include <map>
#include <set>
#include <vector>
#include <stdio.h>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <math.h>
#include <lua.hpp>
#include "bodyDiff.h"
namespace smoothly{
    
    enum GameMessage{
        MESSAGE_GAME=ID_USER_PACKET_ENUM+1
    };
    
    enum MessageMethodFlag{
        M_UPDATE_BUILDING = 'b',
        M_UPDATE_TERRAIN  = 't',
        M_UPDATE_OBJECT   = 'o',
        M_UPDATE_SUBS     = 's',
        M_UPDATE_USER     = 'u',
        M_UPDATE_ATTACHING= 'c',
        M_UPDATE_ATTACK   = 'k',
        M_ADMIN           = 'a'
    };
    
    
    enum AttackMethod{
        K_ADD_ATTACK_AM     = 'a',
        K_UPLOAD_ATTACK_AM  = 'A'
    };
    
    enum AttachingMethod{
        C_SET_ATTACHING    ='s',
        C_GET_ATTACHING    ='g',
        C_UPLOAD_ATTACHING ='u'
    };
    
    enum SubsMethod{
        S_UL_TELEPORT   ='p',
        S_DL_TELEPORT   ='P',
        S_UL_STATUS     ='^',
        S_DL_STATUS     ='S',
        S_UL_CREATE     ='c',
        S_DL_CREATE     ='C',
        S_DL_CREATE_B   ='B',
        S_UL_ATTACK     ='t',
        S_DL_ATTACK     ='T',
        S_UL_REMOVE     ='r',
        S_DL_REMOVE     ='R',
        S_RQ_CHUNK      ='k',
        S_RQ_UUID       ='i',
        S_FAIL          ='f',
        S_SET_USER_SUBS ='e',
        S_SUBS_UUID     ='d',
        S_SUBS_UUIDS    ='o',
        S_RUN_CHUNK     ='N'
    };
    
    enum UserMethod{
        U_MOVE          ='m',
        U_LOGIN         ='l',
        U_LOGOUT        ='o',
        U_CHANGE_PWD    ='c'
    };
    enum BuildingMethod{
        B_ATTACK            ='a',
        B_ATTACK_UPLOAD     ='u',
        B_CREATE            ='c',
        B_CREATE_UPLOAD     ='p',
        B_DESTROY           ='d',
        B_GENER             ='g',
        B_DOWNLOAD_CHUNK    ='l',
        B_DOWNLOAD_UUID     ='i'
    };
    
    enum TerrainMethod{
        T_REMOVE    ='r',
        T_DOWNLOAD  ='d',
        T_SEND_ONE  ='o',
        T_SEND_TABLE='t',
        T_APPLY     ='a'
    };
    
    enum AdminMethod{
        A_CREATE_USER       = 'a',
        A_SEND_USER_UUID    = 'u',
        A_TELEPORT          = 't',
        A_SET_OWNER         = 'o',
        A_SET_SUBS_STR      = 'j'
    };
    
    enum WeaponType{
        WEAPON_LASER,
        WEAPON_CLOSE,
        WEAPON_EXPLODE,
        WEAPON_SHOT,
        WEAPON_NONE
    };
    
    typedef std::vector<WeaponType> WeaponsType;
    
    enum AttackMode{
        ATTACK_RANGE_SINGLE,
        ATTACK_RANGE_DOUBLE,
        ATTACK_RANGE_BOTH,
        ATTACK_CLOSE_SINGLE,
        ATTACK_CLOSE_DOUBLE,
        ATTACK_CLOSE_BOTH,
        ATTACK_NONE
    };
    
    inline void quaternion2euler(const btQuaternion & q , irr::core::vector3df & e){
        irr::core::quaternion irrq(q.x(),q.y(),q.z(),q.w());
        irrq.toEuler(e);
    }
    inline void euler2quaternion(const irr::core::vector3df & e , btQuaternion & q){
        irr::core::quaternion irrq(e);
        q.setValue(irrq.X , irrq.Y , irrq.Z , irrq.W);
    }
    inline float getLenSq(const irr::core::vector3df & f,const irr::core::vector3df & t){
        auto l=t-f;
        return (l.X*l.X + l.Y*l.Y + l.Z*l.Z);
    }
    inline float mhtDist(const irr::core::vector3df & A,const irr::core::vector3df & B){
        irr::core::vector3df d=A-B;
        return fabs(d.X)+fabs(d.Y)+fabs(d.Z);
    }
    inline void rotate2d(irr::core::vector2df & v,double a){
        auto cosa=cos(a);
        auto sina=sin(a);
        auto x=v.X*cosa - v.Y*sina;
        auto y=v.X*sina + v.Y*cosa;
        v.X=x;
        v.Y=y;
        //v.normalize();
    }
    
    class ipair{
        public:
            int32_t x,y;
            inline bool operator==(const ipair & i)const{
                return (x==i.x) && (y==i.y);
            }
            inline bool operator<(const ipair & i)const{
                if(x<i.x)
                    return true;
                else
                if(x==i.x){
                    if(y<i.y)
                        return true;
                }
                    return false;
            }
            inline ipair & operator=(const ipair & i){
                x=i.x;
                y=i.y;
                return *this;
            }
            inline ipair(const ipair & i){
                x=i.x;
                y=i.y;
            }
            inline ipair(const int & ix , const int & iy){
                x=ix;
                y=iy;
            }
            inline ipair(){
                x=0;
                y=0;
            }
    };
    
    class mapid{
        public:
            int x,y,mapId;
            long itemId;
            inline bool operator==(const mapid & i)const{
                return (x==i.x) && (y==i.y) && (itemId==i.itemId) && (mapId==i.mapId);
            }
            inline bool operator<(const mapid & i)const{
                if(x<i.x)
                    return true;
                else
                if(x==i.x){
                    if(y<i.y)
                        return true;
                    else
                    if(y==i.y){
                        if(itemId<i.itemId)
                            return true;
                        else
                        if(itemId==i.itemId){
                            if(mapId<i.mapId)
                                return true;
                        }
                    }
                }
                    return false;
            }
            mapid(){
                x=0;
                y=0;
                itemId=0;
                mapId=0;
            }
            mapid(const mapid & i){
                x=i.x;
                y=i.y;
                itemId=i.itemId;
                mapId=i.mapId;
            }
            mapid(int ix,int iy,long iitemId,int imapId){
                x=ix;
                y=iy;
                itemId=iitemId;
                mapId=imapId;
            }
            mapid & operator=(const mapid & i){
                x=i.x;
                y=i.y;
                itemId=i.itemId;
                mapId=i.mapId;
                return *this;
        }
    };
    
    class predictableRand{
        public:
            inline int rand(){
                next = next * 1103515245 + 12345;
                return((unsigned)((next/65536)&0x7fffffff) % 32768);
            }
            inline float frand(){
                return ((float)rand())/32768.0f;
            }
            inline void setSeed(unsigned long seed){
                next=seed;
            }
        private:
            unsigned long next=1;
    };
    
    class lineReader{
        public:
            const char * str;
            const char * ptr;
            bool eof;
            inline lineReader(const char * p){
                this->str=p;
                this->ptr=p;
                this->eof=false;
            }
            inline int read(char * out , int len , char end='\n'){
                if(*ptr=='\0' || this->eof){
                    this->eof=true;
                    return 0;
                }
                while(1){
                    if(*ptr=='\0'){
                        this->eof=true;
                        return 0;
                    }
                    if(*ptr==end)
                        ++ptr;
                    else
                        break;
                }
                int i=0;
                while(1){
                    if(*ptr=='\0'){
                        this->eof=true;
                    }
                    if(i>=len-1){
                        out[i]='\0';
                        break;
                    }else
                    if(*ptr=='\0'){
                        out[i]='\0';
                        break;
                    }else
                    if(*ptr==end){
                        out[i]='\0';
                        ++ptr;
                        break;
                    }else{
                        out[i]=*ptr;
                        ++i;
                        ++ptr;
                    }
                }
                return i;
            }
    };
    
}
#endif
