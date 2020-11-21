#include "terrain.h"
namespace smoothly{

terrain::terrain(){
    setSeed(1234);
    mapBuf=new float*[33];
    for(auto i=0;i<33;i++)
        mapBuf[i]=new float[33];
    altitudeK=0.08;
    hillK=100;
    temperatureK=0.3;
    humidityK=0.3;
    altitudeArg=20000;
    hillArg=500;
    temperatureArg=2000;
    humidityArg=2000;

    temperatureMax=2000;
    humidityMax=2000;

    temperatureMin=0;
    humidityMin=0;

    printf("[terrain]load shaderv1\n");
    shaderv1 = driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                "../shader/terrain.vs.glsl", "main", irr::video::EVST_VS_1_1,
                "../shader/terrain_lod1.ps.glsl", "main", irr::video::EPST_PS_1_1,
                &terrainShaderCallback);
    printf("[terrain]load shaderv2\n");
    shaderv2 = driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                "../shader/terrain.vs.glsl", "main", irr::video::EVST_VS_1_1,
                "../shader/terrain_lod2.ps.glsl", "main", irr::video::EPST_PS_1_1,
                &terrainShaderCallback);
    printf("[terrain]load shaderv3\n");
    shaderv3 = driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                "../shader/terrain.vs.glsl", "main", irr::video::EVST_VS_1_1,
                "../shader/terrain_lod3.ps.glsl", "main", irr::video::EPST_PS_1_1,
                &terrainShaderCallback);
    printf("[terrain]load shaderv4\n");
    shaderv4 = driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                "../shader/terrain.vs.glsl", "main", irr::video::EVST_VS_1_1,
                "../shader/terrain_lod4.ps.glsl", "main", irr::video::EPST_PS_1_1,
                &terrainShaderCallback);

    first = true;
    lastUCT = 0;

    terrainShaderCallback.parent = this;

}
terrain::~terrain(){
    for(auto it:chunks)
        freeChunk(it.second);

    for(auto i=0;i<33;i++)
        delete [] mapBuf[i];
    delete [] mapBuf;
}
float terrain::genTerrain(float ** img,int x , int y ,int pointNum){
    float h;
    float ix,iy;
    float max=0;
    float begX=x*32;
    float begY=y*32;
    float len=33.0f/((float)pointNum);
    //int mx = 0;
    //int my = 0;
    for(int i=0;i<pointNum;i++){
        for(int j=0;j<pointNum;j++){
            //ix=len*(pointNum-i+1)+begX;
            ix=len*i+begX;
            //iy=len*(pointNum-j+1)+begY;
            iy=len*j+begY;
            h=getRealHight(ix,iy);
            if(h>max){
                max=h;
                //mx=ix;
                //my=iy;
            }
            img[i][j]=h;
        }
    }
    //printf("max:(%f,%f) begin:(%d,%d)\n",mx,my,begX,begY);
    return max;
}
terrain::chunk * terrain::genChunk(int x,int y){
    //printf("[genChunk](%d,%d)\n",x,y);
    auto res = new chunk;
    genTerrain(mapBuf , x , y , 33);
    //创建寻路单元
    for(int i=0;i<16;++i){
        for(int j=0;j<16;++j){
            res->collMap[i][j]=floor(mapBuf[i*2][j*2]/2);
        }
    }

    auto posi = vec3(x*32.0f , 0 , y*32.0f);

    res->updateLOD(x,y,cm_cx,cm_cy);

    for(int i=0;i<4;++i){
        int meshLod = 1;
        if(i==1)
            meshLod = 4;
        else if(i==2)
            meshLod = 8;
        else if(i==3)
            meshLod = 16;

        auto mesh=this->createTerrainMesh(
            NULL ,
            mapBuf , 33 , 33 ,
            irr::core::dimension2d<irr::f32>(1 , 1),
            irr::core::dimension2d<irr::u32>(33 , 33),
            meshLod,true
        );
        res->node[i] = scene->addMeshSceneNode(mesh,0,-1);
        res->node[i]->setPosition(posi);

        if(i==0){
            res->shadowNode = createShadowNode(mesh,0,-1);
            res->shadowNode->setPosition(posi);
            res->shadowNode->getMaterial(0).BackfaceCulling = false;
            auto selector=scene->createOctreeTriangleSelector(mesh,res->node[i]);   //创建选择器
            res->node[i]->setTriangleSelector(selector);
            selector->drop();
            res->bodyMesh  = createBtMesh(mesh);
        }

        res->node[i]->setMaterialFlag(irr::video::EMF_LIGHTING, true );
        res->node[i]->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true );

        res->node[i]->setMaterialTexture(0,shadowMapTexture);
        res->node[i]->getMaterial(0).ZWriteFineControl = irr::video::EZI_ZBUFFER_FLAG;
        //res->node[i]->getMaterial(0).Wireframe = true;

        mesh->drop();
    }


    res->node[0]->setMaterialType((irr::video::E_MATERIAL_TYPE)shaderv1);
    res->node[1]->setMaterialType((irr::video::E_MATERIAL_TYPE)shaderv2);
    res->node[2]->setMaterialType((irr::video::E_MATERIAL_TYPE)shaderv3);
    res->node[3]->setMaterialType((irr::video::E_MATERIAL_TYPE)shaderv4);

    res->node[0]->updateAbsolutePosition();
    res->bodyState =setMotionState(res->node[0]->getAbsoluteTransformation().pointer());

    res->bodyShape =createShape(res->bodyMesh);

    res->rigidBody =createBody(res->bodyShape,res->bodyState);
    res->rigidBody->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
    res->rigidBody->setFriction(0.7);
    res->rigidBody->setRestitution(0.1);
    
    res->info.type=BODY_TERRAIN;
    res->info.ptr=res;
    res->rigidBody->setUserPointer(&(res->info));
    
    this->dynamicsWorld->addRigidBody(res->rigidBody);

    res->show();

    return res;
}
void terrain::freeChunk(terrain::chunk * p){
    dynamicsWorld->removeRigidBody(p->rigidBody);
    delete p->rigidBody;
    delete p->bodyState;
    delete p->bodyShape;
    delete p->bodyMesh;
    for(int i=0;i<4;++i){
        p->node[i]->remove();
    }
    p->shadowNode->remove();
    delete p;
}

