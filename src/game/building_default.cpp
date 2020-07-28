#include "building.h"

namespace smoothly{
/*
void genCubeBuffer(irr::scene::SMeshBuffer* buffer,
                   const vec3 & FLD,const vec3 & FRD,const vec3 & BRD,const vec3 & BLD,
                   const vec3 & FLU,const vec3 & FRU,const vec3 & BRU,const vec3 & BLU){
    int index=0;
    //添加一个面（顺时针）
    #define addFace(a,b,c,d) \
        buffer->Vertices.push_back(video::S3DVertex(a,vec3(0,0,1), video::SColor(255,255,255,255), 0, 1));\
        buffer->Vertices.push_back(video::S3DVertex(b,vec3(0,0,1), video::SColor(255,255,255,255), 0, 1));\
        buffer->Vertices.push_back(video::S3DVertex(c,vec3(0,0,1), video::SColor(255,255,255,255), 0, 1));\
        buffer->Vertices.push_back(video::S3DVertex(d,vec3(0,0,1), video::SColor(255,255,255,255), 0, 1));\
        buffer->Indices.push_back(index+1);\
        buffer->Indices.push_back(index+0);\
        buffer->Indices.push_back(index+2);\
        buffer->Indices.push_back(index+2);\
        buffer->Indices.push_back(index+0);\
        buffer->Indices.push_back(index+3);\
        index+=4;

    addFace(FLU,BLU,BRU,FRU);//顶面
    addFace(FLD,FLU,FRU,FRD);//正面
    addFace(BLD,BLU,FLU,FLD);//左面
    addFace(BRD,BRU,BLU,BLD);//背面
    addFace(FRD,FRU,BRU,BRD);//右面
    addFace(FRU,BRU,BLU,FLU);//底面
}
*/
void building::addDefaultBuilding(){
    def_shader = driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                "../shader/building_default.vs.glsl", "main", irr::video::EVST_VS_1_1,
                "../shader/building_default.ps.glsl", "main", irr::video::EPST_PS_1_1,
                &buildingShaderCallback);
    {
        auto mesh = scene->getMesh("../../res/model/building/building-1.obj");
        auto ptr  = new buildingConf;
        ptr->id   = -1;
        config[-1]= ptr;
        ptr->mesh[0] = mesh;
        ptr->texture = driver->getTexture("../../res/tree_trunk.tga");
        ptr->haveBody = true;
        ptr->bodyShape.init("+b0 0 0 0 0 0 1 5 0.5 5");
        ptr->canBuildOn = true;
        ptr->fetchBB=ptr->mesh[0]->getBoundingBox();
        ptr->useAutoAttach = true;
        ptr->autoAttach.deltaHor = 10;
        ptr->haveShader = true;
        ptr->shader = def_shader;
        ptr->desTexture = driver->getTexture("../../res/description/building-1.png");
    }
    {

        auto mesh = scene->getMesh("../../res/model/building/building-2.obj");
        //创建配置
        auto ptr  = new buildingConf;
        ptr->id   = -2;
        config[-2]= ptr;
        ptr->mesh[0] = mesh;
        ptr->texture = driver->getTexture("../../res/tree_trunk.tga");
        ptr->haveBody = true;
        ptr->bodyShape.init("+b0 5 5 0.924 0 0 0.383 5 0.5 7.07");
        ptr->canBuildOn = true;
        ptr->fetchBB=ptr->mesh[0]->getBoundingBox();
        ptr->useAutoAttach = true;
        ptr->autoAttach.deltaHor = 10;
        ptr->autoAttach.deltaHei = 10;
        ptr->haveShader = true;
        ptr->shader = def_shader;
        ptr->autoAttach.allowed[1]=false;
        ptr->autoAttach.allowed[2]=false;
        ptr->autoAttach.allowed[3]=false;

        ptr->desTexture = driver->getTexture("../../res/description/building-2.png");
    }
    {

        auto mesh = scene->getMesh("../../res/model/building/building-3.obj");
        auto ptr  = new buildingConf;
        ptr->id   = -3;
        config[-3]= ptr;
        ptr->mesh[0] = mesh;
        ptr->texture = driver->getTexture("../../res/tree_trunk.tga");
        ptr->haveBody = true;
        ptr->bodyShape.init("+b0 2.5 5 0.973 0 0 0.23 5 0.5 5.6");
        ptr->canBuildOn = true;
        ptr->fetchBB=ptr->mesh[0]->getBoundingBox();
        ptr->useAutoAttach = true;
        ptr->autoAttach.deltaHor = 10;
        ptr->autoAttach.deltaHei = 5;
        ptr->haveShader = true;
        ptr->shader = def_shader;
        ptr->autoAttach.allowed[1]=false;
        ptr->autoAttach.allowed[2]=false;
        ptr->autoAttach.allowed[3]=false;
        ptr->desTexture = driver->getTexture("../../res/description/building-3.png");
    }
    {
        auto mesh = scene->getMesh("../../res/model/building/building-4.obj");
        auto ptr  = new buildingConf;
        ptr->id   = -4;
        config[-4]= ptr;
        ptr->mesh[0] = mesh;
        ptr->texture = driver->getTexture("../../res/tree_trunk.tga");
        ptr->haveBody = true;
        ptr->bodyShape.init("+b0 5 0 0 0 0 1 5 5 0.5");
        ptr->canBuildOn = true;
        ptr->fetchBB=ptr->mesh[0]->getBoundingBox();
        ptr->useAutoAttach = true;
        ptr->autoAttach.deltaHor = 10;
        ptr->autoAttach.allowed[0]=false;
        ptr->autoAttach.allowed[1]=false;
        ptr->haveShader = true;
        ptr->shader = def_shader;
        ptr->desTexture = driver->getTexture("../../res/description/building-4.png");
    }

