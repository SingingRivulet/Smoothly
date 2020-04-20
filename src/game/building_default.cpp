#include "building.h"

namespace smoothly{

void building::addDefaultBuilding(){
    def_shader = driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                "../shader/building_default.vs.glsl", "main", irr::video::EVST_VS_1_1,
                "../shader/building_default.ps.glsl", "main", irr::video::EPST_PS_1_1);
    {
        auto ptr  = new conf;
        ptr->id   = -1;
        config[-1]= ptr;
        ptr->mesh[0] = scene->getGeometryCreator()->createCubeMesh(vec3(10,0.5,10));
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
        irr::scene::SMeshBuffer* buffer = new irr::scene::SMeshBuffer();

        // Create indices
        const u16 u[36] = {   0,2,1,   0,3,2,   1,5,4,   1,2,5,   4,6,7,   4,5,6,
                              7,3,0,   7,6,3,   9,5,2,   9,8,5,   0,11,10,   0,10,7};

        buffer->Indices.set_used(36);

        for (u32 i=0; i<36; ++i)
            buffer->Indices[i] = u[i];


        // Create vertices
        video::SColor clr(255,255,255,255);

        buffer->Vertices.reallocate(12);

        buffer->Vertices.push_back(video::S3DVertex(-5,-0.5,0, -1,-1,-1, clr, 0, 1));
        buffer->Vertices.push_back(video::S3DVertex(5 ,-0.5,0,  1,-1,-1, clr, 1, 1));
        buffer->Vertices.push_back(video::S3DVertex(5 ,0   ,0,  1, 1,-1, clr, 1, 0));
        buffer->Vertices.push_back(video::S3DVertex(-5,0   ,0, -1, 1,-1, clr, 0, 0));
        buffer->Vertices.push_back(video::S3DVertex(5 ,9.5 ,10,  1,-1, 1, clr, 0, 1));
        buffer->Vertices.push_back(video::S3DVertex(5 ,10  ,10,  1, 1, 1, clr, 0, 0));
        buffer->Vertices.push_back(video::S3DVertex(-5,10  ,10, -1, 1, 1, clr, 1, 0));
        buffer->Vertices.push_back(video::S3DVertex(-5,9.5 ,10, -1,-1, 1, clr, 1, 1));
        buffer->Vertices.push_back(video::S3DVertex(-5,10  ,10, -1, 1, 1, clr, 0, 1));
        buffer->Vertices.push_back(video::S3DVertex(-5,0   ,0, -1, 1,-1, clr, 1, 1));
        buffer->Vertices.push_back(video::S3DVertex(5 ,9.5 ,10,  1,-1, 1, clr, 1, 0));
        buffer->Vertices.push_back(video::S3DVertex(5 ,-0.5,0,  1,-1,-1, clr, 0, 0));

        // Recalculate bounding box
        buffer->BoundingBox.reset(0,0,0);

        for (u32 i=0; i<12; ++i){
            buffer->BoundingBox.addInternalPoint(buffer->Vertices[i].Pos);
        }

        irr::scene::SMesh* mesh = new irr::scene::SMesh;
        mesh->addMeshBuffer(buffer);
        buffer->drop();

        mesh->recalculateBoundingBox();

        //创建配置
        auto ptr  = new conf;
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
        irr::scene::SMeshBuffer* buffer = new irr::scene::SMeshBuffer();

        // Create indices
        const u16 u[36] = {   0,2,1,   0,3,2,   1,5,4,   1,2,5,   4,6,7,   4,5,6,
                              7,3,0,   7,6,3,   9,5,2,   9,8,5,   0,11,10,   0,10,7};

        buffer->Indices.set_used(36);

        for (u32 i=0; i<36; ++i)
            buffer->Indices[i] = u[i];


        // Create vertices
        video::SColor clr(255,255,255,255);

        buffer->Vertices.reallocate(12);

        buffer->Vertices.push_back(video::S3DVertex(-5,-0.5,0, -1,-1,-1, clr, 0, 1));
        buffer->Vertices.push_back(video::S3DVertex(5 ,-0.5,0,  1,-1,-1, clr, 1, 1));
        buffer->Vertices.push_back(video::S3DVertex(5 ,0   ,0,  1, 1,-1, clr, 1, 0));
        buffer->Vertices.push_back(video::S3DVertex(-5,0   ,0, -1, 1,-1, clr, 0, 0));
        buffer->Vertices.push_back(video::S3DVertex(5 ,4.5 ,10,  1,-1, 1, clr, 0, 1));
        buffer->Vertices.push_back(video::S3DVertex(5 ,5   ,10,  1, 1, 1, clr, 0, 0));
        buffer->Vertices.push_back(video::S3DVertex(-5,5   ,10, -1, 1, 1, clr, 1, 0));
        buffer->Vertices.push_back(video::S3DVertex(-5,4.5 ,10, -1,-1, 1, clr, 1, 1));
        buffer->Vertices.push_back(video::S3DVertex(-5,5   ,10, -1, 1, 1, clr, 0, 1));
        buffer->Vertices.push_back(video::S3DVertex(-5,0   ,0, -1, 1,-1, clr, 1, 1));
        buffer->Vertices.push_back(video::S3DVertex(5 ,4.5 ,10,  1,-1, 1, clr, 1, 0));
        buffer->Vertices.push_back(video::S3DVertex(5 ,-0.5,0,  1,-1,-1, clr, 0, 0));

        // Recalculate bounding box
        buffer->BoundingBox.reset(0,0,0);

        for (u32 i=0; i<12; ++i){
            buffer->BoundingBox.addInternalPoint(buffer->Vertices[i].Pos);
        }

        irr::scene::SMesh* mesh = new irr::scene::SMesh;
        mesh->addMeshBuffer(buffer);
        buffer->drop();

        mesh->recalculateBoundingBox();

        //创建配置
        auto ptr  = new conf;
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
        irr::scene::SMeshBuffer* buffer = new irr::scene::SMeshBuffer();

        // Create indices
        const u16 u[36] = {   0,2,1,   0,3,2,   1,5,4,   1,2,5,   4,6,7,   4,5,6,
                              7,3,0,   7,6,3,   9,5,2,   9,8,5,   0,11,10,   0,10,7};

        buffer->Indices.set_used(36);

        for (u32 i=0; i<36; ++i)
            buffer->Indices[i] = u[i];


        // Create vertices
        video::SColor clr(255,255,255,255);

        buffer->Vertices.reallocate(12);

        buffer->Vertices.push_back(video::S3DVertex(-5,0,0, -1,-1,-1, clr, 0, 1));
        buffer->Vertices.push_back(video::S3DVertex(5,0,0,  1,-1,-1, clr, 1, 1));
        buffer->Vertices.push_back(video::S3DVertex(5,10,0,  1, 1,-1, clr, 1, 0));
        buffer->Vertices.push_back(video::S3DVertex(-5,10,0, -1, 1,-1, clr, 0, 0));
        buffer->Vertices.push_back(video::S3DVertex(5,0,0.25,  1,-1, 1, clr, 0, 1));
        buffer->Vertices.push_back(video::S3DVertex(5,10,0.25,  1, 1, 1, clr, 0, 0));
        buffer->Vertices.push_back(video::S3DVertex(-5,10,0.25, -1, 1, 1, clr, 1, 0));
        buffer->Vertices.push_back(video::S3DVertex(-5,0,0.25, -1,-1, 1, clr, 1, 1));
        buffer->Vertices.push_back(video::S3DVertex(-5,10,0.25, -1, 1, 1, clr, 0, 1));
        buffer->Vertices.push_back(video::S3DVertex(-5,10,0, -1, 1,-1, clr, 1, 1));
        buffer->Vertices.push_back(video::S3DVertex(5,0,0.25,  1,-1, 1, clr, 1, 0));
        buffer->Vertices.push_back(video::S3DVertex(5,0,0,  1,-1,-1, clr, 0, 0));
        // Recalculate bounding box
        buffer->BoundingBox.reset(0,0,0);

        for (u32 i=0; i<12; ++i){
            buffer->BoundingBox.addInternalPoint(buffer->Vertices[i].Pos);
        }

        irr::scene::SMesh* mesh = new irr::scene::SMesh;
        mesh->addMeshBuffer(buffer);
        buffer->drop();

        mesh->recalculateBoundingBox();

        auto ptr  = new conf;
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
        irr::scene::SMeshBuffer* buffer = new irr::scene::SMeshBuffer();

        // Create indices
        const u16 u[36] = {   0,2,1,   0,3,2,   1,5,4,   1,2,5,   4,6,7,   4,5,6,
                              7,3,0,   7,6,3,   9,5,2,   9,8,5,   0,11,10,   0,10,7};

        buffer->Indices.set_used(36);

        for (u32 i=0; i<36; ++i)
            buffer->Indices[i] = u[i];


        // Create vertices
        video::SColor clr(255,255,255,255);

        buffer->Vertices.reallocate(12);

        buffer->Vertices.push_back(video::S3DVertex(-5,-0.5,0, -1,-1,-1, clr, 0, 1));
        buffer->Vertices.push_back(video::S3DVertex(5 ,-0.5,0,  1,-1,-1, clr, 1, 1));
        buffer->Vertices.push_back(video::S3DVertex(5 ,0   ,0,  1, 1,-1, clr, 1, 0));
        buffer->Vertices.push_back(video::S3DVertex(-5,0   ,0, -1, 1,-1, clr, 0, 0));
        buffer->Vertices.push_back(video::S3DVertex(5 ,-10.5 ,10,  1,-1, 1, clr, 0, 1));
        buffer->Vertices.push_back(video::S3DVertex(5 ,-10  ,10,  1, 1, 1, clr, 0, 0));
        buffer->Vertices.push_back(video::S3DVertex(-5,-10  ,10, -1, 1, 1, clr, 1, 0));
        buffer->Vertices.push_back(video::S3DVertex(-5,-10.5 ,10, -1,-1, 1, clr, 1, 1));
        buffer->Vertices.push_back(video::S3DVertex(-5,-10  ,10, -1, 1, 1, clr, 0, 1));
        buffer->Vertices.push_back(video::S3DVertex(-5,0   ,0, -1, 1,-1, clr, 1, 1));
        buffer->Vertices.push_back(video::S3DVertex(5 ,-10.5 ,10,  1,-1, 1, clr, 1, 0));
        buffer->Vertices.push_back(video::S3DVertex(5 ,-0.5,0,  1,-1,-1, clr, 0, 0));

        // Recalculate bounding box
        buffer->BoundingBox.reset(0,0,0);

        for (u32 i=0; i<12; ++i){
            buffer->BoundingBox.addInternalPoint(buffer->Vertices[i].Pos);
        }

        irr::scene::SMesh* mesh = new irr::scene::SMesh;
        mesh->addMeshBuffer(buffer);
        buffer->drop();

        mesh->recalculateBoundingBox();

        //创建配置
        auto ptr  = new conf;
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
        irr::scene::SMeshBuffer* buffer = new irr::scene::SMeshBuffer();

        // Create indices
        const u16 u[36] = {   0,2,1,   0,3,2,   1,5,4,   1,2,5,   4,6,7,   4,5,6,
                              7,3,0,   7,6,3,   9,5,2,   9,8,5,   0,11,10,   0,10,7};

        buffer->Indices.set_used(36);

        for (u32 i=0; i<36; ++i)
            buffer->Indices[i] = u[i];


        // Create vertices
        video::SColor clr(255,255,255,255);

        buffer->Vertices.reallocate(12);

        buffer->Vertices.push_back(video::S3DVertex(-5,-0.5,0, -1,-1,-1, clr, 0, 1));
        buffer->Vertices.push_back(video::S3DVertex(5 ,-0.5,0,  1,-1,-1, clr, 1, 1));
        buffer->Vertices.push_back(video::S3DVertex(5 ,0   ,0,  1, 1,-1, clr, 1, 0));
        buffer->Vertices.push_back(video::S3DVertex(-5,0   ,0, -1, 1,-1, clr, 0, 0));
        buffer->Vertices.push_back(video::S3DVertex(5 ,-5.5 ,10,  1,-1, 1, clr, 0, 1));
        buffer->Vertices.push_back(video::S3DVertex(5 ,-5   ,10,  1, 1, 1, clr, 0, 0));
        buffer->Vertices.push_back(video::S3DVertex(-5,-5   ,10, -1, 1, 1, clr, 1, 0));
        buffer->Vertices.push_back(video::S3DVertex(-5,-5.5 ,10, -1,-1, 1, clr, 1, 1));
        buffer->Vertices.push_back(video::S3DVertex(-5,-5   ,10, -1, 1, 1, clr, 0, 1));
        buffer->Vertices.push_back(video::S3DVertex(-5,0   ,0, -1, 1,-1, clr, 1, 1));
        buffer->Vertices.push_back(video::S3DVertex(5 ,-5.5 ,10,  1,-1, 1, clr, 1, 0));
        buffer->Vertices.push_back(video::S3DVertex(5 ,-0.5,0,  1,-1,-1, clr, 0, 0));

        // Recalculate bounding box
        buffer->BoundingBox.reset(0,0,0);

        for (u32 i=0; i<12; ++i){
            buffer->BoundingBox.addInternalPoint(buffer->Vertices[i].Pos);
        }

        irr::scene::SMesh* mesh = new irr::scene::SMesh;
        mesh->addMeshBuffer(buffer);
        buffer->drop();

        mesh->recalculateBoundingBox();

        //创建配置
        auto ptr  = new conf;
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
    availableBuilding.push_back(-1);
    availableBuilding.push_back(-2);
    availableBuilding.push_back(-3);
    availableBuilding.push_back(-4);
    availableBuilding.push_back(-5);
    availableBuilding.push_back(-6);
}

}
