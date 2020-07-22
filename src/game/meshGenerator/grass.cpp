#include "../terrain_item.h"
#include <functional>

namespace smoothly{

irr::scene::ISceneNode * terrain_item::genGrass(int seed,int & lodLevel){
    world::terrain::predictableRand randg;
    randg.setSeed(seed);

    auto res = scene->addMeshSceneNode(mesh_grass);
    res->setMaterialFlag(irr::video::EMF_LIGHTING, true );
    res->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true );
    res->setMaterialFlag(irr::video::EMF_ANISOTROPIC_FILTER, true );
    res->setMaterialType((irr::video::E_MATERIAL_TYPE)shader_tree);

    int r = randg.rand()%texture_grass.size();
    res->setMaterialTexture(0,texture_grass[r]);

    res->getMaterial(0).BlendOperation=irr::video::EBO_NONE;

    res->setRotation(vec3(0,randg.frand()*360,0));
    res->setScale(vec3(1+randg.frand()*2,1+randg.frand()*2,1+randg.frand()*4));

    int lodRand =  randg.rand() % 10;

    if(lodRand>7)
        lodLevel = 4;
    else
        lodLevel = 3;

    return res;
}

void terrain_item::genGrassMesh(){
    auto buffer = new irr::scene::SMeshBuffer();
    irr::video::S3DVertex v;
    v.Color.set(255,64,128,16);

    int len = 1;
    u32 index = 0;

#define drawSurface \
    v.Pos.X = -len;\
    v.Pos.Y = -len;\
    v.Pos.Z = 0;\
    transform.transformVect(v.Pos);\
    v.TCoords.X=0.f;\
    v.TCoords.Y=0.f;\
    v.Normal = v.Pos;\
    v.Normal.normalize();\
    buffer->Vertices.push_back(v);\
    v.Pos.X = -len;\
    v.Pos.Y = len;\
    v.Pos.Z = 0;\
    transform.transformVect(v.Pos);\
    v.TCoords.X=0.f;\
    v.TCoords.Y=1.f;\
    v.Normal = v.Pos;\
    v.Normal.normalize();\
    buffer->Vertices.push_back(v);\
    v.Pos.X = len;\
    v.Pos.Y = len;\
    v.Pos.Z = 0;\
    transform.transformVect(v.Pos);\
    v.TCoords.X=1.f;\
    v.TCoords.Y=1.f;\
    v.Normal = v.Pos;\
    v.Normal.normalize();\
    buffer->Vertices.push_back(v);\
    v.Pos.X = len;\
    v.Pos.Y = -len;\
    v.Pos.Z = 0;\
    transform.transformVect(v.Pos);\
    v.TCoords.X=1.f;\
    v.TCoords.Y=0.f;\
    v.Normal = v.Pos;\
    v.Normal.normalize();\
    buffer->Vertices.push_back(v);\
    buffer->Indices.push_back(index+0);\
    buffer->Indices.push_back(index+1);\
    buffer->Indices.push_back(index+3);\
    buffer->Indices.push_back(index+1);\
    buffer->Indices.push_back(index+2);\
    buffer->Indices.push_back(index+3);\
    index+=4;\
    v.Pos.X = -len;\
    v.Pos.Y = -len;\
    v.Pos.Z = 0;\
    transform.transformVect(v.Pos);\
    v.TCoords.X=0.f;\
    v.TCoords.Y=0.f;\
    v.Normal = v.Pos;\
    v.Normal.normalize();\
    buffer->Vertices.push_back(v);\
    v.Pos.X = -len;\
    v.Pos.Y = len;\
    v.Pos.Z = 0;\
    transform.transformVect(v.Pos);\
    v.TCoords.X=0.f;\
    v.TCoords.Y=1.f;\
    v.Normal = v.Pos;\
    v.Normal.normalize();\
    buffer->Vertices.push_back(v);\
    v.Pos.X = len;\
    v.Pos.Y = len;\
    v.Pos.Z = 0;\
    transform.transformVect(v.Pos);\
    v.TCoords.X=1.f;\
    v.TCoords.Y=1.f;\
    v.Normal = v.Pos;\
    v.Normal.normalize();\
    buffer->Vertices.push_back(v);\
    v.Pos.X = len;\
    v.Pos.Y = -len;\
    v.Pos.Z = 0;\
    transform.transformVect(v.Pos);\
    v.TCoords.X=1.f;\
    v.TCoords.Y=0.f;\
    v.Normal = v.Pos;\
    v.Normal.normalize();\
    buffer->Vertices.push_back(v);\
    buffer->Indices.push_back(index+1);\
    buffer->Indices.push_back(index+0);\
    buffer->Indices.push_back(index+2);\
    buffer->Indices.push_back(index+2);\
    buffer->Indices.push_back(index+0);\
    buffer->Indices.push_back(index+3);\
    index+=4;

    irr::core::matrix4 transform;
    drawSurface;
    transform.setRotationDegrees(vec3(0,60,0));
    drawSurface;
    transform.setRotationDegrees(vec3(0,120,0));
    drawSurface;

    buffer->recalculateBoundingBox();
    scene->getMeshManipulator()->recalculateNormals(buffer);

    auto mesh = new irr::scene::SMesh();
    mesh->addMeshBuffer(buffer);
    mesh->setHardwareMappingHint(irr::scene::EHM_STATIC);
    mesh->recalculateBoundingBox();
    mesh->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true );
    buffer->drop();

    mesh_grass = mesh;
}

}