    {
        auto mesh = scene->getMesh("../../res/model/building/building-5.obj");
        //创建配置
        auto ptr  = new buildingConf;
        ptr->id   = -5;
        config[-5]= ptr;
        ptr->mesh[0] = mesh;
        ptr->texture = driver->getTexture("../../res/tree_trunk.tga");
        ptr->haveBody = true;
        ptr->bodyShape.init("+b0 -5 5 0.924 0 0 -0.383 5 0.5 7.07");
        ptr->canBuildOn = true;
        ptr->fetchBB=ptr->mesh[0]->getBoundingBox();
        ptr->useAutoAttach = true;
        ptr->autoAttach.deltaHor = 10;
        ptr->autoAttach.deltaHei = -10;
        ptr->haveShader = true;
        ptr->shader = def_shader;
        ptr->autoAttach.allowed[1]=false;
        ptr->autoAttach.allowed[2]=false;
        ptr->autoAttach.allowed[3]=false;

        ptr->desTexture = driver->getTexture("../../res/description/building-5.png");
    }
    {
        auto mesh = scene->getMesh("../../res/model/building/building-6.obj");
        //创建配置
        auto ptr  = new buildingConf;
        ptr->id   = -6;
        config[-6]= ptr;
        ptr->mesh[0] = mesh;
        ptr->texture = driver->getTexture("../../res/tree_trunk.tga");
        ptr->haveBody = true;
        ptr->bodyShape.init("+b0 -2.5 5 0.973 0 0 -0.23 5 0.5 5.6");
        ptr->canBuildOn = true;
        ptr->fetchBB=ptr->mesh[0]->getBoundingBox();
        ptr->useAutoAttach = true;
        ptr->autoAttach.deltaHor = 10;
        ptr->autoAttach.deltaHei = -5;
        ptr->haveShader = true;
        ptr->shader = def_shader;
        ptr->autoAttach.allowed[1]=false;
        ptr->autoAttach.allowed[2]=false;
        ptr->autoAttach.allowed[3]=false;
        ptr->desTexture = driver->getTexture("../../res/description/building-6.png");
    }
    //availableBuilding.push_back(-1);
    //availableBuilding.push_back(-2);
    //availableBuilding.push_back(-3);
    //availableBuilding.push_back(-4);
    //availableBuilding.push_back(-5);
    //availableBuilding.push_back(-6);
}

}
