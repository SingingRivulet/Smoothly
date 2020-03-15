#include "fire.h"

namespace smoothly{

fire::fire(){
    openConfig();
    L = luaL_newstate();
    luaL_openlibs(L);
    luaL_dofile(L, "../script/fire.lua");
}

fire::~fire(){
    for(auto b:bullets){
        b->node->removeAll();
        b->node->remove();
        dynamicsWorld->removeRigidBody(b->rigidBody);
        delete b->rigidBody;
        delete b->bodyState;
        bullets.erase(b);
        delete b;
    }
    for(auto it:emitters){
        delete it;
    }
    lua_close(L);
    closeConfig();
}

void fire::fireTo(const std::string & uuid , int id , const vec3 & from , const vec3 & dir){
    cmd_fire(uuid,id,
             from.X , from.Y , from.Z,
             dir.X  , dir.Y  , dir.Z);
    fireTo_act(uuid,id,from,dir,true);
}
void fire::msg_fire(const char * uuid,int id,float fx,float fy,float fz,float dx,float dy,float dz){
    auto p = seekBody(uuid);
    if(p){
        if(!myUUID.empty() && p->owner==myUUID)//自己拥有的body不接受来自服务器的消息
            return;
        fireTo_act(uuid,id,
                   vec3(fx,fy,fz),
                   vec3(dx,dy,dz));
    }
}

void fire::fireTo_act(const std::string & uuid , int id , const vec3 & from , const vec3 & dir,bool attack){
    auto it = config.find(id);
    if(it==config.end())
        return;
    fireConfig * conf = it->second;
    if(conf==NULL)
        return;

    if(conf->type==FIRE_SHOOT){
        shoot(uuid,conf,from,dir,attack);
    }else
    if(conf->type==FIRE_STREAM){
        auto em     = new emitter;//创建一个emitter
        em->config  = conf;
        em->leave   = conf->streamParticleNum;
        em->uuid    = uuid;
        em->lastProcess = timer->getRealTime();
        em->attack  = attack;
        emitters.insert(em);
    }else
    if(conf->type==FIRE_CHOP || conf->type==FIRE_RADIO){

        if(attack && conf->castShape){

            btTransform bfrom,bto;
            auto rotate=dir.getHorizontalAngle();//旋转角度
            irr::core::matrix4 matF,matT;//从矩阵F到矩阵T
            matF.setRotationDegrees(rotate);
            matT=matF;
            matF.setTranslation(from);
            matT.setTranslation(from+dir);
            bfrom.setFromOpenGLMatrix(matF.pointer());
            bto.setFromOpenGLMatrix(matT.pointer());
            class closeCombCallback:public btCollisionWorld::ConvexResultCallback{//定义一个bt的回调函数类
                public:
                    fire * parent;
                    fireConfig * attconf;
                    std::string uuid;
                    virtual	btScalar addSingleResult(btCollisionWorld::LocalConvexResult & convexResult,bool){
                        auto ptr = (bodyInfo*)convexResult.m_hitCollisionObject->getUserPointer();
                        parent->attackBody(uuid , attconf , ptr);
                        return convexResult.m_hitFraction;
                    }
            };
            closeCombCallback callback;
            callback.parent     = this;
            callback.attconf    = conf;
            callback.uuid       = uuid;
            //使用convexSweepTest实现
            dynamicsWorld->convexSweepTest(
                        conf->castShape,
                        bfrom,
                        bto,
                        callback);

        }
        if(conf->type==FIRE_CHOP){
            //这个不知道怎么写，暂时留空
        }else
        if(conf->type==FIRE_RADIO){

            //创建粒子
            if(conf->particleConfig.have){
                auto ps = scene->addParticleSystemSceneNode(false);
                auto em = ps->createPointEmitter(
                            vec3(0,1,0),//垂直向上发射
                            conf->particleConfig.minParticlesPerSecond  ,   conf->particleConfig.maxParticlesPerSecond,
                            conf->particleConfig.minStartColor          ,   conf->particleConfig.maxStartColor,
                            conf->particleConfig.lifeTimeMin            ,   conf->particleConfig.lifeTimeMax,
                            conf->particleConfig.maxAngleDegrees,
                            conf->particleConfig.minStartSize           ,   conf->particleConfig.maxStartSize
                            );
                ps->setEmitter(em);//粒子系统设置发射器
                em->drop();

                ps->setParticlesAreGlobal(false);
                ps->setMaterialFlag(irr::video::EMF_LIGHTING, conf->particleConfig.light);
                ps->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true);
                if(conf->particleConfig.texture){
                    ps->setMaterialTexture(0, conf->particleConfig.texture); // fireball
                    //ps->setMaterialType(video::EMT_TRANSPARENT_VERTEX_ALPHA);
                    ps->setMaterialType(irr::video::EMT_TRANSPARENT_ADD_COLOR);
                }
                if(conf->particleConfig.gravity.have){
                    auto af = ps->createGravityAffector(conf->particleConfig.gravity.gravity,conf->particleConfig.gravity.timeForceLost);
                    ps->addAffector(af);
                    af->drop();
                }
                auto dp = scene->createDeleteAnimator(conf->lifeTime);
                ps->addAnimator(dp);//自动删除
                dp->drop();
            }

        }

    }else
    if(conf->type==FIRE_LASER){

        auto to=from+dir;
        vec3 tdir = dir;
        btVector3 bfrom(from.X,from.Y,from.Z),bto(to.X,to.Y,to.Z);//转换为bullet向量
        btCollisionWorld::ClosestRayResultCallback rayCallback(bfrom,bto);
        dynamicsWorld->rayTest(bfrom, bto, rayCallback);//使用bullet的rayTest接口
        if (rayCallback.hasHit()){
            //攻击到物体
            //被攻击的物体是m_collisionObject
            //激光终点是m_hitPointWorld
            vec3 targ(
                            rayCallback.m_hitPointWorld.getX(),
                            rayCallback.m_hitPointWorld.getY(),
                            rayCallback.m_hitPointWorld.getZ());
            tdir=targ-from;
        }
        if(attack){
            auto ptr = (bodyInfo*)rayCallback.m_collisionObject->getUserPointer();
            attackBody(uuid , conf , ptr);
        }

        //创建十字交叉的两个矩形，包上粒子来作为激光
        {
            auto node   = scene->addEmptySceneNode();//创建主节点
            auto length = tdir.getLength();
            vec3 center(0,0,length/2);

            auto geo  = scene->getGeometryCreator();//几何体绘制工具

            auto mesh = geo->createPlaneMesh(irr::core::dimension2d<irr::f32>(1,1));//创建一个正方形

            //创建一个水平的和一个垂直的矩形，贴上相同的纹理
            auto n1 = scene->addMeshSceneNode(
                        mesh,node,-1,
                        center,
                        vec3(0,0,0),//水平方向
                        vec3(conf->radius,conf->radius,length));
            n1->setMaterialFlag(irr::video::EMF_LIGHTING, false );//不受光照
            if(conf->bulletConf.texture)
                n1->setMaterialTexture( 0 , conf->bulletConf.texture);//纹理
            n1->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);//开启透明

            auto n2 = scene->addMeshSceneNode(
                        mesh,node,-1,
                        center,
                        vec3(0,0,90),//垂直方向
                        vec3(conf->radius,conf->radius,length));

            n2->setMaterialFlag(irr::video::EMF_LIGHTING, false );//不受光照
            if(conf->bulletConf.texture)
                n2->setMaterialTexture( 0 , conf->bulletConf.texture);//纹理
            n2->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);//开启透明

