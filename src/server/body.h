#ifndef SMOOTHLY_SERVER_BODY
#define SMOOTHLY_SERVER_BODY
#include "removeTable.h"
#include <functional>
#include <map>
namespace smoothly{
namespace server{

class body:public removeTable{
    public:
        void updateBody(const std::string & uuid , int x , int y);
        void getBody(const std::string & uuid , int & x , int & y);
        void getBodies(int x,int y,std::list<std::string> & o);
        void removeBody(const std::string & uuid);
        
        //对物体的操作
        void wearing_add(const std::string & uuid , int);
        void wearing_remove(const std::string & uuid , int);
        void wearing_get(const std::string & uuid , std::set<int> & o);
        void setLookAt(const std::string & uuid , const vec3 & );
        void setStatus(const std::string & uuid , int s);
        void setPosition(const std::string & uuid , const vec3 & );
        void setRotation(const std::string & uuid , const vec3 & );
        void interactive(const std::string & uuid , const std::string & s);
        
        //带验证的操作
        void wearing_add(const std::string & user,const std::string & uuid , int);
        void wearing_remove(const std::string & user,const std::string & uuid , int);
        void wearing_get(const std::string & user,const std::string & uuid , std::set<int> & o);
        void setLookAt  (const std::string & user,const std::string & uuid , const vec3 & );
        void setStatus  (const std::string & user,const std::string & uuid , int s);
        void setPosition(const std::string & user,const std::string & uuid , const vec3 & );
        void setRotation(const std::string & user,const std::string & uuid , const vec3 & );
        void interactive(const std::string & user,const std::string & uuid , const std::string & s);
        
        void HPInc(const std::string & uuid,int delta);

        std::string addCharacter(const std::string & owner,int id,const vec3 & posi);
        void addCharacter(const std::string & uuid,const std::string & owner,int id,int hp,const vec3 & posi);
        void removeCharacter(const std::string & uuid);
        
        ipair getCharPosition(const std::string & uuid);
        
        vec3 getPosition(const std::string & uuid);
        vec3 getRotation(const std::string & uuid);
        vec3 getLookAt(const std::string & uuid);
        int getId(const std::string & uuid);
        int getStatus(const std::string & uuid);
        int getHP(const std::string & uuid);
        std::string getOwner(const std::string & uuid);

        std::string getMainControl(const std::string & user);
        void setMainControl(const std::string & user,const std::string & uuid);
        
        void sendMapToUser(const std::string & to);
        
        void sendChunk(const ipair & p , const std::string & to);
        virtual void sendRemoveTable(const ipair & p , const std::string & to)=0;
        
        void sendToUser(const std::string & uuid,const std::string & to);
        
        void getBody(
            const std::string & uuid , 
            std::function<void (
                int id,int hp,int status,const std::string & owner,
                const vec3 & p,const vec3 & r,const vec3 & l,const std::set<int> & wearing
            )> const & callback,
            std::function<void (void)> const & fail
        );
        
        //virtuals
        
        virtual void boardcast_wearing_add(const std::string & uuid,int x,int y,int d)=0;
        virtual void boardcast_wearing_remove(const std::string & uuid,int x,int y,int d)=0;
        virtual void boardcast_hp(const std::string & uuid,int x,int y,int hp)=0;
        virtual void boardcast_setStatus(const std::string & uuid,int x,int y,int s)=0;
        virtual void boardcast_setLookAt(const std::string & uuid,int x,int y,const vec3 & v)=0;
        virtual void boardcast_setPosition(const std::string & uuid,int x,int y,const vec3 & v)=0;
        virtual void boardcast_setRotation(const std::string & uuid,int x,int y,const vec3 & v)=0;
        virtual void boardcast_interactive(const std::string & uuid,int x,int y,const std::string & s)=0;
        
        virtual void boardcast_bodyRemove(const std::string & uuid,int x,int y)=0;
        
        virtual void boardcast_createBody(const std::string & uuid,int x,int y,
            int id,int hp,int status,const std::string & owner,
            const vec3 & p,const vec3 & r,const vec3 & l)=0;
        
        virtual void updateChunkDBVT(const std::string & uuid,const std::string & owner,int x, int y)=0;
        virtual void removeFromDBVT(const std::string & uuid)=0;

        virtual void send_body(const std::string & to,
            const std::string & uuid,
            int id,int hp,int status,const std::string & owner,
            const vec3 & p,const vec3 & r,const vec3 & l,
            const std::set<int> & wearing)=0;

        body();
        ~body();

        void release()override;
        void loop()override;

        class cache_bodyRota_t:public cache<vec3>{
                void onExpire(const std::string &,vec3 & )override;
                void onLoad(const std::string &, vec3 & )override;
            public:
                body * parent;
        }cache_bodyRota;
        class cache_bodyPosi_t:public cache<vec3>{
                void onExpire(const std::string &,vec3 & )override;
                void onLoad(const std::string &, vec3 & )override;
            public:
                body * parent;
        }cache_bodyPosi;
        class cache_lookat_t:public cache<vec3>{
                void onExpire(const std::string &,vec3 & )override;
                void onLoad(const std::string &, vec3 & )override;
            public:
                body * parent;
        }cache_lookat;
        void setChunkOwnerByBody(const std::string & user, const std::string & buuid);
    private:
        struct bconf{
            int hp;
        };
        std::map<int,int>    wearingToBullet;
        std::map<int,bconf*> config;
};

}//////server
}//////smoothly
#endif
