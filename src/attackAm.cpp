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
        
        auto mesh = geo->createPlaneMesh(irr::core::dimension2d<f32>(1,1));//创建一个正方形
        
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
        
        node->setPosition(from);//位置
        node->setRotation(dir.getHorizontalAngle());//旋转
        
        setVanishingTime(node,scene,this->delayMs);
    }
}
