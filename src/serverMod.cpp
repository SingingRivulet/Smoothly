#include "serverMod.h"
namespace smoothly{
int serverMod::getUserMaxSubs(){
    return userMaxSubs;
}
int serverMod::getHPByType(long type){
    auto it=buildingHPMap.find(type);
    if(it==buildingHPMap.end())
        return -1;
    else
        return it->second;
}
bool serverMod::getSubsConf(long id , bool & isLasting , int & hp){
    auto it=subsMap.find(id);
    if(it==subsMap.end())
        return false;
    else{
        isLasting=it->second.lasting;
        hp=it->second.hp;
        return true;
    }
}
int serverMod::itemidNum(long id){
    auto it=buildingHPMap.find(id);
    if(it==buildingHPMap.end())
        return 0;
    else
        return it->second;
}
bool serverMod::checkAdminPwd(const std::string & name,const std::string & pwd){
    auto it=adminPwd.find(name);
    if(it!=adminPwd.end()){
        if(it->second==pwd)
            return true;
    }
    return false;
}
static int mod_addTerrainItemID(lua_State * L){
    if(!lua_isuserdata(L,1))
        return 0;
    void * ptr=lua_touserdata(L,1);
    if(ptr==NULL)
        return 0;
    auto self=(serverMod*)ptr;
    int id=luaL_checkinteger(L,2);
    int num=luaL_checkinteger(L,3);
    
    self->terrainNumMap[id]=num;
    
    lua_pushboolean(L,1);
    return 1;
}
static int mod_addSubsConf(lua_State * L){
    if(!lua_isuserdata(L,1))
        return 0;
    void * ptr=lua_touserdata(L,1);
    if(ptr==NULL)
        return 0;
    auto self=(serverMod*)ptr;
    int id=luaL_checkinteger(L,2);
    
    bool lasting=true;
    if(lua_isboolean(L,3))
        lasting=lua_toboolean(L,3);
    else
        return 0;
    
    int hp=luaL_checkinteger(L,4);
    
    serverMod::subsConf & s=self->subsMap[id];
    s.lasting=lasting;
    s.hp=hp;
    
    lua_pushboolean(L,1);
    return 1;
}
static int mod_addBuildingTypeHP(lua_State * L){
    if(!lua_isuserdata(L,1))
        return 0;
    void * ptr=lua_touserdata(L,1);
    if(ptr==NULL)
        return 0;
    auto self=(serverMod*)ptr;
    int id=luaL_checkinteger(L,2);
    int hp=luaL_checkinteger(L,3);
    
    self->buildingHPMap[id]=hp;
    
    lua_pushboolean(L,1);
    return 1;
}
static int mod_setViewRange(lua_State * L){
    if(!lua_isuserdata(L,1))
        return 0;
    void * ptr=lua_touserdata(L,1);
    if(ptr==NULL)
        return 0;
    auto self=(serverMod*)ptr;
    self->viewRange=luaL_checknumber(L,2);
    lua_pushboolean(L,1);
    return 1;
}
static int mod_setUserMaxSubs(lua_State * L){
    if(!lua_isuserdata(L,1))
        return 0;
    void * ptr=lua_touserdata(L,1);
    if(ptr==NULL)
        return 0;
    auto self=(serverMod*)ptr;
    self->userMaxSubs=luaL_checknumber(L,2);
    lua_pushboolean(L,1);
    return 1;
}
static int mod_setAdmin(lua_State * L){
    if(!lua_isuserdata(L,1))
        return 0;
    void * ptr=lua_touserdata(L,1);
    if(ptr==NULL)
        return 0;
    auto self=(serverMod*)ptr;
    
    auto u=luaL_checkstring(L,2);
    auto p=luaL_checkstring(L,3);
    
    self->adminPwd[u]=p;
    
    lua_pushboolean(L,1);
    return 1;
}
void serverMod::scriptInit(const char * path){
    L=luaL_newstate();
    luaL_openlibs(L);
    struct luaL_Reg funcs[]={
        {"addTerrainItemID" ,mod_addTerrainItemID},
        {"addBuildingTypeHP",mod_addBuildingTypeHP},
        {"setViewRange"     ,mod_setViewRange},
        {"setUserMaxSubs"   ,mod_setUserMaxSubs},
        {"addSubsConf"      ,mod_addSubsConf},
        {"setAdmin"         ,mod_setAdmin},
        {NULL,NULL}
    };
    luaL_newlib(L,funcs);
    lua_setglobal(L,"smoothly");
    
    if(luaL_loadfile(L,path) || lua_pcall(L, 0,0,0)){
        printf("error %s\n", lua_tostring(L,-1));
        return;
    }
    
    lua_getglobal(L,"modInit");
    lua_pushlightuserdata(L,this);
    if(lua_pcall(L, 1, 0, 0) != 0){
        printf("error %s\n", lua_tostring(L,-1));
        return;
    }
    
}
void serverMod::scriptDestroy(){
    lua_close(L);
}

}
