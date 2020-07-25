#include "building.h"

namespace smoothly{

void building::markVoxel(const building::blockPosition & p, int delta){
    auto it = collTable.find(p);
    if(it!=collTable.end()){
        it->second+=delta;
        if(it->second <= 0){
            collTable.erase(it);
        }
    }else{
        if(delta>0){
            collTable[p] = delta;
        }
    }
}

void building::showVoxelsByRay(const vec3 & start, const vec3 & end){
    fetchByRay(start , end,[&](const vec3 & ,bodyInfo * b){
        if(b->type==BODY_BUILDING){
            auto tb = (buildingBody*)b->ptr;
            if(tb){
                tb->showVoxels();
            }
        }
    });
}

void building::showVoxelsByCamera(){
    auto ori    = camera->getPosition();
    auto dir    = camera->getTarget()-ori;
    dir.normalize();
    auto start  = ori;
    auto end    = ori+dir*100;
    showVoxelsByRay(start,end);
}

void building::buildingBody::showVoxels(){
    getVoxels([&](const blockPosition & p){
        vec3 posi(
                    p.x*2+1,
                    p.y*2+1,
                    p.z*2+1);
        auto n = parent->scene->addCubeSceneNode(2,0,-1,posi);
        n->setMaterialTexture(0,parent->texture_collPoint);
        n->setMaterialFlag(irr::video::EMF_LIGHTING, false );
        n->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);
        n->getMaterial(0).ZWriteFineControl = irr::video::EZI_ZBUFFER_FLAG;
        auto am = parent->scene->createDeleteAnimator(1000);
        n->addAnimator(am);
        am->drop();
    });
}

void building::buildingBody::getVoxels(std::function<void (const building::blockPosition &)> callback){
    auto mat = node[0]->getAbsoluteTransformation();
    std::set<blockPosition> voxels;
    for(auto it:config->collisionVoxels){
        vec3 tmp = it;
        mat.transformVect(tmp);
        blockPosition posi(floor(tmp.X/2),floor(tmp.Y/2),floor(tmp.Z/2));
        voxels.insert(posi);
    }
    for(auto it:voxels){
        callback(it);
    }
}

static int luafunc_addVoxel(lua_State * L){
    if(!lua_isuserdata(L,1))
        return 0;
    void * ptr=lua_touserdata(L,1);
    if(ptr==NULL)
        return 0;
    auto self=(building::buildingConf*)ptr;
    auto x = luaL_checknumber(L,2);
    auto y = luaL_checknumber(L,3);
    auto z = luaL_checknumber(L,4);
    self->collisionVoxels.push_back(vec3(x,y,z));
    lua_pushboolean(L,true);
    return 1;
}

static int luafunc_seekConfig(lua_State * L){//定位配置
    if(!lua_isuserdata(L,1))
        return 0;
    void * ptr=lua_touserdata(L,1);
    if(ptr==NULL)
        return 0;
    auto self=(building*)ptr;
    auto id = luaL_checkinteger(L,2);
    auto it = self->config.find(id);
    if(it!=self->config.end()){
        building::buildingConf * conf = it->second;
        lua_pushlightuserdata(L,conf);
        return 1;
    }
    return 0;
}

void building::pathFindingInit(){
    lua_pushcfunction(L,luafunc_addVoxel);
    lua_setglobal(L,"addVoxel");
    lua_pushcfunction(L,luafunc_seekConfig);
    lua_setglobal(L,"seekConfig");

    lua_getglobal(L,"pathFindingInit");
    if(lua_isfunction(L,-1)){

        lua_pushlightuserdata(L,this);
        if (lua_pcall(L, 1, 0, 0) != 0)
             printf("error running function : %s \n",lua_tostring(L, -1));
        else{

        }

    }
    lua_settop(L , 0);
}

}
