#include "serverMod.h"
namespace smoothly{
int serverMod::getHPByType(long type){
    auto it=buildingHPMap.find(type);
    if(it==buildingHPMap.end())
        return -1;
    else
        return it->second;
}
int serverMod::itemidNum(long id){
    auto it=buildingHPMap.find(id);
    if(it==buildingHPMap.end())
        return 0;
    else
        return it->second;
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
void serverMod::scriptInit(const char * path){
    L=luaL_newstate();
    struct luaL_Reg funcs[]={
        {"addTerrainItemID" ,mod_addTerrainItemID},
        {"addBuildingTypeHP",mod_addBuildingTypeHP},
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