#define findChunk(x,y) \
    auto it = chunks.find(ipair(x,y)); \
    if(it!=chunks.end())

void terrain::createChunk(int x,int y){
    findChunk(x,y){

    }else{
        chunks[ipair(x,y)] = genChunk(x,y);
        buildingChunkFetch(x,y);
    }
}
void terrain::showChunk(int x,int y){
    findChunk(x,y){
        it->second->show();
    }
}
void terrain::hideChunk(int x,int y){
    findChunk(x,y){
        it->second->hide();
    }
}
void terrain::releaseChunk(int x,int y){
    findChunk(x,y){
        freeChunk(it->second);
        chunks.erase(it);
    }
}
bool terrain::chunkShowing(int x,int y){
    auto it = chunks.find(ipair(x,y));
    if(it!=chunks.end()){
        return it->second->node[0]->isVisible() || it->second->node[1]->isVisible() || it->second->node[2]->isVisible() || it->second->node[3]->isVisible();
    }else
        return false;
}

void terrain::loop(){
    building::loop();
    //每2秒强制进行一次lod更新
    int tm = time(0);
    if(updateCamChunk() || abs(lastUCT-tm)>2){
        lastUCT = tm;
        for(auto it:chunks){
            ipair ch = it.first;
            int l = std::max(abs(ch.x-cm_cx),abs(ch.y-cm_cy));
            it.second->updateLOD(ch.x,ch.y,cm_cx,cm_cy);
            if(l<=getVisualRange()){
                updateLOD(ch.x,ch.y,it.second->lodLevel);
                it.second->show();
            }else{
                updateLOD(ch.x,ch.y,0);
                it.second->hide();
            }
        }
    }
}

bool terrain::chunkCreated(int x, int y){
    auto it = chunks.find(ipair(x,y));
    return it!=chunks.end() && buildingChunkCreated(x,y);
}

