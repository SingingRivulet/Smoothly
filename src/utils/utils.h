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
#include <leveldb/write_batch.h>
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
#include "cJSON.h"

namespace smoothly{
    
    typedef irr::core::vector3df vec3;
    
    enum GameMessage{
        MESSAGE_GAME=ID_USER_PACKET_ENUM+1
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
    inline bool isPrefix(const std::string & pfx , const std::string & str){
        auto p=pfx.c_str();
        auto s=str.c_str();
        int len = pfx.size();
        for(int i=0;i<len;i++){
            if(p[i]!=s[i])
                return false;
        }
        return true;
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
            int itemId;
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
