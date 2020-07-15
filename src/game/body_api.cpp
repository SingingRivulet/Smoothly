#include "body.h"

namespace smoothly{

static int luafunc_pushCommond(lua_State * L){
    if(!lua_isuserdata(L,1))
        return 0;
    void * ptr=lua_touserdata(L,1);
    if(ptr==NULL)
        return 0;
    auto self=(body::bodyItem*)ptr;

    body::commond cmd;
    cmd.uuid = self->uuid;

    if(lua_istable(L,-1)){

        lua_pushstring(L, "str");
        lua_gettable(L, -2);
        if(lua_isstring(L,-1)){
            cmd.data_str = lua_tostring(L,-1);
        }
        lua_pop(L,1);

        lua_pushstring(L, "int");
        lua_gettable(L, -2);
        if(lua_isinteger(L,-1)){
            cmd.data_int = lua_tointeger(L,-1);
        }
        lua_pop(L,1);

        lua_pushstring(L, "vec");
        lua_gettable(L, -2);
        if(lua_istable(L,-1)){
            if(luaL_len(L,-1)>=3){

                lua_geti(L,-1,1);
                if(lua_isnumber(L,-1)){
                    cmd.data_vec.X = lua_tonumber(L,-1);
                }
                lua_pop(L,1);

                lua_geti(L,-1,2);
                if(lua_isnumber(L,-1)){
                    cmd.data_vec.Y = lua_tonumber(L,-1);
                }
                lua_pop(L,1);

                lua_geti(L,-1,3);
                if(lua_isnumber(L,-1)){
                    cmd.data_vec.Z = lua_tonumber(L,-1);
                }
                lua_pop(L,1);

            }
        }
        lua_pop(L,1);

        lua_pushstring(L, "cmd");
        lua_gettable(L, -2);
        if(lua_isstring(L,-1)){
            auto cmdStr = lua_tostring(L,-1);
            if(strcmp(cmdStr,"lookat")==0){
                cmd.cmd = body::CMD_SET_LOOKAT;
            }else if(strcmp(cmdStr,"rotaton")==0){
                cmd.cmd = body::CMD_SET_ROTATION;
            }else if(strcmp(cmdStr,"jump")==0){
                cmd.cmd = body::CMD_JUMP;
            }else if(strcmp(cmdStr,"interactive")==0){
                cmd.cmd = body::CMD_INTERACTIVE;
            }else if(strcmp(cmdStr,"fire_begin")==0){
                cmd.cmd = body::CMD_FIRE_BEGIN;
            }else if(strcmp(cmdStr,"fire_end")==0){
                cmd.cmd = body::CMD_FIRE_END;
            }else if(strcmp(cmdStr,"tool_reload_start")==0){
                cmd.cmd = body::CMD_TOOL_RELOAD_START;
            }else if(strcmp(cmdStr,"tool_reload_end")==0){
                cmd.cmd = body::CMD_TOOL_RELOAD_END;
            }else if(strcmp(cmdStr,"status_add")==0){
                cmd.cmd = body::CMD_STATUS_ADD;
            }else if(strcmp(cmdStr,"status_remove")==0){
                cmd.cmd = body::CMD_STATUS_REMOVE;
            }else if(strcmp(cmdStr,"status_set")==0){
                cmd.cmd = body::CMD_STATUS_SET;
            }
        }
        lua_pop(L,1);

        self->parent->pushCommond(cmd);
    }
    return 0;
}

static int luafunc_addStatus(lua_State * L){
    int a = luaL_checkinteger(L,1);
    int b = luaL_checkinteger(L,2);
    lua_pushinteger(L,a|b);
    return 1;
}

static int luafunc_session_get(lua_State * L){
    if(!lua_isuserdata(L,1))
        return 0;
    void * ptr=lua_touserdata(L,1);
    if(ptr==NULL)
        return 0;
    auto self=(body::bodyItem*)ptr;

    auto s = luaL_checkstring(L,2);

    auto it = self->behaviorStatus.session.find(s);
    if(it==self->behaviorStatus.session.end()){
        lua_pushnil(L);
        return 1;
    }else{
        lua_pushstring(L,it->second.c_str());
        return 1;
    }
}

static int luafunc_session_set(lua_State * L){
    if(!lua_isuserdata(L,1))
        return 0;
    void * ptr=lua_touserdata(L,1);
    if(ptr==NULL)
        return 0;
    auto self=(body::bodyItem*)ptr;

    auto k = luaL_checkstring(L,2);
    auto v = luaL_checkstring(L,3);

    self->behaviorStatus.session[k] = v;

    return 0;
}

static int luafunc_session_del(lua_State * L){
    if(!lua_isuserdata(L,1))
        return 0;
    void * ptr=lua_touserdata(L,1);
    if(ptr==NULL)
        return 0;
    auto self=(body::bodyItem*)ptr;

    auto s = luaL_checkstring(L,2);

    self->behaviorStatus.session.erase(s);

    return 0;
}

static int luafunc_navigation(lua_State * L){
    if(!lua_isuserdata(L,1))
        return 0;
    void * ptr=lua_touserdata(L,1);
    if(ptr==NULL)
        return 0;
    auto self=(body::bodyItem*)ptr;

    vec3 target;

    if(lua_istable(L,-1)){
        if(luaL_len(L,-1)>=3){

            lua_geti(L,-1,1);
            if(lua_isnumber(L,-1)){
                target.X = lua_tonumber(L,-1);
            }
            lua_pop(L,1);

            lua_geti(L,-1,2);
            if(lua_isnumber(L,-1)){
                target.Y = lua_tonumber(L,-1);
            }
            lua_pop(L,1);

            lua_geti(L,-1,3);
            if(lua_isnumber(L,-1)){
                target.Z = lua_tonumber(L,-1);
            }
            lua_pop(L,1);

            self->parent->navigation(self,target);
        }
    }
    return 0;
}

void body::loadBodyLuaAPI(lua_State * L){
    lua_createtable(L,0,5);
    {
        lua_pushstring(L,"pushCommond");
        lua_pushcfunction(L,luafunc_pushCommond);
        lua_settable(L,-3);

        lua_pushstring(L,"addStatus");
        lua_pushcfunction(L,luafunc_addStatus);
        lua_settable(L,-3);

        lua_pushstring(L,"navigation");
        lua_pushcfunction(L,luafunc_navigation);
        lua_settable(L,-3);

        lua_pushstring(L,"status");
        lua_createtable(L,0,23);
        {
#define addConst(name) \
    lua_pushstring(L,#name);\
    lua_pushinteger(L,name);\
    lua_settable(L,-3);

            addConst(BM_VANISH);
            addConst(BM_SQUAT);
            addConst(BM_SIT);
            addConst(BM_RIDE);
            addConst(BM_LIE);
            addConst(BM_LIEP);
            addConst(BM_HAND_LEFT);
            addConst(BM_HAND_RIGHT);
            addConst(BM_HAND_BOTH);
            addConst(BM_AIM);
            addConst(BM_THROW);
            addConst(BM_BUILD);
            addConst(BM_BUILDP);
            addConst(BM_OPERATE);
            addConst(BM_LIFT);
            addConst(BM_ACT_SHOT_L);
            addConst(BM_ACT_SHOT_R);
            addConst(BM_ACT_THROW);
            addConst(BM_ACT_CHOP);
            addConst(BM_WALK_F);
            addConst(BM_WALK_B);
            addConst(BM_WALK_L);
            addConst(BM_WALK_R);
            lua_pushstring(L,"BM_WALK");
            lua_pushinteger(L,BM_WALK_F|BM_WALK_B|BM_WALK_L|BM_WALK_R);
            lua_settable(L,-3);
        }
        lua_settable(L,-3);

        lua_pushstring(L,"session");
        lua_createtable(L,0,3);
        {
            lua_pushstring(L,"del");
            lua_pushcfunction(L,luafunc_session_del);
            lua_settable(L,-3);

            lua_pushstring(L,"get");
            lua_pushcfunction(L,luafunc_session_get);
            lua_settable(L,-3);

            lua_pushstring(L,"set");
            lua_pushcfunction(L,luafunc_session_set);
            lua_settable(L,-3);
        }
        lua_settable(L,-3);
    }

    lua_setglobal(L,"bodyAPI");
}

}
