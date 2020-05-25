#include "fire.h"

namespace smoothly{

void fire::attackBody(const std::string &uuid, fireConfig *conf, bodyInfo *body, float force){
    if(body->type==BODY_BODY_PART){
        auto b   = getBodyFromBodyPart(body);
        if(b){
            int dh = attackBody(uuid,conf,b,force);
            if(dh!=0)
                cmd_HPInc(b->uuid , dh);
        }
    }else
    if(body->type==BODY_BUILDING){
        auto b = (buildingBody*)body->ptr;
        if(b){
            int dh = attackBuilding(uuid,conf,b,force);
            if(dh!=0)
                cmd_damageBuilding(b->uuid , dh);
        }
    }else
    if(body->type==BODY_TERRAIN_ITEM){
        auto mid = (mapId*)body->ptr;
        if(attackTerrainItem(uuid,conf,mid,force))
            cmd_addRemovedItem(mid->x , mid->y , mid->id.id , mid->id.index);
    }
}

int fire::attackBody(const std::string &uuid, fireConfig *conf, bodyItem * b, float force){
    int res = 0;
    lua_getglobal(L,"attackBody");
    if(lua_isfunction(L,-1)){

        lua_pushlightuserdata(L,this);
        lua_pushstring (L, uuid.c_str());
        lua_pushinteger(L, conf->id);
        lua_pushinteger(L, b->id);
        lua_pushstring (L, b->uuid.c_str());
        lua_pushnumber (L, force);

        if (lua_pcall(L, 6, 1, 0) != 0)
             printf("error running function : %s \n",lua_tostring(L, -1));
        else{
            if(lua_isnumber(L,-1)){
                res = lua_tonumber(L,-1);
            }

        }

    }
    lua_settop(L , 0);
    return res;
}
bool fire::attackTerrainItem(const std::string &uuid, fireConfig *conf, mapId * m, float force){
    bool res = false;
    lua_getglobal(L,"attackTerrainItem");
    if(lua_isfunction(L,-1)){

        lua_pushlightuserdata(L,this);
        lua_pushstring (L, uuid.c_str());
        lua_pushinteger(L, conf->id);
        lua_pushinteger(L, m->x);
        lua_pushinteger(L, m->y);
        lua_pushinteger(L, m->id.id);
        lua_pushinteger(L, m->id.index);
        lua_pushnumber (L, force);

        if (lua_pcall(L, 8, 1, 0) != 0)
             printf("error running function : %s \n",lua_tostring(L, -1));
        else{
            if(lua_isboolean(L,-1)){
                res = lua_toboolean(L,-1);
            }

        }

    }
    lua_settop(L , 0);
    return res;
}

int fire::attackBuilding(const std::string &uuid, fireConfig *conf, buildingBody * b, float force){
    bool res = false;
    lua_getglobal(L,"attackBuilding");
    if(lua_isfunction(L,-1)){

        lua_pushlightuserdata(L,this);
        lua_pushstring (L, uuid.c_str());
        lua_pushinteger(L, conf->id);
        lua_pushstring (L, b->uuid.c_str());
        lua_pushinteger(L, b->config->id);
        lua_pushnumber (L, force);

        if (lua_pcall(L, 6, 1, 0) != 0)
             printf("error running function : %s \n",lua_tostring(L, -1));
        else{
            if(lua_isnumber(L,-1)){
                res = lua_tointeger(L,-1);
            }

        }

    }
    lua_settop(L , 0);
    return res;
}

}
