#include "terrain_item.h"
#include <stdio.h>
#include <QString>
#include <raknet/RakSleep.h>
namespace smoothly{

#define findChunk(x,y) \
    auto it = chunks.find(ipair(x,y)); \
    if(it!=chunks.end())

bool terrain_item::setRemoveTable(int x, int y, const std::set<mapItem> & rmt, bool cache){
    chunk * ptr;
    if(!chunkLoaded(x,y)){//chunk未被创建，将不再创建items，并保存到cache
        if(cache){
            rmtCache[ipair(x,y)] = rmt;
        }
        return false;
    }
    findChunk(x,y){
        return true;
        //ptr = it->second;
        //releaseChildren(ptr);
    }else{
        ptr = new chunk;
        ptr->x = x;
        ptr->y = y;
        ptr->parent = this;
        ptr->lodLevel = 0;
        linkChunk(ptr,x,y);
        chunks[ipair(x,y)]=ptr;
    }
    printf("[setRemoveTable](%d,%d)\n",x,y);
    std::list<genProb> pl;//物体->概率
    world::terrain::predictableRand randg;

    int tem = getTemperatureF(x,y);
    int hu  = getHumidityF(x,y);

    getGenList(x,y,tem,hu,getRealHight(x*32,y*32),pl);

    randg.setSeed((x+10)*(y+20)*(hu+30)*(tem+40));

    flatCells.clear();
    auto tbuf = genTerrain(x,y,mapBuffer);
    for(int i=0;i<16;++i){
        for(int j=0;j<16;++j){
            int ti = i*2;
            int tj = j*2;
            float d = (fabs(tbuf[ti+tj*33]-tbuf[ti+1+tj*33])+fabs(tbuf[ti+tj*33]-tbuf[ti+(tj+1)*33]))*0.5;
            if(d<0.5){
                flatCells.push_back(ipair(ti,tj));
            }
        }
    }

    for(auto it:pl){
        int delta=it.prob*1000;
        int num = it.num;

        bool flat = false;

        if(it.id>0){
            auto bit = config.find(it.id);
            if(bit==config.end())
                continue;

            conf * c = bit->second;
            flat = c->flat;
        }else{
            if(it.id==-2)
                flat = true;
        }

        for(int i=0;i<num;i++){
            int pr=(randg.rand())%1000;
            if(pr<delta){
                float mx;
                float my;

                if(flat){
                    if(flatCells.empty())
                        continue;
                    int ci = randg.rand()%flatCells.size();
                    auto cell = flatCells[ci];
                    mx = x*32+(cell.x+randg.frand())*2;
                    my = y*32+(cell.y+randg.frand())*2;
                }else{
                    mx=(randg.frand()+x)*32;
                    my=(randg.frand()+y)*32;
                }

                float mr=randg.frand()*360;
                if(rmt.find(mapItem(it.id , i))==rmt.end()){
                    auto im=makeTerrainItem(it.id , i , mx , my , mr);
                    if(im==NULL)
                        continue;

                    ptr->children[mapItem(it.id , i)]=im;
                    im->parent = ptr;
                }
            }
        }
    }
    {
        ptr->minimap_element = NULL;
        cmd_getChunkACL(x,y);
        //auto cr = mapScene->getGeometryCreator();
        //auto m = cr->createPlaneMesh(irr::core::dimension2df(32,32));
        //auto n = mapScene->addMeshSceneNode(m,0,-1,vec3(x*32,0,y*32));
        //n->setMaterialFlag(irr::video::EMF_LIGHTING, false );
        //ptr->minimap_element = n;
        //m->drop();
    }
    return true;
}

void terrain_item::pushRemoveTable(int x, int y, const std::set<mapItem> &r){
    rmt tmp;
    tmp.x = x;
    tmp.y = y;
    tmp.r = r;
    rmtQueue.push(tmp);
}

void terrain_item::releaseChunk(int x,int y){
    printf("[releaseChunk](%d,%d)\n",x,y);
    terrain::releaseChunk(x,y);
    auto it = chunks.find(ipair(x,y));
    if(it!=chunks.end()){
        it->second->unlink();
        releaseChildren(it->second);
        chunks.erase(it);
        delete it->second;
    }
}

void terrain_item::loop(){
    terrain::loop();
    clock_t starts,ends;
    starts=clock();
    while(!rmtQueue.empty()){
        auto t = rmtQueue.front();
        setRemoveTable(t.x,t.y,t.r);
        rmtQueue.pop();
        ends = clock();
        if((ends-starts)/(CLOCKS_PER_SEC/1000)>1)
            break;
    }

    hizBegin();
    for(auto it:chunks){
        it.second->updateVisible();
    }
    while(hiz_num>0){
        RakSleep(5);
        hiz_solve_wake();
    }
    hizEnd();

    int left = rmtQueue.size();
    if(left==0){
        chunkLeft=0;
        if(chunkLeft_text){
            chunkLeft_text->remove();
            chunkLeft_text=NULL;
        }
    }else{
        if(chunkLeft!=left){
            chunkLeft=left;
            if(chunkLeft_text){
                chunkLeft_text->remove();
            }
            wchar_t buf[256];
            swprintf(buf,256,L"%d terrain item chunks left for generate",left);
            chunkLeft_text=gui->addStaticText(buf,irr::core::rect<irr::s32>(0,height-90,256,height),false,false);
            chunkLeft_text->setOverrideColor(irr::video::SColor(255,255,255,255));
            chunkLeft_text->setOverrideFont(font);
        }
    }
}

void terrain_item::onDraw(){
    terrain::onDraw();
    if(fullmap_gui->isVisible()){

        auto posi = camera->getPosition();
        wchar_t buf[256];
        swprintf(buf,256,L"位置：（%d , %d , %d）",(int)posi.X,(int)posi.Y,(int)posi.Z);
        ttf->draw(buf,irr::core::rect<irr::s32>(width-256,64,width,64+32),irr::video::SColor(255,255,255,255));

        int cx = floor(posi.X / 32);
        int cy = floor(posi.Z / 32);
        swprintf(buf,256,L"chunk：（%d , %d）",cx,cy);
        ttf->draw(buf,irr::core::rect<irr::s32>(width-256,64+32,width,64+64),irr::video::SColor(255,255,255,255));

        findChunk(cx,cy){
            if(!it->second->owner.empty()){
                if(it->second->owner==myUUID){
                    swprintf(buf,256,L"已占领（按F9放弃，按F10添加任务）");
                    ttf->draw(buf,irr::core::rect<irr::s32>(width-256,128,width,128+32),irr::video::SColor(255,255,255,255));
                }else{
                    swprintf(buf,256,L"拥有者：%s",it->second->owner.c_str());
                    ttf->draw(buf,irr::core::rect<irr::s32>(width-256,128,width,128+32),irr::video::SColor(255,255,255,255));
                }
            }else{
                swprintf(buf,256,L"当前位置无人占领（按F9占领）");
                ttf->draw(buf,irr::core::rect<irr::s32>(width-256,128,width,128+32),irr::video::SColor(255,255,255,255));
            }

            if(it->second->allowBuilding){
                swprintf(buf,256,L"允许建造");
            }else{
                swprintf(buf,256,L"禁止建造");
            }
            ttf->draw(buf,irr::core::rect<irr::s32>(width-256,160,width,160+32),irr::video::SColor(255,255,255,255));

            if(it->second->allowColl){
                swprintf(buf,256,L"允许采集");
            }else{
                swprintf(buf,256,L"禁止采集");
            }
            ttf->draw(buf,irr::core::rect<irr::s32>(width-256,192,width,192+32),irr::video::SColor(255,255,255,255));

            if(it->second->allowAttack){
                swprintf(buf,256,L"允许攻击");
            }else{
                swprintf(buf,256,L"禁止攻击");
            }
            ttf->draw(buf,irr::core::rect<irr::s32>(width-256,224,width,224+32),irr::video::SColor(255,255,255,255));

            terrmapacl_1->setVisible(true);
            terrmapacl_2->setVisible(true);
            terrmapacl_3->setVisible(true);

        }else{
            swprintf(buf,256,L"找不到坐标信息");
            ttf->draw(buf,irr::core::rect<irr::s32>(width-256,128,width,128+32),irr::video::SColor(255,255,255,255));
        }
        if(time(0)-showText_time<3){
            ttf->draw(showingText.c_str(),irr::core::rect<irr::s32>(width-256,height-128-32,width,height-128),irr::video::SColor(255,255,255,255));
        }
    }
}
void terrain_item::releaseChunk(chunk * ch){
    ch->unlink();
    releaseChildren(ch);
    chunks.erase(ipair(ch->x,ch->y));
    delete ch;
}
void terrain_item::releaseAllChunk(){
    for(auto it:chunks){
        releaseChildren(it.second);
        delete it.second;
    }
    chunks.clear();
}
void terrain_item::releaseChildren(chunk * ch){
    if(ch->minimap_element){
        ch->minimap_element->remove();
        ch->minimap_element = NULL;
    }
    for(auto it:ch->children){
        releaseTerrainItem(it.second);
    }
    ch->children.clear();
}
void terrain_item::removeTerrainItem(chunk * ch,int index,int id){
    auto it = ch->children.find(mapItem(index,id));
    if(it!=ch->children.end()){
        releaseTerrainItem(it->second);
        ch->children.erase(it);
    }
}
void terrain_item::removeTerrainItem(int x , int y ,int index,int id){
    findChunk(x,y){
        removeTerrainItem(it->second , index , id);
    }
}
void terrain_item::releaseTerrainItems(int x , int y){
    findChunk(x,y){
        releaseChildren(it->second);
        delete it->second;
        chunks.erase(it);
    }
}
terrain_item::item * terrain_item::makeTerrainItem(int id,int index,float x,float y,float r){
    float realHeight = getRealHight(x,y);
    if(id<0){
        if(id==-1){//树叶
            if(realHeight<waterLevel)
                return NULL;
            auto res = new item;
            res->id.cx=floor(x/32.f);
            res->id.cy=floor(y/32.f);
            res->id.x =x;
            res->id.y =y;
            res->id.id.id=id;
            res->id.id.index=index;
            btTriangleMesh * mesh;
            irr::scene::SMesh * shadowMesh1,* shadowMesh2;
            res->node[0]=genTree(x * y + index + id, shadowMesh1 , shadowMesh2 , mesh);
            res->bodyMesh = mesh;
            res->bodyShape = createShape(mesh);
            res->node[0]->setMaterialFlag(irr::video::EMF_LIGHTING, true );
            res->node[0]->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true );
            vec3 pos(x,realHeight-4,y);
            res->position = pos;
            res->node[0]->setPosition(pos);
            res->node[0]->updateAbsolutePosition();//更新矩阵

            res->shadowNode = createShadowNode(shadowMesh1,0,-1,pos);
            shadowMesh1->drop();
            createShadowNode(shadowMesh2,res->shadowNode)->setMaterialTexture(0,texture_treeLeaves);
            shadowMesh2->drop();

            res->bodyState=setMotionState(res->node[0]->getAbsoluteTransformation().pointer());
            res->rigidBody =createBody(res->bodyShape,res->bodyState);
            res->rigidBody->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
            res->rigidBody->setFriction(0.7);
            res->rigidBody->setRestitution(0.1);
            res->info.type=BODY_TERRAIN_ITEM;//设置用户数据，指向mapId
            res->info.ptr=&res->id;
            res->rigidBody->setUserPointer(&(res->info));
            dynamicsWorld->addRigidBody(res->rigidBody);
            res->hideLodLevel=4;
            return res;
        }else if(id==-2){//草
            if(realHeight<waterLevel)
                return NULL;
            if(texture_grass.empty())
                return NULL;
            auto res = new item;
            res->id.cx=floor(x/32.f);
            res->id.cy=floor(y/32.f);
            res->id.x =x;
            res->id.y =y;
            res->id.id.id=id;
            res->id.id.index=index;
            res->useHiZ = true;
            irr::video::ITexture * tex;
            res->node[0]=genGrass(x * y + index + id,res->hideLodLevel , tex);
            res->node[0]->setMaterialFlag(irr::video::EMF_LIGHTING, true );
            res->node[0]->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true );
            vec3 pos(x,realHeight,y);
            res->position = pos;
            res->node[0]->setPosition(pos);
            res->node[0]->updateAbsolutePosition();//更新矩阵
            res->shadowNode = createShadowNode(mesh_grass,0,-1,pos);
            res->shadowNode->setMaterialTexture(0,tex);
            return res;
        }
        return NULL;
    }
    auto it = config.find(id);
    if(it==config.end())
        return NULL;

    auto res = new item;
    res->id.cx=floor(x/32.f);
    res->id.cy=floor(y/32.f);
    res->id.x =x;
    res->id.y =y;
    res->id.id.id=id;
    res->id.id.index=index;

    int ml=3;
    for(int i = 0;i<4;++i){
        if(it->second->mesh[i]==NULL){
            ml=i-1;
            if(ml<0)
                ml=0;
            break;
        }
        res->position = vec3(x,realHeight+it->second->deltaHeight,y);
        auto n = scene->addMeshSceneNode(
                    it->second->mesh[i],NULL,
                    -1,
                    res->position ,
                    vec3(0,r,0),
                    it->second->scale
                    );
        n->setMaterialFlag(irr::video::EMF_LIGHTING, true );
        n->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true );
        n->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);

        n->updateAbsolutePosition();//更新矩阵

        res->node[i]=n;

        if(it->second->useShader){
            n->setMaterialType((irr::video::E_MATERIAL_TYPE)it->second->shader);
        }
    }
    res->node[ml]->setVisible(true);//显示最低lod级别

    if(it->second->haveBody){
        res->bodyState=setMotionState(res->node[0]->getAbsoluteTransformation().pointer());//创建状态
        res->rigidBody=createBody(it->second->shape.compound,res->bodyState);//创建物体
        res->rigidBody->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);//设置碰撞模式

        res->info.type=BODY_TERRAIN_ITEM;//设置用户数据，指向mapId
        res->info.ptr=&res->id;
        res->rigidBody->setUserPointer(&(res->info));

        dynamicsWorld->addRigidBody(res->rigidBody);//添加物体
    }else{
        res->bodyState=NULL;
        res->rigidBody=NULL;
    }

    return res;
}

