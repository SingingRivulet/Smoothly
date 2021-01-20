#include "terrain.h"
namespace smoothly{

terrain::terrain(){
    setSeed(1234);
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
}
float terrain::genTerrain(float * img, int x , int y , int pointNum){
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
            img[i+j*33]=h;
        }
    }
    //printf("max:(%f,%f) begin:(%d,%d)\n",mx,my,begX,begY);
    return max;
}

void terrain::initChunk(terrain::chunk * res){
    //创建寻路单元
    for(int i=0;i<16;++i){
        for(int j=0;j<16;++j){
            res->collMap[i][j]=floor(res->mapBuf[i*2+j*2*33]/2);
        }
    }

    auto posi = vec3(res->x*32.0f , 0 , res->y*32.0f);

    res->updateLOD(res->x , res->y , cm_cx , cm_cy);

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
            res->mapBuf ,
            res->digMap ,
            33 , 33 ,
            irr::core::dimension2d<irr::f32>(1 , 1),
            irr::core::dimension2d<irr::u32>(33 , 33),
            meshLod,true
        );
        res->node[i] = scene->addMeshSceneNode(mesh,0,-1);
        res->node[i]->setPosition(posi);

        if(i==0){
            res->bodyMesh  = createBtMesh(mesh);
        }else if(i==2){
            res->shadowNode = createShadowNode(mesh,0,-1);
            res->shadowNode->setPosition(posi);
            res->shadowNode->getMaterial(0).BackfaceCulling = false;
            auto selector=scene->createOctreeTriangleSelector(mesh,res->node[i]);   //创建选择器
            res->node[i]->setTriangleSelector(selector);
            selector->drop();
        }

        res->node[i]->setMaterialFlag(irr::video::EMF_LIGHTING, true );
        res->node[i]->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true );

        //res->node[i]->setMaterialTexture(0,shadowMapTexture);
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
}

void terrain::updateChunk(terrain::chunk * p){
    if(p->rigidBody){
        dynamicsWorld->removeRigidBody(p->rigidBody);
    }
    if(p->rigidBody){
        delete p->rigidBody;
    }
    if(p->bodyState){
        delete p->bodyState;
    }
    if(p->bodyShape){
        delete p->bodyShape;
    }
    if(p->bodyMesh){
        delete p->bodyMesh;
    }
    for(int i=0;i<4;++i){
        if(p->node[i]){
            p->node[i]->remove();
        }
    }
    if(p->shadowNode){
        p->shadowNode->remove();
    }
    initChunk(p);
}
terrain::chunk * terrain::genChunk(int x,int y){
    //printf("[genChunk](%d,%d)\n",x,y);
    auto res = new chunk;
    res->mapBuf = new float[33*33];
    res->digMap = new int16_t[33*33];
    res->x = x;
    res->y = y;
    res->needUpdateMesh = false;
    for(int i=0;i<33*33;++i){
        res->digMap[i] = 0;
    }
    genTerrain(res->mapBuf , x , y , 33);

    initChunk(res);

    return res;
}
void terrain::freeChunk(terrain::chunk * p){
    if(p->rigidBody)
        dynamicsWorld->removeRigidBody(p->rigidBody);
    if(p->rigidBody)
        delete p->rigidBody;
    if(p->bodyState)
        delete p->bodyState;
    if(p->bodyShape)
        delete p->bodyShape;
    if(p->bodyMesh)
        delete p->bodyMesh;
    if(p->mapBuf)
        delete [] p->mapBuf;
    if(p->digMap)
        delete [] p->digMap;
    for(int i=0;i<4;++i){
        if(p->node[i])
            p->node[i]->remove();
    }
    if(p->shadowNode)
        p->shadowNode->remove();
    delete p;
}

#define findChunk(x,y) \
    auto it = chunks.find(ipair(x,y)); \
    if(it!=chunks.end())

