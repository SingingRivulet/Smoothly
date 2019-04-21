#include "subsapi.h"
#include "substance.h"
namespace smoothly{
namespace subsAPI{

#define getSelfPtr \
    if(!lua_isuserdata(L,1)) \
        return 0; \
    void * sptr=lua_touserdata(L,1); \
    if(sptr==NULL) \
        return 0; \
    auto self=(substance*)sptr;

#define getSubsUUID \
    if(!lua_isstring(L,2)) \
        return 0; \
    std::string suuid=lua_tostring(L,2);

#define getSubsPtr \
    auto ptr=self->seekSubs(suuid); \
    if(ptr==NULL) \
        return 0;

#define getVector1 \
    if((!lua_isnumber(L,3)) || (!lua_isnumber(L,4)) || (!lua_isnumber(L,5))) \
        return 0; \
    float x1=lua_tonumber(L,3); \
    float y1=lua_tonumber(L,4); \
    float z1=lua_tonumber(L,5);

#define getVector2 \
    if((!lua_isnumber(L,6)) || (!lua_isnumber(L,7)) || (!lua_isnumber(L,8))) \
        return 0; \
    float x2=lua_tonumber(L,6); \
    float y2=lua_tonumber(L,7); \
    float z2=lua_tonumber(L,8);

static int getPosition(lua_State * L){
    getSelfPtr;
    getSubsUUID;
    getSubsPtr;
    
    auto p=ptr->node->getPosition();
    lua_pushnumber(L,p.X);
    lua_pushnumber(L,p.Y);
    lua_pushnumber(L,p.Z);
    return 3;
}
static int setPosition(lua_State * L){
    getSelfPtr;
    getSubsUUID;
    getSubsPtr;
    getVector1;
    
    irr::core::vector3df p(x1,y1,z1);
    ptr->setPosition(p);
    ptr->wake=true;
    
    lua_pushboolean(L,1);
    return 1;
}

static int getRotation(lua_State * L){
    getSelfPtr;
    getSubsUUID;
    getSubsPtr;
    
    auto r=ptr->node->getRotation();
    lua_pushnumber(L,r.X);
    lua_pushnumber(L,r.Y);
    lua_pushnumber(L,r.Z);
    return 3;
}
static int setRotation(lua_State * L){
    getSelfPtr;
    getSubsUUID;
    getSubsPtr;
    getVector1;
    
    irr::core::vector3df r(x1,y1,z1);
    ptr->setRotation(r);
    ptr->wake=true;
    
    lua_pushboolean(L,1);
    return 1;
}

static int getAngularVelocity(lua_State * L){
    getSelfPtr;
    getSubsUUID;
    getSubsPtr;
    
    auto a=ptr->rigidBody->getAngularVelocity();
    lua_pushnumber(L,a.getX());
    lua_pushnumber(L,a.getY());
    lua_pushnumber(L,a.getZ());
    return 3;
}
static int getLinearVelocity(lua_State * L){
    getSelfPtr;
    getSubsUUID;
    getSubsPtr;
    
    auto l=ptr->rigidBody->getLinearVelocity();
    lua_pushnumber(L,l.getX());
    lua_pushnumber(L,l.getY());
    lua_pushnumber(L,l.getZ());
    return 3;
}

static int setAngularVelocity(lua_State * L){
    getSelfPtr;
    getSubsUUID;
    getSubsPtr;
    getVector1;
    
    btVector3 a(x1,y1,z1);
    ptr->rigidBody->setAngularVelocity(a);
    ptr->wake=true;
    
    lua_pushboolean(L,1);
    return 1;
}
static int setLinearVelocity(lua_State * L){
    getSelfPtr;
    getSubsUUID;
    getSubsPtr;
    
    getVector1;
    
    btVector3 l(x1,y1,z1);
    ptr->rigidBody->setLinearVelocity(l);
    ptr->wake=true;
    
    lua_pushboolean(L,1);
    return 1;
}

static int teleport(lua_State * L){
    getSelfPtr;
    getSubsUUID;
    getVector1;
    
    irr::core::vector3df p(x1,y1,z1);
    self->requestTeleport(suuid,p);
    
    lua_pushboolean(L,1);
    return 1;
}

static int getOwner(lua_State * L){
    getSelfPtr;
    getSubsUUID;
    getSubsPtr;
    
    lua_pushstring(L,ptr->owner.c_str());
    return 1;
}
static int getMyUUID(lua_State * L){
    getSelfPtr;
    
    lua_pushstring(L,self->myUUID.c_str());
    return 1;
}

static int attack(lua_State * L){
    getSelfPtr;
    getSubsUUID;
    getSubsPtr;
    
    if(!lua_isinteger(L,3))
        return 0;
    int dmg=lua_tointeger(L,3);
    
    if(ptr->type!=mods::SUBS_LASTING)
        return 0;
    
    self->requestAttackSubs(suuid,dmg);
    
    lua_pushboolean(L,1);
    return 1;
}
static int remove(lua_State * L){
    getSelfPtr;
    getSubsUUID;
    getSubsPtr;
    
    self->removeSubs(ptr);
    
    lua_pushboolean(L,1);
    return 1;
}

static int applyImpulse(lua_State * L){
    getSelfPtr;
    getSubsUUID;
    getSubsPtr;
    getVector1;
    getVector2;
    
    btVector3 l(x1,y1,z1);
    btVector3 p(x2,y2,z2);
    ptr->rigidBody->applyImpulse(l,p);
    ptr->wake=true;
    
    lua_pushboolean(L,1);
    return 1;
}
static int applyForce(lua_State * L){
    getSelfPtr;
    getSubsUUID;
    getSubsPtr;
    getVector1;
    getVector2;
    
    btVector3 l(x1,y1,z1);
    btVector3 p(x2,y2,z2);
    ptr->rigidBody->applyForce(l,p);
    ptr->wake=true;
    
    lua_pushboolean(L,1);
    return 1;
}


static int manual(lua_State * L){
    getSelfPtr;
    getSubsUUID;
    getSubsPtr;
    
    if(ptr->owner!=self->myUUID){
        lua_pushboolean(L,0);
        return 1;
    }
    self->mainControlUUID=suuid;
    
    lua_pushboolean(L,1);
    return 1;
}

static void openlibs_subs(lua_State * L){
    struct luaL_Reg funcs[]={
        {"setPosition"                  , setPosition},
        {"setRotation"                  , setRotation},
        
        {"getPosition"                  , getPosition},
        {"getRotation"                  , getRotation},
        
        {"getAngularVelocity"           , getAngularVelocity},
        {"getLinearVelocity"            , getLinearVelocity},
        
        {"setAngularVelocity"           , setAngularVelocity},
        {"getLinearVelocity"            , getLinearVelocity},
        
        {"teleport"                     , teleport},
        
        {"getOwner"                     , getOwner},
        {"getMyUUID"                    , getMyUUID},
        
        {"attack"                       , attack},
        {"remove"                       , remove},
        
        {"applyImpulse"                 , applyImpulse},
        {"applyForce"                   , applyForce},
        
        {"manual"                       , manual},
        
        {NULL,NULL}
    };
    luaL_newlib(L,funcs);
    lua_setglobal(L,"substance");
}

void openlibs(lua_State * L){
    openlibs_subs(L);
}

}//namespace subsAPI
}//namespace smoothly