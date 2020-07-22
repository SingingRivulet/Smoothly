#include "body.h"

#include <QFile>
#include <QByteArray>

namespace smoothly{

void body::bodyItem::AIExec(){
    if(!config->haveAILoop)
        return;
    int aiFunc = config->AILoop;
    lua_settop(parent->L , 0);
    lua_rawgeti(parent->L,LUA_REGISTRYINDEX,aiFunc);
    if(lua_isfunction(parent->L,-1)){
        lua_createtable(parent->L,0,14);//创建主数组
        {
            lua_pushstring(parent->L,"uuid");
            lua_pushstring(parent->L, uuid.c_str());
            lua_settable(parent->L, -3);

            lua_pushstring(parent->L,"attackTargetFlag");
            lua_pushinteger(parent->L, behaviorStatus.attackTargetFlag);
            lua_settable(parent->L, -3);

            lua_pushstring(parent->L,"time");
            lua_pushinteger(parent->L, parent->timer->getTime());
            lua_settable(parent->L, -3);

            lua_pushstring(parent->L,"body");
            lua_pushlightuserdata(parent->L, this);
            lua_settable(parent->L, -3);

            lua_pushstring(parent->L,"haveAttackTarget");
            lua_pushboolean(parent->L, behaviorStatus.haveAttackTarget);
            lua_settable(parent->L, -3);

            lua_pushstring(parent->L,"haveFollow");
            lua_pushboolean(parent->L, behaviorStatus.haveFollow);
            lua_settable(parent->L, -3);

            lua_pushstring(parent->L,"hitBody");
            lua_pushboolean(parent->L, behaviorStatus.hitBody);
            lua_settable(parent->L, -3);

            lua_pushstring(parent->L,"hitBuilding");
            lua_pushboolean(parent->L, behaviorStatus.hitBuilding);
            lua_settable(parent->L, -3);

            lua_pushstring(parent->L,"hitTerrainItem");
            lua_pushboolean(parent->L, behaviorStatus.hitTerrainItem);
            lua_settable(parent->L, -3);

            lua_pushstring(parent->L,"onFloor");
            lua_pushboolean(parent->L, behaviorStatus.onFloor);
            lua_settable(parent->L, -3);

            lua_pushstring(parent->L,"pathFindingMode");
            lua_pushboolean(parent->L, behaviorStatus.pathFindingMode);
            lua_settable(parent->L, -3);

            lua_pushstring(parent->L,"attackTarget");
            lua_createtable(parent->L,3,0);
            {
                lua_pushnumber(parent->L, behaviorStatus.attackTarget.X);
                lua_seti(parent->L,-2,1);

                lua_pushnumber(parent->L, behaviorStatus.attackTarget.Y);
                lua_seti(parent->L,-2,2);

                lua_pushnumber(parent->L, behaviorStatus.attackTarget.Z);
                lua_seti(parent->L,-2,3);
            }
            lua_settable(parent->L, -3);

            lua_pushstring(parent->L,"pathFindingTarget");
            lua_createtable(parent->L,3,0);
            {
                lua_pushnumber(parent->L, behaviorStatus.pathFindingTarget.X);
                lua_seti(parent->L,-2,1);

                lua_pushnumber(parent->L, behaviorStatus.pathFindingTarget.Y);
                lua_seti(parent->L,-2,2);

                lua_pushnumber(parent->L, behaviorStatus.pathFindingTarget.Z);
                lua_seti(parent->L,-2,3);
            }
            lua_settable(parent->L, -3);

            lua_pushstring(parent->L,"pathFindingEnd");
            lua_createtable(parent->L,3,0);
            {
                lua_pushnumber(parent->L, behaviorStatus.pathFindingEnd.X);
                lua_seti(parent->L,-2,1);

                lua_pushnumber(parent->L, behaviorStatus.pathFindingEnd.Y);
                lua_seti(parent->L,-2,2);

                lua_pushnumber(parent->L, behaviorStatus.pathFindingEnd.Z);
                lua_seti(parent->L,-2,3);
            }
            lua_settable(parent->L, -3);

            lua_pushstring(parent->L,"followTarget");
            lua_createtable(parent->L,3,0);
            {
                lua_pushnumber(parent->L, behaviorStatus.followTarget.X);
                lua_seti(parent->L,-2,1);

                lua_pushnumber(parent->L, behaviorStatus.followTarget.Y);
                lua_seti(parent->L,-2,2);

                lua_pushnumber(parent->L, behaviorStatus.followTarget.Z);
                lua_seti(parent->L,-2,3);
            }
            lua_settable(parent->L, -3);

            lua_pushstring(parent->L,"position");
            lua_createtable(parent->L,3,0);
            {
                lua_pushnumber(parent->L, behaviorStatus.position.X);
                lua_seti(parent->L,-2,1);

                lua_pushnumber(parent->L, behaviorStatus.position.Y);
                lua_seti(parent->L,-2,2);

                lua_pushnumber(parent->L, behaviorStatus.position.Z);
                lua_seti(parent->L,-2,3);
            }
            lua_settable(parent->L, -3);

        }

        if (lua_pcall(parent->L, 1, 1, 0) != 0)
             printf("error running function : %s \n",lua_tostring(parent->L, -1));

    }
    lua_settop(parent->L , 0);
}

}

