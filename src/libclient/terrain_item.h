//地形物体生成
#ifndef SMOOTHLY_WORLD_TERRAIN_ITEM
#define SMOOTHLY_WORLD_TERRAIN_ITEM
#include <list>
#include <lua.hpp>
namespace smoothly{
namespace world{
namespace terrain{
////////////////
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
class terrain_item{
    public:
        struct genProb{
            int id,num;
            float prob;
        };
        terrain_item(){
            L = luaL_newstate();
            luaL_openlibs(L);
            if (luaL_loadfile(L, "script/terrainItem.lua") || lua_pcall(L, 0, 0, 0))
                printf("[error]terrainItem:%s\n" , lua_tostring(L, -1));
        }
        ~terrain_item(){
            lua_close(L);
        }
    private:
        lua_State * L;
    public:
        void getGenList(int x,int y,int tem,int hu,float h, std::list<genProb> & pl){
            lua_settop(L,0);
            lua_getglobal(L, "main");
            if(!lua_isfunction(L,-1)){
                lua_pop(L,1);
                printf("[error]getGenList:'main' no found\n");
                return;
            }
            lua_pushinteger(L,x);
            lua_pushinteger(L,y);
            lua_pushinteger(L,tem);
            lua_pushinteger(L,hu);
            lua_pushnumber(L,h);
            if(lua_pcall(L, 5, 1, 0) != 0){
                printf("[error]getGenList:%s\n", lua_tostring(L,-1));
                lua_settop(L,0);
                return;
            }
            if(lua_istable(L,-1)){
                int len=luaL_len(L,-1);
                for(int i=1;i<=len;i++){
                    lua_rawgeti(L,-1,i);
                    if(lua_istable(L,-1)){
                        
                        genProb pb;
                        
                        lua_rawgeti(L,-1,1);
                        pb.id=lua_tointeger(L,-1);
                        lua_pop(L,1);
                        
                        lua_rawgeti(L,-1,2);
                        pb.prob=lua_tonumber(L,-1);
                        lua_pop(L,1);
                        
                        lua_rawgeti(L,-1,3);
                        pb.num=lua_tointeger(L,-1);
                        lua_pop(L,1);
                        
                        pl.push_back(pb);
                    }
                    lua_pop(L,1);
                }
            }
            lua_settop(L,0);
        }
};
////////////////
}//////terrain
}//////world
}//////smoothly
#endif
