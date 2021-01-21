#include "body.h"
namespace smoothly{

void body::loop(){
    mission::loop();
    for(auto it:bodies){
        bodyItem * b = it.second;
        auto p = b->node->getPosition();
        auto cp=camera->getPosition();
        int l = std::max(abs((p.X-cp.X)/32),abs((p.Z-cp.Z)/32));
        if(l<getVisualRange()){
            if(mainControlBody==b){
                b->node->setVisible(false);
            }else{
                b->node->setVisible(true);
            }
        }else{
            b->node->setVisible(false);
        }
        if(b->uncreatedChunk){
            int cx = floor(b->lastPosition.X/32);
            int cy = floor(b->lastPosition.Z/32);
            if(chunkCreated(cx,cy)){
                b->m_character.setGravity(gravity);
                b->uncreatedChunk = false;
            }else{
                b->m_character.setPosition(b->lastPosition);
                b->updateFromWorld();
                continue;
            }
        }
        b->walk(b->status.walk_forward , b->status.walk_leftOrRight,b->config->walkVelocity);
        b->updateFromWorld();
        auto h = getRealHight(p.X,p.Z)-1024;
        if(p.Y<h){
            if(b->lastPosition.Y>=h){//挖掘的上限是1024
                p.Y = b->lastPosition.Y;
            }else{
                p.Y = (b->config->height+b->config->width)*0.5+h;//防止掉出地图
            }
            b->m_character.setPosition(p);
        }
    }
    for(auto it:myBodies){
        bodyItem * b = it.second;
        if(b->owner == myUUID && (!myUUID.empty())){//是自己拥有的

            //设置行走
            //b->walk(b->status.walk_forward , b->status.walk_leftOrRight,b->config->walkVelocity);
            //转移到上面了
            //b->walk(b->status.walk_forward , b->status.walk_leftOrRight,b->config->walkVelocity*timer->getRealTime());

            //设置dispather
            btTransform transform;
            b->m_character.getTransform(transform);

            btVector3 btPos;
            btPos = transform.getOrigin();
            setCharacterChunk(b->uuid,floor(btPos.x()/32),floor(btPos.z()/32));

            if(b->firing)
                b->doFire();

            //ai控制
            if(mainControlBody!=b){
                commond cmd;//命令结构体
                cmd.uuid = b->uuid;

                vec3 posi(btPos.x(), btPos.y(), btPos.z());
                {
                    //获取碰撞列表
                    b->behaviorStatus.statusInit();
                    try{
                        auto & overlap = b->m_character.m_ghostObject->getOverlappingPairs();
                        auto len = overlap.size();
                        for(int i = 0;i<len;++i){
                            auto op = ((bodyInfo*)(overlap.at(i)->getUserPointer()));
                            if(op->type==BODY_BODY_PART){
                                auto c = (building::character*)op->ptr;
                                if(c){
                                    auto bdp = (bodyInfo*)c->m_ghostObject->getUserPointer();
                                    if(bdp->type==BODY_BODY){
                                        auto bd = (bodyItem*)bdp->ptr;
                                        if(bd!=b){
                                            b->behaviorStatus.hitBody = true;
                                        }
                                    }
                                }
                            }else if(op->type==BODY_BUILDING){
                                b->behaviorStatus.hitBuilding = true;
                            }else if(op->type==BODY_TERRAIN){
                                b->behaviorStatus.onFloor = true;
                            }else if(op->type==BODY_TERRAIN_ITEM){
                                b->behaviorStatus.hitTerrainItem = true;
                            }
                        }
                    }catch(...){}
                    b->behaviorStatus.position = posi;
                }
                {
                    if(b->follow != NULL){
                        //跟随模式
                        bodyItem * bd = b->follow;
                        auto target  = bd->node->getPosition();
                        b->behaviorStatus.followTarget = target;
                        b->behaviorStatus.haveFollow = true;
                    }
                }
                {
                    while(!b->autoWalk.empty()){
                        auto target = b->autoWalk.front();
                        vec3 dir = target-posi;
                        irr::core::vector2df tdir(dir.X,dir.Z);
                        if(tdir.getLengthSQ()<1){
                            b->autoWalk.pop_front();//到达目标，删除任务
                        }else{
                            b->behaviorStatus.pathFindingTarget = target;
                            b->behaviorStatus.pathFindingMode = true;
                            break;
                        }
                    }
                }
                //调用AI脚本
                b->AIExec();
                b->behaviorStatus.edge();
            }
        }
    }
    doCommonds();

    selectBodyUpdate();

    if(needUpdateUI){
        updateBagUI();
    }
}

void body::onDraw(){
    mission::onDraw();
    for(auto it:myBodies){//在屏幕上标出自己拥有的单位
        bodyItem * bd = it.second;
        if(bd!=mainControlBody){
            auto p = scene->getSceneCollisionManager()->getScreenCoordinatesFrom3DPosition(bd->node->getPosition(),camera);
            bd->screenPosition = p;
            if(p.X<0 || p.Y<0 || p.X>width || p.Y>height)
                continue;
            driver->draw2DLine(irr::core::vector2d<irr::s32>(p.X-2,p.Y),irr::core::vector2d<irr::s32>(p.X+2,p.Y),irr::video::SColor(255,64,255,255));
            driver->draw2DLine(irr::core::vector2d<irr::s32>(p.X-2,p.Y),irr::core::vector2d<irr::s32>(p.X,p.Y+5),irr::video::SColor(255,64,255,255));
            driver->draw2DLine(irr::core::vector2d<irr::s32>(p.X+2,p.Y),irr::core::vector2d<irr::s32>(p.X,p.Y+5),irr::video::SColor(255,64,255,255));
            {
                float hp = bd->hp;
                float maxhp = bd->config->hp;
                if(hp<maxhp){//不是满血的标出血量
                    auto pr = hp/maxhp;
                    if(pr>0.f){
                        if(pr>1.f)
                            pr = 1.f;
                        int wsize = pr*16;
                        if(wsize>0){
                            driver->draw2DLine(
                                        irr::core::vector2d<irr::s32>(p.X - wsize, p.Y -25),
                                        irr::core::vector2d<irr::s32>(p.X + wsize, p.Y -25),
                                        irr::video::SColor(255,64,255,64));
                        }
                    }
                }
            }
            if(!bd->autoWalk.empty()){
            #define dig120 (3.14159265358979323846*2)/3
            #define inr    19
            #define our    25
                double t=(std::clock()*500.0/ CLOCKS_PER_SEC)/100.0;
                driver->draw2DLine(
                            irr::core::vector2d<irr::s32>(p.X + (inr*cos(-t)), p.Y + (inr*sin(-t))),
                            irr::core::vector2d<irr::s32>(p.X + (our*cos(-t)), p.Y + (our*sin(-t))),
                            irr::video::SColor(255,255,255,255));
                driver->draw2DLine(
                            irr::core::vector2d<irr::s32>(p.X + (inr*cos(-t+dig120)), p.Y + (inr*sin(-t+dig120))),
                            irr::core::vector2d<irr::s32>(p.X + (our*cos(-t+dig120)), p.Y + (our*sin(-t+dig120))),
                            irr::video::SColor(255,255,255,255));
                driver->draw2DLine(
                            irr::core::vector2d<irr::s32>(p.X + (inr*cos(-t-dig120)), p.Y + (inr*sin(-t-dig120))),
                            irr::core::vector2d<irr::s32>(p.X + (our*cos(-t-dig120)), p.Y + (our*sin(-t-dig120))),
                            irr::video::SColor(255,255,255,255));
            #undef our
            #undef inr
            #undef dig120
            }
        }
    }
    for(auto it:selectedBodies){
        bodyItem * bd = it;
        auto p = bd->screenPosition;
        if(bd==mainControlBody)
            continue;
        if(p.X<0 || p.Y<0 || p.X>width || p.Y>height)
            continue;

        #define dig120 (3.14159265358979323846*2)/3
        #define inr    7
        #define our    19
        double t=(std::clock()*500.0/ CLOCKS_PER_SEC)/100.0;
        driver->draw2DLine(
                    irr::core::vector2d<irr::s32>(p.X + (inr*cos(t)), p.Y + (inr*sin(t))),
                    irr::core::vector2d<irr::s32>(p.X + (our*cos(t)), p.Y + (our*sin(t))),
                    irr::video::SColor(255,255,255,64));
        driver->draw2DLine(
                    irr::core::vector2d<irr::s32>(p.X + (inr*cos(t+dig120)), p.Y + (inr*sin(t+dig120))),
                    irr::core::vector2d<irr::s32>(p.X + (our*cos(t+dig120)), p.Y + (our*sin(t+dig120))),
                    irr::video::SColor(255,255,255,64));
        driver->draw2DLine(
                    irr::core::vector2d<irr::s32>(p.X + (inr*cos(t-dig120)), p.Y + (inr*sin(t-dig120))),
                    irr::core::vector2d<irr::s32>(p.X + (our*cos(t-dig120)), p.Y + (our*sin(t-dig120))),
                    irr::video::SColor(255,255,255,64));
        #undef our
        #undef inr
        #undef dig120
    }

    if(selecting){
        driver->draw2DPolygon(screenCenter,selectBodyRange,irr::video::SColor(255,255,255,255),36);
    }
    if(mainControlBody && mainControlBody->reloading){//换弹进度条
        auto ntm = timer->getTime();
        auto delta = ntm - mainControlBody->reloadStartTime;
        float prog = ((float)delta)/((float)mainControlBody->reloadNeedTime);
        drawArcProgressBar(screenCenter,128,irr::video::SColor(255,255,255,255),128,prog);
    }
    //血条
    if(mainControlBody){
        float hp = mainControlBody->hp;
        float maxhp = mainControlBody->config->hp;
        auto pr = hp/maxhp;
        if(pr>0.f){
            if(pr>1.f)
                pr = 1.f;
            int wsize = pr*256;
            if(wsize>0){
                int centerw = width*0.5;
                irr::core::rect<irr::s32> des(centerw-wsize , 0 , centerw+wsize , 64);
                irr::core::rect<irr::s32> res(256-wsize , 0 , 256+wsize , 64);
                driver->draw2DImage(texture_hp , des , res , 0 , 0 , true);
            }
        }
    }

    /*
     * 坐标轴及光源
    auto px = scene->getSceneCollisionManager()->getScreenCoordinatesFrom3DPosition(camera->getPosition()+vec3(100,0,0),camera);
    auto py = scene->getSceneCollisionManager()->getScreenCoordinatesFrom3DPosition(camera->getPosition()+vec3(0,0,100),camera);
    auto pl = scene->getSceneCollisionManager()->getScreenCoordinatesFrom3DPosition(light->getPosition(),camera);
    driver->draw2DLine(
                px,
                px+irr::core::vector2di(0,30),
                irr::video::SColor(255,255,0,0));
    driver->draw2DLine(
                py,
                py+irr::core::vector2di(0,30),
                irr::video::SColor(255,0,255,255));
    driver->draw2DLine(
                pl,
                pl+irr::core::vector2di(0,30),
                irr::video::SColor(255,255,255,255));
    */
}

}