bool terrain_item::chunkCreated(int x, int y){
    auto it = chunks.find(ipair(x,y));
    if(it==chunks.end())
        return false;
    chunk * c = it->second;

    //检查上下左右
    if(c->nearx0==NULL)
        return false;
    if(c->nearx1==NULL)
        return false;
    if(c->neary0==NULL)
        return false;
    if(c->neary1==NULL)
        return false;

    if(c->nearx0->neary0==NULL)
        return false;
    if(c->nearx1->neary1==NULL)
        return false;
    if(c->neary0->nearx0==NULL)
        return false;
    if(c->neary1->nearx1==NULL)
        return false;

    return terrain::chunkCreated(x,y);
}

void terrain_item::occupy(int x, int y){
    findChunk(x,y){
        if(it->second->owner.empty()){//占领
            wchar_t buf[256];
            swprintf(buf,256,L"尝试占领：（%d , %d）",x,y);
            setTerrainMapMessage(buf);
            occupyChunk();
        }else if(it->second->owner == myUUID){//放弃
            wchar_t buf[256];
            swprintf(buf,256,L"尝试放弃：（%d , %d）",x,y);
            setTerrainMapMessage(buf);
            cmd_giveUpChunk(x,y);
        }
    }
}

void terrain_item::updateLOD(int x, int y, int lv){
    terrain::updateLOD(x,y,lv);
    findChunk(x,y){
        it->second->lodLevel = lv;
        if(lv==0){
            for(auto c:it->second->children){
                item * im = c.second;
                im->lodLevel = -1;
                //防止视野外的物体没有隐藏
                for(int i=0;i<4;++i){
                    if(im->node[i])
                        im->node[i]->setVisible(false);
                }
            }
        }else{
            for(auto c:it->second->children){
                item * im = c.second;
                im->lodLevel = -1;
                //for(int i=0;i<4;++i){
                //    if(im->node[i])
                //        im->node[i]->setVisible(false);
                //}
                if(lv<im->hideLodLevel){
                    for(int i=lv-1;i>=0;--i){
                        //反向遍历lod列表
                        if(im->node[i]){
                            //找到可用的最大lod
                            //im->node[i]->setVisible(true);
                            im->lodLevel = i;
                            break;
                        }
                    }
                }
            }
        }
    }
}