bool terrain::selectByRay(const irr::core::line3d<irr::f32> &ray, vec3 &outCollisionPoint, irr::core::triangle3df &outTriangle, irr::scene::ISceneNode *&outNode){
    int cx = floor(ray.start.X/32);
    int cy = floor(ray.start.Z/32);//得到区块
    float ipx = ray.start.X-cx*32;
    float ipy = ray.start.Z-cy*32;//得到区块delta
    if(selectPointInChunk(cx,cy,ray,outCollisionPoint,outTriangle,outNode))
        return true;
    if(ipx>16){
        if(ipy>16){
            //(cx+1,cy+1)
            if(selectPointInChunk(cx+1,cy,ray,outCollisionPoint,outTriangle,outNode))
                return true;
            else
            if(selectPointInChunk(cx+1,cy+1,ray,outCollisionPoint,outTriangle,outNode))
                return true;
            else
            if(selectPointInChunk(cx  ,cy+1,ray,outCollisionPoint,outTriangle,outNode))
                return true;
            else
                return false;
        }else{
            //(cx+1,cy-1)
            if(selectPointInChunk(cx+1,cy,ray,outCollisionPoint,outTriangle,outNode))
                return true;
            else
            if(selectPointInChunk(cx+1,cy-1,ray,outCollisionPoint,outTriangle,outNode))
                return true;
            else
            if(selectPointInChunk(cx  ,cy-1,ray,outCollisionPoint,outTriangle,outNode))
                return true;
            else
                return false;
        }
    }else{
        if(ipy>16){
            //(cx-1,cy+1)
            if(selectPointInChunk(cx-1,cy,ray,outCollisionPoint,outTriangle,outNode))
                return true;
            else
            if(selectPointInChunk(cx-1,cy+1,ray,outCollisionPoint,outTriangle,outNode))
                return true;
            else
            if(selectPointInChunk(cx  ,cy+1,ray,outCollisionPoint,outTriangle,outNode))
                return true;
            else
                return false;
        }else{
            //(cx-1,cy-1)
            if(selectPointInChunk(cx-1,cy,ray,outCollisionPoint,outTriangle,outNode))
                return true;
            else
            if(selectPointInChunk(cx-1,cy-1,ray,outCollisionPoint,outTriangle,outNode))
                return true;
            else
            if(selectPointInChunk(cx  ,cy-1,ray,outCollisionPoint,outTriangle,outNode))
                return true;
            else
                return false;
        }
    }
}

int terrain::getLodLevel(int x, int y){
    int len = std::max(abs(x-cm_cx) , abs(y-cm_cy));
    if(len>getVisualRange())
        return 0;
    if(len<2){
        return 1;
    }else
    if(len<4){
        return 2;
    }else
    if(len<8){
        return 3;
    }else{
        return 4;
    }
}

int terrain::getCollHeight(int x, int y){
    int tx = floor(x/16.0);
    int ty = floor(y/16.0);
    int cx = x - tx*16;
    int cy = y - ty*16;
    if(cx<0 || cy<0 || cx>=16 || cy>=16){
        printf("[error]getCollHeight:%d %d cx=%d cy=%d\n",x,y,cx,cy);
        return 0;
    }
    auto it = chunks.find(ipair(tx,ty));
    if(it!=chunks.end()){
        chunk * c = it->second;
        return c->collMap[cx][cy];
    }else
        return 0;
}

void terrain::TerrainShaderCallback::OnSetConstants(video::IMaterialRendererServices * services, s32 userData){
    services->setPixelShaderConstant(services->getPixelShaderConstantID("shadowMapSize"),&parent->shadowMapSize, 1);

    services->setPixelShaderConstant(services->getPixelShaderConstantID("clipY"),&parent->clipY, 1);
    services->setPixelShaderConstant(services->getPixelShaderConstantID("clipYUp"),&parent->clipYUp, 1);
    services->setPixelShaderConstant(services->getPixelShaderConstantID("enableClipY"),&parent->enableClipY, 1);

    s32 var0 = 0;

    irr::f32 sas = parent->scan_animation_showing;
    services->setPixelShaderConstant(services->getPixelShaderConstantID("scan_animation_showing"),&sas, 1);
    services->setPixelShaderConstant(services->getPixelShaderConstantID("scan_animation_size"),&parent->scan_animation_size, 1);

    auto campos = parent->camera->getPosition();
    services->setPixelShaderConstant(services->getPixelShaderConstantID("campos"),&campos.X, 3);

    services->setPixelShaderConstant(services->getPixelShaderConstantID("shadowMap"),&var0, 1);
    services->setPixelShaderConstant(services->getPixelShaderConstantID("shadowFactor"),&parent->shadowFactor, 1);
    services->setVertexShaderConstant(services->getVertexShaderConstantID("shadowMatrix") , parent->shadowMatrix.pointer() , 16);
    core::matrix4 world = parent->driver->getTransform(video::ETS_WORLD);
    services->setVertexShaderConstant(services->getVertexShaderConstantID("modelMatrix") , world.pointer() , 16);
    services->setPixelShaderConstant(services->getVertexShaderConstantID("ambientColor"), &parent->ambientColor.r, 4);
    services->setVertexShaderConstant(services->getVertexShaderConstantID("transformMatrix") , (parent->camera->getProjectionMatrix()*parent->camera->getViewMatrix()).pointer() , 16);
    {
        irr::video::SColor color;
        irr::video::E_FOG_TYPE fogType;
        f32 start;
        f32 end;
        f32 density;
        bool pixelFog;
        bool rangeFog;
        parent->driver->getFog(color, fogType, start, end, density, pixelFog, rangeFog);
        services->setPixelShaderConstant(services->getVertexShaderConstantID("FogMode"), (int*)&fogType, 1);
    }
}

}
