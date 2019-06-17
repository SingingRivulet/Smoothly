#include "mods.h"
#include "subsapi.h"
#include <string.h>
namespace smoothly{

void mods::init(){
    defaultBuildingList[0]=1;
    defaultBuildingList[1]=2;
    defaultBuildingList[2]=3;
    defaultBuildingList[3]=-1;
    defaultBuildingList[4]=-1;
    defaultBuildingList[5]=-1;
    defaultBuildingList[6]=-1;
}

static int mod_addBuildingMesh(lua_State * L){
    if(!lua_isuserdata(L,1))
        return 0;
    void * ptr=lua_touserdata(L,1);
    if(ptr==NULL)
        return 0;
    auto self=(mods*)ptr;
    
    if(self->scene==NULL){
        lua_pushstring(L,"Irrlicht scene has not been initializated!");
        return 1;
    }
    
    int id=luaL_checkinteger(L,2);
    
    if(self->buildings.find(id)!=self->buildings.end()){
        lua_pushstring(L,"ID has been existed!");
        return 1;
    }
    
    std::string scmpath,texture,shape;
    bool havebody=false;
    bool useAlpha=false;
    
    float friction=0.5;
    float restitution=0.1;
    
    if(!lua_istable(L,-1)){
        lua_pushstring(L,"smoothly.addBuildingMesh(handler,itemId,{mesh=path,havebody=havebody})");
        return 1;
    }
    
    lua_pushstring(L,"friction");
    lua_gettable(L,-2);
    if(lua_isnumber(L,-1)){
        friction=lua_tonumber(L,-1);
    }
    lua_pop(L,1);
    
    lua_pushstring(L,"restitution");
    lua_gettable(L,-2);
    if(lua_isnumber(L,-1)){
        restitution=lua_tonumber(L,-1);
    }
    lua_pop(L,1);
    
    lua_pushstring(L,"texture");
    lua_gettable(L,-2);
    if(lua_isstring(L,-1)){
        texture=lua_tostring(L,-1);
        lua_pop(L,1);
    }else{
        lua_pop(L,1);
        lua_pushstring(L,"Texture doesn't exist!");
        return 1;
    }
    
    lua_pushstring(L,"shape");
    lua_gettable(L,-2);
    if(lua_isstring(L,-1)){
        shape=lua_tostring(L,-1);
        lua_pop(L,1);
    }else{
        lua_pop(L,1);
        lua_pushstring(L,"Shape doesn't exist!");
        return 1;
    }
    
    lua_pushstring(L,"mesh");
    lua_gettable(L,-2);
    if(lua_isstring(L,-1)){
        scmpath=lua_tostring(L,-1);
        lua_pop(L,1);
    }else{
        lua_pop(L,1);
        lua_pushstring(L,"Mesh doesn't exist!");
        return 1;
    }
    
    auto mesh=self->scene->getMesh(scmpath.c_str());
    if(mesh==NULL){
        lua_pushstring(L,"Load mesh fail!");
        return 1;
    }
    
    lua_pushstring(L,"useAlpha");
    lua_gettable(L,-2);
    if(lua_isboolean(L,-1)){
        useAlpha=lua_toboolean(L,-1);
    }
    lua_pop(L,1);
    
    auto b  = new mods::building(
        mesh,
        self->scene->getVideoDriver()->getTexture(texture.c_str()),
        shape,
        useAlpha,
        friction,
        restitution
    );
    
    lua_pushstring(L,"onAimAtBuilding");
    lua_gettable(L,-2);
    if(lua_isfunction(L,-1)){
        b->onAimAtBuilding=luaL_ref(L,LUA_REGISTRYINDEX);
        b->haveOnAimAtBuilding=true;
    }
    lua_pop(L,1);
    
    self->buildings[id]=b;
    
    lua_pushstring(L,"OK");
    return 1;
}

static int mod_addTerrainMesh(lua_State * L){
    if(!lua_isuserdata(L,1))
        return 0;
    void * ptr=lua_touserdata(L,1);
    if(ptr==NULL)
        return 0;
    auto self=(mods*)ptr;
    
    if(self->scene==NULL){
        lua_pushstring(L,"Irrlicht scene has not been initializated!");
        return 1;
    }
    
    int id=luaL_checkinteger(L,2);
    
    if(self->items.find(id)!=self->items.end() || self->terrainItemNum.find(id)!=self->terrainItemNum.end()){
        lua_pushstring(L,"ID has been existed!");
        return 1;
    }
    
    std::string scmpath;
    bool havebody=false;
    
    if(!lua_istable(L,-1)){
        lua_pushstring(L,"smoothly.addTerrainMesh(handler,itemId,{mesh=path,havebody=havebody,maxnum=maxnum})");
        return 1;
    }
    
    lua_pushstring(L,"mesh");
    lua_gettable(L,-2);
    if(lua_isstring(L,-1)){
        scmpath=lua_tostring(L,-1);
        lua_pop(L,1);
    }else{
        lua_pop(L,1);
        lua_pushstring(L,"Mesh doesn't exist!");
        return 1;
    }
    
    lua_pushstring(L,"havebody");
    lua_gettable(L,-2);
    if(lua_isboolean(L,-1)){
        havebody=lua_toboolean(L,-1);
    }
    lua_pop(L,1);
    
    auto mesh=self->scene->getMesh(scmpath.c_str());
    if(mesh==NULL){
        lua_pushstring(L,"Load mesh fail!");
        return 1;
    }
    
    auto b=new mods::itemConfig;
    b->mesh=mesh;
    
    b->haveBody=false;
    if(havebody){
        lua_pushstring(L,"shape");
        lua_gettable(L,-2);
        if(lua_isstring(L,-1)){
            auto subsConf=lua_tostring(L,-1);
            b->shape.init(subsConf);
            b->haveBody=true;
        }
        lua_pop(L,1);
    }else{
        
    }
    
    b->BB=mesh->getBoundingBox();
    
    int maxnum=5;
    
    lua_pushstring(L,"maxnum");
    lua_gettable(L,-2);
    if(lua_isinteger(L,-1)){
        maxnum=lua_tointeger(L,-1);
    }
    lua_pop(L,1);
    
    lua_pushstring(L,"lodMeshV2");
    lua_gettable(L,-2);
    if(lua_isstring(L,-1)){
        auto lmsh=self->scene->getMesh(lua_tostring(L,-1));
        if(lmsh)
            b->meshv2=lmsh;
    }
    lua_pop(L,1);
    
    lua_pushstring(L,"lodMeshV3");
    lua_gettable(L,-2);
    if(lua_isstring(L,-1)){
        auto lmsh=self->scene->getMesh(lua_tostring(L,-1));
        if(lmsh)
            b->meshv3=lmsh;
    }
    lua_pop(L,1);
    
    lua_pushstring(L,"lodMeshV4");
    lua_gettable(L,-2);
    if(lua_isstring(L,-1)){
        auto lmsh=self->scene->getMesh(lua_tostring(L,-1));
        if(lmsh)
            b->meshv4=lmsh;
    }
    lua_pop(L,1);
    
    
    lua_pushstring(L,"texture");
    lua_gettable(L,-2);
    if(lua_isstring(L,-1)){
        auto texture=self->scene->getVideoDriver()->getTexture(lua_tostring(L,-1));
        if(texture)
            b->texture=texture;
    }
    lua_pop(L,1);
    
    lua_pushstring(L,"useAlpha");
    lua_gettable(L,-2);
    if(lua_isboolean(L,-1)){
        b->useAlpha=lua_toboolean(L,-1);
    }
    lua_pop(L,1);
    
    lua_pushstring(L,"deltaPosition");
    lua_gettable(L,-2);
    if(lua_istable(L,-1)){
        
        lua_rawgeti(L,-1,1);
        if(lua_isnumber(L,-1)){
            b->deltaPosition.X=lua_tonumber(L,-1);
        }
        lua_pop(L,1);
        
        lua_rawgeti(L,-1,2);
        if(lua_isnumber(L,-1)){
            b->deltaPosition.Y=lua_tonumber(L,-1);
        }
        lua_pop(L,1);
        
        lua_rawgeti(L,-1,3);
        if(lua_isnumber(L,-1)){
            b->deltaPosition.Z=lua_tonumber(L,-1);
        }
        lua_pop(L,1);
        
    }
    lua_pop(L,1);
    
    lua_pushstring(L,"scale");
    lua_gettable(L,-2);
    if(lua_istable(L,-1)){
        
        lua_rawgeti(L,-1,1);
        if(lua_isnumber(L,-1)){
            b->scale.X=lua_tonumber(L,-1);
        }
        lua_pop(L,1);
        
        lua_rawgeti(L,-1,2);
        if(lua_isnumber(L,-1)){
            b->scale.Y=lua_tonumber(L,-1);
        }
        lua_pop(L,1);
        
        lua_rawgeti(L,-1,3);
        if(lua_isnumber(L,-1)){
            b->scale.Z=lua_tonumber(L,-1);
        }
        lua_pop(L,1);
        
    }
    lua_pop(L,1);
    
    self->items[id]=b;
    self->terrainItemNum[id]=maxnum;
    
    lua_pushstring(L,"OK");
    return 1;
}


static int mod_addSubstance(lua_State * L){
    if(!lua_isuserdata(L,1))
        return 0;
    void * ptr=lua_touserdata(L,1);
    if(ptr==NULL)
        return 0;
    auto self=(mods*)ptr;
    
    if(self->scene==NULL){
        lua_pushstring(L,"Irrlicht scene has not been initializated!");
        return 1;
    }
    
    int id=luaL_checkinteger(L,2);
    
    if(self->subsConfs.find(id)!=self->subsConfs.end()){
        lua_pushstring(L,"ID has been existed!");
        return 1;
    }
    
    std::string scmpath;
    std::string shapeConf;
    
    float friction=0.5;
    float restitution=0.1;
    
    if(!lua_istable(L,-1)){
        lua_pushstring(L,"The third argument must be table!");
        return 1;
    }
    
    lua_pushstring(L,"shape");
    lua_gettable(L,-2);
    if(lua_isstring(L,-1)){
        shapeConf=lua_tostring(L,-1);
    }
    lua_pop(L,1);
    
    lua_pushstring(L,"friction");
    lua_gettable(L,-2);
    if(lua_isnumber(L,-1)){
        friction=lua_tonumber(L,-1);
    }
    lua_pop(L,1);
    
    lua_pushstring(L,"restitution");
    lua_gettable(L,-2);
    if(lua_isnumber(L,-1)){
        restitution=lua_tonumber(L,-1);
    }
    lua_pop(L,1);
    
    lua_pushstring(L,"mesh");
    lua_gettable(L,-2);
    if(lua_isstring(L,-1)){
        scmpath=lua_tostring(L,-1);
        lua_pop(L,1);
    }else{
        lua_pop(L,1);
        lua_pushstring(L,"Mesh doesn't exist!");
        return 1;
    }
    
    auto mesh=self->scene->getMesh(scmpath.c_str());
    if(mesh==NULL){
        lua_pushstring(L,"Load mesh fail!");
        return 1;
    }
    
    auto b=new mods::subsConf;
    b->mesh=mesh;
    
    b->shape.init(shapeConf);
    b->bodyShape=b->shape.compound;
    
    lua_pushstring(L,"type");
    lua_gettable(L,-2);
    if(lua_isstring(L,-1)){
        auto p=lua_tostring(L,-1);
        if(strcmp(p,"lasting")==0){
            b->type=mods::SUBS_LASTING;
        }else
        if(strcmp(p,"brief")==0){
            b->type=mods::SUBS_BRIEF;
        }
    }
    lua_pop(L,1);
    
    lua_pushstring(L,"bodyType");
    lua_gettable(L,-2);
    if(lua_isstring(L,-1)){
        b->bodyType=lua_tostring(L,-1);
    }
    lua_pop(L,1);
    
    lua_pushstring(L,"animationType");
    lua_gettable(L,-2);
    if(lua_isstring(L,-1)){
        b->animationType=lua_tostring(L,-1);
    }
    lua_pop(L,1);
    
    lua_pushstring(L,"characterWidth");
    lua_gettable(L,-2);
    if(lua_isnumber(L,-1)){
        b->characterWidth=lua_tonumber(L,-1);
    }
    lua_pop(L,1);
    
    lua_pushstring(L,"characterHeight");
    lua_gettable(L,-2);
    if(lua_isnumber(L,-1)){
        b->characterHeight=lua_tonumber(L,-1);
    }
    lua_pop(L,1);
    
    lua_pushstring(L,"texture");
    lua_gettable(L,-2);
    if(lua_isstring(L,-1)){
        auto texture=self->scene->getVideoDriver()->getTexture(lua_tostring(L,-1));
        if(texture)
            b->texture=texture;
    }
    lua_pop(L,1);
    
    lua_pushstring(L,"useAlpha");
    lua_gettable(L,-2);
    if(lua_isboolean(L,-1)){
        b->useAlpha=lua_toboolean(L,-1);
    }
    lua_pop(L,1);
    
    //callbacks
    //subs
    lua_pushstring(L,"hitSubsCallback");
    lua_gettable(L,-2);
    if(lua_isfunction(L,-1)){
        b->hitSubsCallback=luaL_ref(L,LUA_REGISTRYINDEX);
        b->haveHitSubsCallback=true;
    }
    lua_pop(L,1);
    
    lua_pushstring(L,"hitSubsCallbackOnlyForMe");
    lua_gettable(L,-2);
    if(lua_isboolean(L,-1)){
        b->hitSubsCallbackOnlyForMe=lua_toboolean(L,-1);
    }
    lua_pop(L,1);
    
    //building
    lua_pushstring(L,"hitBuildingCallback");
    lua_gettable(L,-2);
    if(lua_isfunction(L,-1)){
        b->hitBuildingCallback=luaL_ref(L,LUA_REGISTRYINDEX);
        b->haveHitBuildingCallback=true;
    }
    lua_pop(L,1);
    
    lua_pushstring(L,"hitBuildingCallbackOnlyForMe");
    lua_gettable(L,-2);
    if(lua_isboolean(L,-1)){
        b->hitBuildingCallbackOnlyForMe=lua_toboolean(L,-1);
    }
    lua_pop(L,1);
    
    //terrain item
    lua_pushstring(L,"hitTerrainItemCallback");
    lua_gettable(L,-2);
    if(lua_isfunction(L,-1)){
        b->hitTerrainItemCallback=luaL_ref(L,LUA_REGISTRYINDEX);
        b->haveHitTerrainItemCallback=true;
    }
    lua_pop(L,1);
    
    lua_pushstring(L,"hitTerrainItemCallbackOnlyForMe");
    lua_gettable(L,-2);
    if(lua_isboolean(L,-1)){
        b->hitTerrainItemCallbackOnlyForMe=lua_toboolean(L,-1);
    }
    lua_pop(L,1);
    
    //terrain
    lua_pushstring(L,"hitTerrainCallback");
    lua_gettable(L,-2);
    if(lua_isfunction(L,-1)){
        b->hitTerrainCallback=luaL_ref(L,LUA_REGISTRYINDEX);
        b->haveHitTerrainCallback=true;
    }
    lua_pop(L,1);
    
    lua_pushstring(L,"hitTerrainCallbackOnlyForMe");
    lua_gettable(L,-2);
    if(lua_isboolean(L,-1)){
        b->hitTerrainCallbackOnlyForMe=lua_toboolean(L,-1);
    }
    lua_pop(L,1);
    
    //set active
    lua_pushstring(L,"active");
    lua_gettable(L,-2);
    if(lua_istable(L,-1)){
        
        lua_pushstring(L,"defaultSpeed");
        lua_gettable(L,-2);
        if(lua_isnumber(L,-1)){
            b->defaultSpeed=lua_tonumber(L,-1);
        }
        lua_pop(L,1);
        
        lua_pushstring(L,"defaultLiftForce");
        lua_gettable(L,-2);
        if(lua_isnumber(L,-1)){
            b->defaultLiftForce=lua_tonumber(L,-1);
        }
        lua_pop(L,1);
        
        lua_pushstring(L,"defaultPushForce");
        lua_gettable(L,-2);
        if(lua_isnumber(L,-1)){
            b->defaultPushForce=lua_tonumber(L,-1);
        }
        lua_pop(L,1);
        
        lua_pushstring(L,"defaultJumpImp");
        lua_gettable(L,-2);
        if(lua_isnumber(L,-1)){
            b->defaultJumpImp=lua_tonumber(L,-1);
        }
        lua_pop(L,1);
        
        lua_pushstring(L,"deltaCamera");
        lua_gettable(L,-2);
        if(lua_isnumber(L,-1)){
            b->deltaCamera=lua_tonumber(L,-1);
        }
        lua_pop(L,1);
        
        lua_pushstring(L,"noFallDown");
        lua_gettable(L,-2);
        if(lua_isboolean(L,-1)){
            b->noFallDown=lua_toboolean(L,-1);
        }
        lua_pop(L,1);
        
        lua_pushstring(L,"walkInSky");
        lua_gettable(L,-2);
        if(lua_isboolean(L,-1)){
            b->walkInSky=lua_toboolean(L,-1);
        }
        lua_pop(L,1);
        
        lua_pushstring(L,"jumpInSky");
        lua_gettable(L,-2);
        if(lua_isboolean(L,-1)){
            b->jumpInSky=lua_toboolean(L,-1);
        }
        lua_pop(L,1);
    }
    lua_pop(L,1);
    
    lua_pushstring(L,"boneAnimation");
    lua_gettable(L,-2);
    if(lua_istable(L,-1)){
        int len=luaL_len(L,-1);
        for(int i=1;i<=len;i++){
            lua_rawgeti(L,-1,i);
            if(lua_istable(L,-1)){
                
                mods::boneAnimateGroup ag;
                mods::boneAnimate am;
                
                #define getAmArg \
                    lua_rawgeti(L,-1,1); \
                    am.id=lua_tointeger(L,-1); \
                    lua_pop(L,1); \
                    lua_rawgeti(L,-1,2); \
                    am.speed=lua_tointeger(L,-1); \
                    lua_pop(L,1); \
                    lua_rawgeti(L,-1,3); \
                    am.start=lua_tointeger(L,-1); \
                    lua_pop(L,1); \
                    lua_rawgeti(L,-1,4); \
                    am.end=lua_tointeger(L,-1); \
                    lua_pop(L,1); \
                    lua_rawgeti(L,-1,5); 
                    am.loop=lua_toboolean(L,-1); \
                    lua_pop(L,1);
                            
                
                lua_rawgeti(L,-1,1);
                int id=lua_tointeger(L,-1);
                lua_pop(L,1);
                
                lua_pushstring(L,"body");
                lua_gettable(L,-2);
                if(lua_istable(L,-1)){
                    getAmArg;
                    ag.body=am;
                }
                lua_pop(L,1);
                
                lua_pushstring(L,"pair");
                lua_gettable(L,-2);
                if(lua_istable(L,-1)){
                    int len=luaL_len(L,-1);
                    for(int i=1;i<=len;i++){
                        lua_rawgeti(L,-1,i);
                        if(lua_istable(L,-1)){
                            
                            getAmArg;
                            
                            ag.pair.push_back(am);
                        }
                        lua_pop(L,1);
                    }
                }
                lua_pop(L,1);
                
                lua_pushstring(L,"item");
                lua_gettable(L,-2);
                if(lua_istable(L,-1)){
                    int len=luaL_len(L,-1);
                    for(int i=1;i<=len;i++){
                        lua_rawgeti(L,-1,i);
                        if(lua_istable(L,-1)){
                            
                            getAmArg;
                            
                            ag.item.push_back(am);
                        }
                        lua_pop(L,1);
                    }
                }
                lua_pop(L,1);
                
                b->boneAnimation[id]=ag;
                
                #undef getAmArg
            }
            lua_pop(L,1);
        }
    }
    lua_pop(L,1);
    
    lua_pushstring(L,"boneMapping");
    lua_gettable(L,-2);
    if(lua_istable(L,-1)){
        int len=luaL_len(L,-1);
        for(int i=1;i<=len;i++){
            lua_rawgeti(L,-1,i);
            if(lua_istable(L,-1)){
                
                lua_rawgeti(L,-1,1);
                int id=lua_tointeger(L,-1);
                lua_pop(L,1);
                
                lua_rawgeti(L,-1,2);
                bool onbody=lua_toboolean(L,-1);
                lua_pop(L,1);
                
                lua_rawgeti(L,-1,3);
                int mapping=lua_tointeger(L,-1);
                lua_pop(L,1);
                
                b->boneMapping[id]=std::pair<bool,int>(onbody,mapping);
            }
            lua_pop(L,1);
        }
    }
    lua_pop(L,1);
    
    b->friction = friction;
    b->restitution=restitution;
    
    self->subsConfs[id]=b;
    
    lua_pushstring(L,"OK");
    return 1;
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

static int mod_setWindow(lua_State * L){
    if(!lua_isfunction(L,-1))
        return 0;
    int ref = luaL_ref(L,LUA_REGISTRYINDEX);
    if(!lua_isuserdata(L,1))
        return 0;
    void * ptr=lua_touserdata(L,1);
    if(ptr==NULL)
        return 0;
    auto self=(mods*)ptr;
    
    int width=luaL_checkinteger(L,2);
    int height=luaL_checkinteger(L,3);
    
    self->windowWidth =width;
    self->windowHeight=height;
    
    lua_pushboolean(L,1);
    return 1;
}


static int mod_setBooster(lua_State * L){
    if(!lua_isfunction(L,-1))
        return 0;
    int ref = luaL_ref(L,LUA_REGISTRYINDEX);
    if(!lua_isuserdata(L,1))
        return 0;
    void * ptr=lua_touserdata(L,1);
    if(ptr==NULL)
        return 0;
    auto self=(mods*)ptr;
    
    self->booster=luaL_checknumber(L,2);
    
    lua_pushboolean(L,1);
    return 1;
}

static int mod_setAnimation(lua_State * L){
    if(!lua_isfunction(L,-1))
        return 0;
    int ref = luaL_ref(L,LUA_REGISTRYINDEX);
    if(!lua_isuserdata(L,1))
        return 0;
    void * ptr=lua_touserdata(L,1);
    if(ptr==NULL)
        return 0;
    auto self=(mods*)ptr;
    
    int id=luaL_checkinteger(L,2);
    
    auto it=self->animations.find(id);
    
    if(it!=self->animations.end()){
        lua_pushstring(L,"animation exists");
        return 1;
    }
    
    if(!lua_istable(L,-1))
        return 0;
    
    mods::animationConf * ac=NULL;
    irr::scene::IAnimatedMesh * mesh=NULL;
    irr::video::ITexture * texture=NULL;
    
    lua_pushstring(L,"mesh");
    lua_gettable(L,-2);
    if(lua_isstring(L,-1)){
        mesh = self->scene->getMesh(lua_tostring(L,-1));
    }
    lua_pop(L,1);
    
    if(!mesh){
        lua_pushstring(L,"load mesh fail");
        return 1;
    }
    
    lua_pushstring(L,"texture");
    lua_gettable(L,-2);
    if(lua_isstring(L,-1)){
        texture = self->scene->getVideoDriver()->getTexture(lua_tostring(L,-1));
    }
    lua_pop(L,1);
    
    ac=new mods::animationConf;
    ac->mesh=mesh;
    ac->texture=texture;
    
    lua_pushstring(L,"useAlpha");
    lua_gettable(L,-2);
    if(lua_isboolean(L,-1)){
        ac->useAlpha=lua_toboolean(L,-1);
    }
    lua_pop(L,1);
    
    self->animations[id]=ac;
    
    lua_pushstring(L,"ok");
    return 1;
}

int getCharAnimationId(
    int foot,
    int hand,
    int range,
    int move
){
    const static int amMap[7][4]={
        {0  ,   1  ,  2  ,  3},
        {4  ,   5  ,  6  ,  7},
        {8  ,   9  ,  10 , 11},
        {12 ,   13 ,  14 , 15},
        {16 ,   17 ,  18 , 19},
        {20 ,   21 ,  22 , 23},
        {24 ,   25 ,  26 , 27}
    };
    int c=foot;
    int r=0;
    
    if(hand==0){
        r=0;
    }else
    if(hand==1 && range==1){
        r=1;
    }else
    if(hand==2 && range==1){
        r=2;
    }else
    if(hand==3 && range==1){
        r=5;
    }else
    if(hand==1 && range==2){
        r=3;
    }else
    if(hand==2 && range==2){
        r=4;
    }else
    if(hand==3 && range==2){
        r=6;
    }
    
    return amMap[r][c]*8+move;
}

static int mod_getCharAnimationId(lua_State * L){
    //           站     蹲     趴    飞/跳
    //0          0      1     2     3
    //1单手近战    4      5     6     7
    //2双手近战    8      9     10   11
    //3单手远战    12     13    14   15
    //4双手远战    16     17    18   19
    //5双持近战    20     21    22   23
    //6双持远战    24     25    26   27
    //数值乘8加0为站立，加1为行走，加2为奔跑，加3倒走，加4倒跑
    //加100为乘坐，加200骑跨
    
    auto foot =luaL_checkstring(L,1);//站 蹲 趴 飞/跳
    auto hand =luaL_checkstring(L,2);//空手 单手 双手 双持
    auto range=luaL_checkstring(L,3);//无攻击 近战 远战
    auto move =luaL_checkstring(L,4);//站立 行走 奔跑
    
    int ifoot=0,ihand=0,irange=0,imove=0;
    
    if(strcmp(foot,"stand")==0){
        ifoot=0;
    }else
    if(strcmp(foot,"squat")==0){
        ifoot=1;
    }else
    if(strcmp(foot,"prostrate")==0){
        ifoot=2;
    }else
    if(strcmp(foot,"fly")==0){
        ifoot=3;
    }
    
    
    
    if(strcmp(hand,"empty")==0){
        ihand=0;
    }else
    if(strcmp(hand,"single")==0){
        ihand=1;
    }else
    if(strcmp(hand,"both")==0){
        ihand=2;
    }else
    if(strcmp(hand,"double")==0){
        ihand=3;
    }
    
    
    
    if(strcmp(range,"none")==0){
        irange=0;
    }else
    if(strcmp(range,"close")==0){
        irange=1;
    }else
    if(strcmp(range,"ranged")==0){
        irange=2;
    }
    
    
    if(strcmp(move,"stand")==0){
        imove=0;
    }else
    if(strcmp(move,"walk")==0){
        imove=1;
    }else
    if(strcmp(move,"run")==0){
        imove=2;
    }
    
    return getCharAnimationId(ifoot,ihand,irange,imove);
}

void mods::scriptInit(const char * path){
    L=luaL_newstate();
    luaL_openlibs(L);
    subsAPI::openlibs(L);
    struct luaL_Reg funcs[]={
        {"addAutoGen"           ,mod_addAutoGen},
        {"addTerrainMesh"       ,mod_addTerrainMesh},
        {"addBuildingMesh"      ,mod_addBuildingMesh},
        {"addSubstance"         ,mod_addSubstance},
        {"setWindow"            ,mod_setWindow},
        {"setBooster"           ,mod_setBooster},
        {"setAnimation"         ,mod_setAnimation},
        {"getCharAnimationId"   ,mod_getCharAnimationId},
        {NULL,NULL}
    };
    luaL_newlib(L,funcs);
    lua_setglobal(L,"smoothly");
    
    if(luaL_loadfile(L,path) || lua_pcall(L, 0,0,0)){
        printf("[scriptInit:loadfile]\e[0;31merror:\e[0m %s\n", lua_tostring(L,-1));
        return;
    }
    
    lua_getglobal(L,"gameInit");
    lua_pushlightuserdata(L,this);
    if(lua_pcall(L, 1, 0, 0) != 0){
        printf("[scriptInit:gameInit]\e[0;31merror:\e[0m %s\n", lua_tostring(L,-1));
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
void mods::loadConfig(){
    lua_getglobal(L,"sceneInit");
    lua_pushlightuserdata(L,this);
    if(lua_pcall(L, 1, 0, 0) != 0){
        printf("[sceneInit]\e[0;31merror:\e[0m %s\n", lua_tostring(L,-1));
        return;
    }
}
void mods::loadMesh(){
    building * b;
    
    auto creator=scene->getGeometryCreator();
    
    b = new building(
        creator->createCubeMesh(irr::core::vector3df(5.0f , 0.1f , 5.0f)),
        scene->getVideoDriver()->getTexture("./res/model/tree1/tree_trunk.tga"),
        "+b0 0 0 0 0 0 1 2.5 0.05 2.5\nM5 1 1 1",
        true,
        0.5,
        0.5
    );
    b->onAimAtBuildingCFunc=[](
        const irr::core::vector3df & aimAt,
        const irr::core::vector3df & trg,
        const irr::core::vector3df & trgRot,
        const irr::core::vector3df & camRot,
        int aimId,
        irr::core::vector3df & outPosi,
        irr::core::vector3df & outRot
    )->bool{
        
    };
    //b->haveOnAimAtBuilding=true;
    buildings[1]= b;
    
    b = new building(
        creator->createCubeMesh(irr::core::vector3df(5.0f , 5.0f , 0.1f)),
        scene->getVideoDriver()->getTexture("./res/model/tree1/tree_trunk.tga"),
        "+b0 0 0 0 0 0 1 2.5 2.5 0.05\nM5 1 1 1",
        true,
        0.5,
        0.5
    );
    buildings[2]=b;
    
    b = new building(
        creator->createCubeMesh(irr::core::vector3df(1.0f , 5.0f , 1.0f)),
        scene->getVideoDriver()->getTexture("./res/model/tree1/tree_trunk.tga"),
        "+b0 0 0 0 0 0 1 0.5 2.5 0.5\nM5 1 1 1",
        true,
        0.5,
        0.5
    );
    buildings[3]=b;
    
}
void mods::destroy(){
    for(auto it:items){
        delete it.second;
    }
    items.clear();
    
    for(auto it:buildings){
        delete it.second;
    }
    buildings.clear();
    
    for(auto it:subsConfs){
        delete it.second;
    }
    subsConfs.clear();
    
    for(auto it:animations){
        delete it.second;
    }
    animations.clear();
}
void mods::mapGenerator::autoGen(int x,int y,int tem,int hu,float h,mods * mod){
    std::map<long,float> pl;
    predictableRand randg;
    mod->getGenList(x,y,tem,hu,h,pl);
    randg.setSeed((x+10)*(y+20)*(hu+30)*(tem+40));
    for(auto it:pl){
        
        auto tinit=mod->terrainItemNum.find(it.first);
        if(tinit==mod->terrainItemNum.end())
            continue;
        
        
        int num=tinit->second;
        
        if(num<=0)
            continue;
        
        int delta=it.second*1000;
        for(int i=0;i<num;i++){
            int pr=(randg.rand())%1000;
            if(pr>delta){
                float mx=(randg.frand()+x)*32;
                float my=(randg.frand()+y)*32;
                float mr=randg.frand()*360;
                float mh=getRealHight(mx,my);
                //printf("autoGen:%f %f %f %f\n",mx,my,mh,mr);
                this->add(
                    it.first,
                    irr::core::vector3df(mx,mh,my),
                    irr::core::vector3df(0,mr,0),
                    irr::core::vector3df(1,1,1)
                );
            }
        }
        
    }
}

#define pushArg(func) \
    lua_settop(L,0); \
    lua_rawgeti(L,LUA_REGISTRYINDEX,func); \
    if(!lua_isfunction(L,-1)){ \
        lua_pop(L,1); \
        return; \
    } \
    lua_pushlightuserdata(L,self); \
    lua_pushstring(L,uuid.c_str()); \
    lua_pushstring(L,sowner.c_str());

void mods::subsConf::onHitSubs(
    lua_State * L,
    void * self,
    const std::string & uuid,
    const std::string & sowner,
    const std::string & tuuid,
    const std::string & tsowner,
    int imp
){
    pushArg(hitSubsCallback);
    lua_pushstring(L,tuuid.c_str());
    lua_pushstring(L,tsowner.c_str());
    lua_pushnumber(L,imp);
    
    if(lua_pcall(L, 6, 0, 0) != 0){
        //printf("error %s\n", lua_tostring(L,-1));
        return;
    }
}

void mods::subsConf::onHitBuildingCallback(
    lua_State * L,
    void * self,
    const std::string & uuid,
    const std::string & sowner,
    const std::string & tuuid,
    int imp
){
    pushArg(hitBuildingCallback);
    lua_pushstring(L,tuuid.c_str());
    lua_pushnumber(L,imp);
    
    if(lua_pcall(L, 5, 0, 0) != 0){
        //printf("error %s\n", lua_tostring(L,-1));
        return;
    }
}

void mods::subsConf::onHitTerrainItemCallback(
    lua_State * L,
    void * self,
    const std::string & uuid,
    const std::string & sowner,
    const mapid & mapid,
    int imp
){
    pushArg(hitTerrainItemCallback);
    lua_pushinteger(L,mapid.x);
    lua_pushinteger(L,mapid.y);
    lua_pushinteger(L,mapid.itemId);
    lua_pushinteger(L,mapid.mapId);
    lua_pushnumber(L,imp);
    
    if(lua_pcall(L, 8, 0, 0) != 0){
        //printf("error %s\n", lua_tostring(L,-1));
        return;
    }
}

void mods::subsConf::onHitTerrainCallback(
    lua_State * L,
    void * self,
    const std::string & uuid,
    const std::string & sowner,
    int imp
){
    pushArg(hitTerrainCallback);
    lua_pushnumber(L,imp);
    
    if(lua_pcall(L, 4, 0, 0) != 0){
        //printf("error %s\n", lua_tostring(L,-1));
        return;
    }
}

#undef pushArg
}//namespace smoothly