void terrain_item::linkChunk(terrain_item::chunk * c, int x, int y){
    {
        findChunk(x+1,y){
            c->nearx1 = it->second;
            it->second->nearx0 = c;
        }else{
            c->nearx1 = NULL;
        }
    }
    {
        findChunk(x-1,y){
            c->nearx0 = it->second;
            it->second->nearx1 = c;
        }else{
            c->nearx0 = NULL;
        }
    }
    {
        findChunk(x,y+1){
            c->neary1 = it->second;
            it->second->neary0 = c;
        }else{
            c->neary1 = NULL;
        }
    }
    {
        findChunk(x,y-1){
            c->neary0 = it->second;
            it->second->neary1 = c;
        }else{
            c->neary0 = NULL;
        }
    }
}
void terrain_item::releaseTerrainItem(item * p){
    if(p->rigidBody){
        dynamicsWorld->removeRigidBody(p->rigidBody);
        delete p->rigidBody;
    }
    if(p->bodyState)
        delete p->bodyState;
    if(p->bodyShape)
        delete p->bodyShape;
    if(p->bodyMesh)
        delete p->bodyMesh;
    for(int i = 0;i<4;++i){
        if(p->node[i] && (i==0 || p->node[i-1]!=p->node[i])){
            p->node[i]->removeAll();
            p->node[i]->remove();
        }
    }
    if(p->shadowNode)
        p->shadowNode->remove();
    delete p;
}
irr::scene::IMesh * createMapMesh(const irr::video::SColor & color){
    auto buffer = new irr::scene::SMeshBuffer();
    irr::video::S3DVertex v;
    v.Color = color;
    v.Normal.set(0,1,0);

    v.Pos.set(0,0,0);
    buffer->Vertices.push_back(v);
    v.Pos.set(0,0,32);
    buffer->Vertices.push_back(v);
    v.Pos.set(32,0,32);
    buffer->Vertices.push_back(v);
    v.Pos.set(32,0,0);
    buffer->Vertices.push_back(v);

    buffer->Indices.push_back(0);
    buffer->Indices.push_back(1);
    buffer->Indices.push_back(3);
    buffer->Indices.push_back(1);
    buffer->Indices.push_back(2);
    buffer->Indices.push_back(3);

    buffer->recalculateBoundingBox();

    auto mesh = new irr::scene::SMesh();
    mesh->addMeshBuffer(buffer);
    mesh->setHardwareMappingHint(irr::scene::EHM_STATIC);
    mesh->recalculateBoundingBox();
    mesh->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true );
    buffer->drop();

    return mesh;
}
terrain_item::terrain_item(){
    chunkLeft = 0;
    chunkLeft_text = NULL;
    showText_time = 0;
    mapBuffer = new float[33*33];
    loadConfig();

    {
        minimap_terrain_mesh_owned = createMapMesh(irr::video::SColor(128,16,64,32));
        minimap_terrain_mesh[0][0][0] = createMapMesh(irr::video::SColor(128,255,255,64));
        minimap_terrain_mesh[0][0][1] = createMapMesh(irr::video::SColor(128,255,255,255));
        minimap_terrain_mesh[0][1][0] = createMapMesh(irr::video::SColor(128,255,64,64));
        minimap_terrain_mesh[0][1][1] = createMapMesh(irr::video::SColor(128,255,64,255));
        minimap_terrain_mesh[1][0][0] = createMapMesh(irr::video::SColor(128,64,255,64));
        minimap_terrain_mesh[1][0][1] = createMapMesh(irr::video::SColor(128,64,255,255));
        minimap_terrain_mesh[1][1][0] = createMapMesh(irr::video::SColor(128,64,64,64));
        minimap_terrain_mesh[1][1][1] = createMapMesh(irr::video::SColor(128,64,64,255));
        minimap_terrain_shader = driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                                     "../shader/minimap_terrain.vs.glsl", "main", irr::video::EVST_VS_1_1,
                                     "../shader/minimap_terrain.ps.glsl", "main", irr::video::EPST_PS_1_1);
    }

    //树纹理
    texture_treeTrunk = driver->getTexture("../../res/tree_trunk.tga");
    texture_treeLeaves = driver->getTexture("../../res/tree_leaves.tga");
    shader_tree = driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                            "../shader/tree.vs.glsl", "main", irr::video::EVST_VS_1_1,
                            "../shader/tree.ps.glsl", "main", irr::video::EPST_PS_1_1,&defaultCallback);
    //草纹理
    auto fp = fopen("../config/grass.txt","r");
    if(fp){
        char buf[512];
        while(!feof(fp)){
            bzero(buf,sizeof(buf));
            fgets(buf,sizeof(buf),fp);
            if(buf[0]!='\0'){
                auto str = QString(buf).trimmed().toStdString();
                auto tex = driver->getTexture(str.c_str());
                if(tex)
                    texture_grass.push_back(tex);
            }
        }
    }
    //草模型
    genGrassMesh();
    //权限设置
    {
        terrmapacl_1 = gui->addCheckBox(true,irr::core::rect<irr::s32>(width-190,160,width-190+30,160+30));
        terrmapacl_1->setVisible(false);
        terrmapacl_1->setEnabled(false);
        terrmapacl_2 = gui->addCheckBox(true,irr::core::rect<irr::s32>(width-190,192,width-190+30,192+30));
        terrmapacl_2->setVisible(false);
        terrmapacl_2->setEnabled(false);
        terrmapacl_3 = gui->addCheckBox(true,irr::core::rect<irr::s32>(width-190,224,width-190+30,224+30));
        terrmapacl_3->setVisible(false);
        terrmapacl_3->setEnabled(false);
        terrmapacl_save = gui->addButton(irr::core::rect<irr::s32>(width-160,224,width-100,224+30),0,-1,L"SAVE");
        terrmapacl_save->setVisible(false);
        terrmapacl_save->setOverrideColor(video::SColor(255,0,0,0));
    }

    //hiz线程
    hiz_num = 0;
    for(int i=0;i<4;++i){
        std::thread st([&](terrain_item * self){
            printf("[status]create hiz thread\n");
            self->hiz_solve_mainThread();
        },this);
        st.detach();
    }
}

