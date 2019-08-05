#include "attack.h"
namespace smoothly{
    
    void attackInter::attackAmLaser::add(//创建激光
        irr::scene::ISceneManager * scene,
        irr::scene::IAnimatedMeshSceneNode *,
        const irr::core::vector3df & from,
        const irr::core::vector3df & dir
    ){
        auto node = scene->addEmptySceneNode();//创建主节点
        
        auto length=dir.getLength();
        irr::core::vector3df center(0,0,length/2);
        
        auto geo  = scene->getGeometryCreator();//几何体绘制工具
        
        auto mesh = geo->createPlaneMesh(irr::core::dimension2d<irr::f32>(1,1));//创建一个正方形
        
        //创建一个水平的和一个垂直的矩形，贴上相同的纹理
        auto n1 = scene->addMeshSceneNode(
            mesh,node,-1,
            center,
            irr::core::vector3df(0,0,0),//水平方向
            irr::core::vector3df(1,1,length)
        );
        n1->setMaterialFlag(irr::video::EMF_LIGHTING, false );//不受光照
        n1->setMaterialTexture( 0 , this->texture);//纹理
        n1->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);//开启透明
        
        auto n2 = scene->addMeshSceneNode(
            mesh,node,-1,
            center,
            irr::core::vector3df(0,0,90),//垂直方向
            irr::core::vector3df(1,1,length)
        );
        n2->setMaterialFlag(irr::video::EMF_LIGHTING, false );//不受光照
        n2->setMaterialTexture( 0 , this->texture);//纹理
        n2->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);//开启透明
        
        mesh->drop();//mesh不再使用了
        
        //设置节点属性
        node->setPosition(from);//位置
        node->setRotation(dir.getHorizontalAngle());//旋转
        
        setVanishingTime(node,scene,this->delayMs);
    }
    
    void attackInter::attackAmCloseComb::add(//近战
        irr::scene::ISceneManager * scene,
        irr::scene::IAnimatedMeshSceneNode * n,
        const irr::core::vector3df & from,
        const irr::core::vector3df & dir
    ){
        auto particles = scene->addParticleSystemSceneNode(false,n);//创建粒子系统
        
        auto emitter   = particles->createPointEmitter(//创建粒子发射器
            irr::core::vector3df(0,0,particleSpeed),
            minParticlesPerSecond,
            maxParticlesPerSecond,
            minStartColor,
            maxStartColor,
            lifeTimeMin,
            lifeTimeMax,
            360//任意方向发射
        );
        particles->setEmitter(emitter);//设置发射器
        particles->setParticlesAreGlobal(false);//不受场景节点影响，实现挥砍拖尾效果
        emitter->drop();
        
        setVanishingTime(particles,scene,this->delayMs);//挥砍完后粒子发射器消失
    }
    
    void attackInter::attackAmShot::add(//射击时的枪口火光
        irr::scene::ISceneManager * scene,
        irr::scene::IAnimatedMeshSceneNode * n,
        const irr::core::vector3df & from,
        const irr::core::vector3df & dir
    ){
        auto particles = scene->addParticleSystemSceneNode(false,n);//创建粒子系统
        
        irr::core::vector3df rd=dir;
        rd.normalize();
        rd*=particleSpeed;
        
        auto emitter   = particles->createPointEmitter(//创建粒子发射器
            rd,
            minParticlesPerSecond,
            maxParticlesPerSecond,
            minStartColor,
            maxStartColor,
            lifeTimeMin,
            lifeTimeMax,
            maxAngleDegrees
        );
        particles->setEmitter(emitter);//设置发射器
        particles->setParticlesAreGlobal(false);//不受场景节点影响
        emitter->drop();
        
        setVanishingTime(particles,scene,this->delayMs);//攻击完后粒子发射器消失
    }
    
    void attackInter::attackAmExplode::add(//爆炸
        irr::scene::ISceneManager * scene,
        irr::scene::IAnimatedMeshSceneNode * n,
        const irr::core::vector3df & from,
        const irr::core::vector3df & dir
    ){
        auto particles = scene->addParticleSystemSceneNode(false,n);//创建粒子系统
        
        auto emitter   = particles->createPointEmitter(//创建粒子发射器
            irr::core::vector3df(0,particleSpeed,0),
            minParticlesPerSecond,
            maxParticlesPerSecond,
            minStartColor,
            maxStartColor,
            lifeTimeMin,
            lifeTimeMax,
            360//任意方向发射
        );
        particles->setEmitter(emitter);//设置发射器
        particles->setParticlesAreGlobal(false);//不受场景节点影响
        emitter->drop();
        
        auto aff=particles->createGravityAffector(irr::core::vector3df(0.0f,-0.7f, 0.0f),timeForceLost);
        particles->addAffector(aff);//添加重力
        aff->drop();
        
        setVanishingTime(particles,scene,this->delayMs);//粒子发射器消失
    }
    
}
