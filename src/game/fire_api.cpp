#include "fire.h"

namespace smoothly{

static int luafunc_fireTo(lua_State * L){
    if(!lua_isuserdata(L,1))
        return 0;
    void * ptr=lua_touserdata(L,1);
    if(ptr==NULL)
        return 0;
    auto self=(fire*)ptr;

    auto uuid = luaL_checkstring (L,2);
    auto id   = luaL_checkinteger(L,3);
    auto fx   = luaL_checknumber (L,4);
    auto fy   = luaL_checknumber (L,5);
    auto fz   = luaL_checknumber (L,6);
    auto tx   = luaL_checknumber (L,7);
    auto ty   = luaL_checknumber (L,8);
    auto tz   = luaL_checknumber (L,9);

    self->fireTo(uuid,id,vec3(fx,fy,fz),vec3(tx,ty,tz));
    return 0;
}

void fire::loadAPIs(){
    lua_pushcfunction(L,luafunc_fireTo);
    lua_setglobal(L,"fireTo");
}

}