void terrain_item::setFullMapMode(bool m){
    terrain::setFullMapMode(m);
    if(m){
        auto posi = camera->getPosition();
        int cx = floor(posi.X / 32);
        int cy = floor(posi.Z / 32);
        findChunk(cx,cy){
            terrmapacl_1->setChecked(it->second->allowBuilding);
            terrmapacl_2->setChecked(it->second->allowColl);
            terrmapacl_3->setChecked(it->second->allowAttack);

            bool enable = (it->second->owner==myUUID);
            terrmapacl_1->setEnabled(enable);
            terrmapacl_2->setEnabled(enable);
            terrmapacl_3->setEnabled(enable);
            if(enable){
                terrmapacl_save->setVisible(true);
            }
        }
    }else{
        terrmapacl_1->setVisible(false);
        terrmapacl_2->setVisible(false);
        terrmapacl_3->setVisible(false);
        terrmapacl_save->setVisible(false);
    }
}
void terrain_item::uploadChunkACL(){
    setTerrainMapMessage(L"上传权限配置");
    auto posi = camera->getPosition();
    int cx = floor(posi.X / 32);
    int cy = floor(posi.Z / 32);
    bool b = terrmapacl_1->isChecked();
    bool c = terrmapacl_3->isChecked();
    bool t = terrmapacl_2->isChecked();
    cmd_setChunkACL(cx,cy,b,c,t);
}
terrain_item::~terrain_item(){
    releaseAllChunk();
    releaseConfig();
    mesh_grass->drop();
    hiz_solve_wake();
    delete [] mapBuffer;
}
void terrain_item::loadConfig(){
    config.clear();
    //读取文件
    char* text;
    FILE *pf = fopen("../config/terrainItem.json","r");
    if(pf==NULL){
        printf("[error]terrain_item:load config fail\n");
        return;
    }
    fseek(pf,0,SEEK_END);
    long lSize = ftell(pf);
    if(lSize<=0){
        fclose(pf);
        printf("[error]terrain_item:load config fail\n");
        return;
    }
    text=(char*)malloc(lSize+1);// 用完后需要将内存free掉
    rewind(pf);
    fread(text,sizeof(char),lSize,pf);
    text[lSize] = '\0';
    fclose(pf);
    //解析json
    cJSON * json=cJSON_Parse(text);
    if(json){
        if(json->type==cJSON_Array){
            cJSON *c=json->child;//遍历
            while (c){
                if(c->type==cJSON_Object){
                    loadJSON(c);
                }
                c=c->next;
            }
        }
        cJSON_Delete(json);
    }
    free(text);
}
void terrain_item::loadJSON(cJSON * json){
    auto id = cJSON_GetObjectItem(json,"id");
    if(id && id->type==cJSON_Number){
        auto it = config.find(id->valueint);
        if(it!=config.end())//已经存在
            return;
    }else
        return;

    auto path = cJSON_GetObjectItem(json,"mesh");

    irr::scene::IMesh * mesh;
    if(path && path->type==cJSON_String){
        mesh = scene->getMesh(path->valuestring);
        if(mesh==NULL)
            return;
    }else
        return;

    auto c          = new conf;
    c->haveBody     = false;
    c->deltaHeight  = 0;
    c->scale.set(1,1,1);
    c->useShader    = false;
    c->flat         = false;
    for(int i = 0;i<4;++i)
        c->mesh[i] = NULL;

    c->mesh[0] = mesh;

    auto meshv2_j = cJSON_GetObjectItem(json,"meshv2");
    if(meshv2_j && meshv2_j->type==cJSON_String){
        auto meshv2 = scene->getMesh(meshv2_j->valuestring);
        if(meshv2){
            c->mesh[1] = meshv2;
            auto meshv3_j = cJSON_GetObjectItem(json,"meshv3");
            if(meshv3_j && meshv3_j->type==cJSON_String){
                auto meshv3 = scene->getMesh(meshv3_j->valuestring);
                if(meshv3){
                    c->mesh[2] = meshv3;
                    auto meshv4_j = cJSON_GetObjectItem(json,"meshv4");
                    if(meshv4_j && meshv4_j->type==cJSON_String){
                        auto meshv4 = scene->getMesh(meshv4_j->valuestring);
                        if(meshv4){
                            c->mesh[3] = meshv4;
                        }
                    }
                }
            }
        }
    }

    auto body = cJSON_GetObjectItem(json,"body");
    if(body && body->type==cJSON_String){
        c->haveBody = true;
        c->shape.init(body->valuestring);
    }

    auto deltaHeight = cJSON_GetObjectItem(json,"deltaHeight");
    if(deltaHeight && deltaHeight->type==cJSON_Number){
        c->deltaHeight = deltaHeight->valuedouble;
    }

    auto scale = cJSON_GetObjectItem(json,"scale");
    if(scale && scale->type==cJSON_Object){

        auto sx = cJSON_GetObjectItem(scale , "x");
        if(sx && sx->type==cJSON_Number)c->scale.X = sx->valuedouble;

        auto sy = cJSON_GetObjectItem(scale , "y");
        if(sy && sy->type==cJSON_Number)c->scale.Y = sy->valuedouble;

        auto sz = cJSON_GetObjectItem(scale , "z");
        if(sz && sz->type==cJSON_Number)c->scale.Z = sz->valuedouble;

    }

    auto ft = cJSON_GetObjectItem(json,"flat");
    if(ft && ft->type==cJSON_Number){
        c->flat = (ft->valueint!=0);
    }

    auto shader = cJSON_GetObjectItem(json,"shader");
    if(shader && shader->type==cJSON_Object){
        auto vs = cJSON_GetObjectItem(shader,"vs");
        auto ps = cJSON_GetObjectItem(shader,"ps");
        if(vs && ps && vs->type==cJSON_String && ps->type==cJSON_String){
            c->useShader = true;
            c->shader = driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
                        vs->valuestring, "main", irr::video::EVST_VS_1_1,
                        ps->valuestring, "main", irr::video::EPST_PS_1_1,&defaultCallback);
        }
    }

    config[id->valueint] = c;
}
void terrain_item::releaseConfig(){
    for(auto it:config){
        //it.second->mesh->drop();
        delete it.second;
    }
    config.clear();
}
void terrain_item::msg_addRemovedItem(int x,int y,int id,int index){
    removeTerrainItem(x,y,id,index);
}
void terrain_item::msg_setRemovedItem(int x,int y,const std::set<mapItem> & rmt){
    pushRemoveTable(x,y,rmt);
}

