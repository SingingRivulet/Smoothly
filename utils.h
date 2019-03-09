#ifndef SMOOTHLY_UTILS
#define SMOOTHLY_UTILS
#include <irrlicht/irrlicht.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <raknet/RakPeerInterface.h>
#include <raknet/MessageIdentifiers.h>
#include <raknet/BitStream.h>
#include <raknet/RakNetTypes.h>
#include <raknet/RakSleep.h>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <lua.hpp>
namespace smoothly{
    
    enum GameMessage{
        MESSAGE_GAME=ID_USER_PACKET_ENUM+1
    };
    
    enum MessageMethodFlag{
        M_UPDATE_BUILDING = 'b',
        M_UPDATE_TERRAIN  = 't',
        M_UPDATE_OBJECT   = 'o',
        M_UPDATE_USER     = 'u'
    };
    
    enum UserMethod{
        U_SET_POSITION  ='p',
        U_SET_ROTATION  ='r',
        U_MOVE          ='m',
        U_LOGIN         ='l'
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
        T_APPLAY    ='a'
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
}
#endif
