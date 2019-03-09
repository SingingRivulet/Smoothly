#include "mods.h"
namespace smoothly{
void mods::init(const char * path){
    defaultBuildingList[0]=1;
    defaultBuildingList[1]=2;
    defaultBuildingList[2]=3;
    defaultBuildingList[3]=-1;
    defaultBuildingList[4]=-1;
    defaultBuildingList[5]=-1;
    defaultBuildingList[6]=-1;
}
static int mod_addAutoGen(lua_State * L){
    if(!lua_isfunction(L,-1))
        return 0;
    int ref = luaL_ref(L,LUA_REGISTRYINDEX);
    if(!lua_isuserdata(L,1))
        return 0;
    void * ptr=lua_touserdata(L,1);
    if(ptr==NULL)
        return 0;
    auto self=(mods*)ptr;
    
    mods::autoMapGen mg;
    mg.luaFunc=ref;
    self->autoMapGenList.push_back(mg);
    
    lua_pushboolean(L,1);
    return 1;
}
void mods::scriptInit(const char * path){
    L=luaL_newstate();
    luaL_openlibs(L);
    struct luaL_Reg funcs[]={
        {"addAutoGen" ,mod_addAutoGen},
        {NULL,NULL}
    };
    luaL_newlib(L,funcs);
    lua_setglobal(L,"smoothly");
    
    if(luaL_loadfile(L,path) || lua_pcall(L, 0,0,0)){
        printf("error %s\n", lua_tostring(L,-1));
        return;
    }
    
    lua_getglobal(L,"gameInit");
    lua_pushlightuserdata(L,this);
    if(lua_pcall(L, 1, 0, 0) != 0){
        printf("error %s\n", lua_tostring(L,-1));
        return;
    }
}
void mods::scriptDestroy(){
    lua_close(L);
}
void mods::getGenList(
    int x,int y,
    int tem,int hu,
    float h,  
    std::map<long,float> & pl
){
    for(auto it:autoMapGenList){
        it.getGenList(L,x,y,tem,hu,h,pl);
    }
}
void mods::autoMapGen::getGenList(
    lua_State * L,
    int x,int y,
    int tem,int hu,
    float h,  
    std::map<long,float> & pl
){
    lua_settop(L,0);
    lua_rawgeti(L,LUA_REGISTRYINDEX,luaFunc);
    if(!lua_isfunction(L,-1)){
        lua_pop(L,1);
        return;
    }
    lua_pushinteger(L,x);
    lua_pushinteger(L,y);
    lua_pushinteger(L,tem);
    lua_pushinteger(L,hu);
    lua_pushnumber(L,h);
    if(lua_pcall(L, 5, 1, 0) != 0){
        printf("error %s\n", lua_tostring(L,-1));
        return;
    }
    if(lua_istable(L,-1)){
        int len=luaL_len(L,-1);
        for(int i=1;i<=len;i++){
            lua_rawgeti(L,-1,i);
            if(lua_istable(L,-1)){
                
                lua_rawgeti(L,-1,1);
                int id=lua_tointeger(L,-1);
                lua_pop(L,1);
                
                lua_rawgeti(L,-1,2);
                float prob=lua_tonumber(L,-1);
                lua_pop(L,1);
                
                pl[id]=prob;
            }
            lua_pop(L,1);
        }
    }
}
void mods::loadMesh(){
    building * b;
    auto creator=scene->getGeometryCreator();
    
    b           = new building;
    b->mesh     = creator->createCubeMesh(irr::core::vector3df(5.0f , 0.1f , 5.0f));
    b->BB       = b->mesh->getBoundingBox();
    b->bodyMesh = createBtMesh(b->mesh);
    b->bodyShape= createShape(b->bodyMesh);
    buildings[1]= b;
    
    b       = new building;
    b->mesh = creator->createCubeMesh(irr::core::vector3df(5.0f , 5.0f , 0.1f));
    b->BB   = b->mesh->getBoundingBox();
    b->bodyMesh = createBtMesh(b->mesh);
    b->bodyShape= createShape(b->bodyMesh);
    buildings[2]=b;
    
    b       = new building;
    b->mesh = creator->createCubeMesh(irr::core::vector3df(1.0f , 5.0f , 1.0f));
    b->BB   = b->mesh->getBoundingBox();
    b->bodyMesh = createBtMesh(b->mesh);
    b->bodyShape= createShape(b->bodyMesh);
    buildings[3]=b;
}
void mods::destroy(){
    for(auto it:items){
        it.second->mesh->drop();
        delete it.second;
    }
    items.clear();
    for(auto it:buildings){
        it.second->mesh->drop();
        delete it.second;
        if(it.second->bodyMesh) delete it.second->bodyMesh;
        if(it.second->bodyShape)delete it.second->bodyShape;
    }
    buildings.clear();
}
void mods::mapGenerator::autoGen(int x,int y,int tem,int hu,float h,mods * mod){
    std::map<long,float> pl;
    predictableRand randg;
    mod->getGenList(x,y,tem,hu,h,pl);
    for(auto it:pl){
        
        auto tinit=mod->terrainItemNum.find(it.first);
        if(tinit==mod->terrainItemNum.end())
            continue;
        
        int num=tinit->second;
        
        if(num<=0)
            continue;
        
        randg.setSeed((x+10)*(y+20)*(hu+30)*(tem+40));
        
        int delta=it.second*1000;
        for(int i=0;i<num;i++){
            int pr=(randg.rand())%1000;
            if(pr>delta){
                float mx=(randg.frand()+x)*32;
                float my=(randg.frand()+y)*32;
                float mr=randg.frand()*3.14159*2;
                this->add(
                    it.first,
                    irr::core::vector3df(mx,getRealHight(mx,my),my),
                    irr::core::vector3df(0,mr,0),
                    irr::core::vector3df(1,1,1)
                );
            }
        }
        
    }
}

}