            mesh->drop();//mesh不再使用了

            if(conf->particleConfig.have){
                auto ps = scene->addParticleSystemSceneNode(false , node);
                auto em = ps->createCylinderEmitter(
                            center,
                            conf->radius,
                            vec3(0,0,1),
                            length,
                            false,
                            vec3(0,0,0.3),
                            conf->particleConfig.minParticlesPerSecond  ,   conf->particleConfig.maxParticlesPerSecond,
                            conf->particleConfig.minStartColor          ,   conf->particleConfig.maxStartColor,
                            conf->particleConfig.lifeTimeMin            ,   conf->particleConfig.lifeTimeMax,
                            conf->particleConfig.maxAngleDegrees,
                            conf->particleConfig.minStartSize           ,   conf->particleConfig.maxStartSize
                            );
                ps->setEmitter(em);//粒子系统设置发射器
                em->drop();

                ps->setParticlesAreGlobal(false);
                ps->setMaterialFlag(irr::video::EMF_LIGHTING, conf->particleConfig.light);
                ps->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true);
                if(conf->particleConfig.texture){
                    ps->setMaterialTexture(0, conf->particleConfig.texture); // fireball
                    //ps->setMaterialType(video::EMT_TRANSPARENT_VERTEX_ALPHA);
                    ps->setMaterialType(irr::video::EMT_TRANSPARENT_ADD_COLOR);
                }
                if(conf->particleConfig.gravity.have){
                    auto af = ps->createGravityAffector(conf->particleConfig.gravity.gravity,conf->particleConfig.gravity.timeForceLost);
                    ps->addAffector(af);
                    af->drop();
                }
            }

            //设置节点属性
            node->setPosition(from);//位置
            node->setRotation(dir.getHorizontalAngle());//旋转

            auto dp = scene->createDeleteAnimator(conf->lifeTime);
            node->addAnimator(dp);
            dp->drop();
        }
    }
}

}