void terrain_item::msg_chunkACL(int32_t x, int32_t y, bool b, bool c, bool t,const char * owner){
    findChunk(x,y){
        auto ptr = it->second;
        if(ptr){
            if(ptr->minimap_element)
                ptr->minimap_element->remove();
            ptr->owner = owner;

            ptr->allowBuilding = b;
            ptr->allowAttack = c;
            ptr->allowColl = t;

            auto n = mapScene->addMeshSceneNode(minimap_terrain_mesh[b?1:0][c?1:0][t?1:0],0,-1,vec3(x*32,0,y*32));
            n->getMaterial(0).BlendOperation=irr::video::EBO_ADD;
            n->setMaterialFlag(irr::video::EMF_LIGHTING, false );
            n->setMaterialType((video::E_MATERIAL_TYPE)minimap_terrain_shader);
            ptr->minimap_element = n;

            if(owner == myUUID){
                n = mapScene->addMeshSceneNode(minimap_terrain_mesh_owned,n);
                n->getMaterial(0).BlendOperation=irr::video::EBO_ADD;
                n->setMaterialFlag(irr::video::EMF_LIGHTING, false );
                n->setMaterialType((video::E_MATERIAL_TYPE)minimap_terrain_shader);
            }
        }
    }
}

void terrain_item::chunk::updateVisible(){
    if(lodLevel>0){
        for(auto i : children){
            parent->hiz_queue_lock.lock();
            //i.second->updateVisible();
            parent->hiz_queue.push(i.second);
            ++parent->hiz_num;
            parent->hiz_queue_lock.unlock();
            parent->hiz_solve_wake();
        }
    }
}