void terrain::setDig(int x, int y, std::vector<std::pair<uint16_t,int16_t> > & dig){
    findChunk(x,y){
        for(auto d:dig){
            if(d.first<33*33){
                it->second->digMap[d.first] = d.second;
            }
        }
        updateChunk(it->second);
    }
}

void terrain::setDig(RakNet::BitStream * data){
    int32_t x,y,num,rx,ry;
    bool isMap,drawMode,oneChunkMode,comp;
    uint16_t compNum,index,range;
    int16_t dig;
    if(data->Read(x) && data->Read(y) && data->Read(isMap)){
        if(isMap){
            findChunk(x,y){
                int index = 0;
                while(index<33*33){
                    if(data->Read(comp)){//有comp说明压缩过
                        if(comp){
                            if(data->Read(compNum)){//获取数量
                                int i = 0;
                                while(index<33*33 && i<compNum){
                                    it->second->digMap[index] = 0;
                                    ++index;
                                    ++i;
                                }
                                if(index>=33*33){
                                    break;
                                }
                            }else{
                                break;
                            }
                        }else{
                            if(data->Read(dig)){
                                it->second->digMap[index] = dig;
                                ++index;
                            }
                        }
                    }else{
                        break;
                    }
                }
            }
        }else{
            if(data->Read(drawMode)){
                if(drawMode){
                    if(data->Read(oneChunkMode)){
                        if(oneChunkMode){
                            if(data->Read(rx) && data->Read(ry) && data->Read(range)){
                                setDig(x,y,rx,ry,range);
                            }
                        }else{
                            if(data->Read(range)){
                                setDig(x,y,range);
                            }
                        }
                    }
                }else{
                    if(data->Read(num)){
                        findChunk(x,y){
                            for(int i=0;i<num;++i){
                                if(data->Read(index) && data->Read(dig)){
                                    if(index<33*33){
                                        it->second->digMap[index] = dig;
                                    }
                                }else{
                                    break;
                                }
                            }
                            //updateChunk(it->second);
                            it->second->needUpdateMesh = true;
                        }
                    }
                }
            }
        }
    }
}

void terrain::setDig(int x, int y, int rx, int ry, int r){
    findChunk(x,y){
        chunk * c = it->second;

        int chunkBeginX = x*32;
        int chunkEndX   = chunkBeginX+32;
        int chunkBeginY = y*32;
        int chunkEndY   = chunkBeginY+32;

        int sweepBeginX = std::max(rx-r,chunkBeginX);
        int sweepEndX   = std::min(rx+r,chunkEndX);
        int sweepBeginY = std::max(ry-r,chunkBeginY);
        int sweepEndY   = std::min(ry+r,chunkEndY);

        for(int i=sweepBeginX;i<=sweepEndX;++i){
            for(int j=sweepBeginY;j<=sweepEndY;++j){

            }
        }
    }
}

void terrain::setDig(int x, int y, int r){
    ipair p[] = {
        ipair(x-r,y-r),
        ipair(x-r,y+r),
        ipair(x+r,y-r),
        ipair(x+r,y+r)
    };
    std::set<ipair> c;
    for(int i=0;i<4;++i){
        c.insert(ipair(floor(p[i].x/32.0),floor(p[i].y/32.0)));
    }
    for(auto it:c){
        setDig(it.x,it.y,x,y,r);
    }
}

void terrain::msg_setDigMap(RakNet::BitStream * data){
    setDig(data);
}

void terrain::msg_editDigMap(int x, int y, RakNet::BitStream * data){
    findChunk(x,y){
        int16_t index,depth,size;
        if(data->Read(size)){
            for(int i=0;i<size;++i){
                if(data->Read(index) && data->Read(depth)){
                    if(index>=0 && index<33*33){
                        it->second->digMap[index] = depth;
                    }
                    it->second->needUpdateMesh = true;
                }else{
                    break;
                }
            }
        }
    }
}
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
        for(auto & it:chunks){

            if(it.second->needUpdateMesh){
                it.second->needUpdateMesh = false;
                updateChunk(it.second);
            }

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