void terrain_item::chunk::unlink(){
    if(nearx0){
        nearx0->nearx1 = NULL;
        nearx0 = NULL;
    }
    if(nearx1){
        nearx1->nearx0 = NULL;
        nearx1 = NULL;
    }
    if(neary0){
        neary0->neary1 = NULL;
        neary0 = NULL;
    }
    if(neary1){
        neary1->neary0 = NULL;
        neary1 = NULL;
    }
}
bool terrain_item::isMyChunk(int x, int y){
    findChunk(x,y){
        if(it->second->owner == myUUID)
            return true;
    }
    return false;
}

void terrain_item::hiz_solve_mainThread(){
    while(running){
        item * it = NULL;
        hiz_queue_lock.lock();
        if(!hiz_queue.empty()){
            it = hiz_queue.front();
            hiz_queue.pop();
        }
        hiz_queue_lock.unlock();

        if(it){
            it->updateVisible();
            --hiz_num;
        }else{
            hiz_solve_wait();
        }
    }
}

void terrain_item::createChunk(int x, int y){
    terrain::createChunk(x,y);
    auto it = rmtCache.find(ipair(x,y));
    if(it!=rmtCache.end()){
        if(setRemoveTable(x,y,it->second,false)){
            rmtCache.erase(it);
        }
    }
}

void terrain_item::item::updateVisible(){
    if(!useHiZ || parent->parent->pointVisible(position)){
        for(int i=0;i<4;++i){
            if(node[i])
                node[i]->setVisible(i==lodLevel);
        }
    }else{
        for(int i=0;i<4;++i){
            if(node[i])
                node[i]->setVisible(false);
        }
    }
}

}
