#include "../terrain_item.h"
#include <functional>

namespace smoothly{


class meshGenerator{
    public:
        meshGenerator(uint t) {
            buffer = new irr::scene::SMeshBuffer();
            setSteps(t);
            index = 0;
        }
        void setSteps(uint t){
            tesselation = t;
            recTesselation = core::reciprocal((f32)tesselation);
            recTesselationHalf = recTesselation * 0.5f;
            angleStep = (core::PI * 2.f ) / tesselation;
            angleStepHalf = angleStep*0.5f;
        }
        ~meshGenerator(){
            buffer->drop();
        }
        void pushBaseRound(float radius){//添加基座
            irr::video::S3DVertex v;
            f32 tcx = 0.f;
            irr::core::matrix4 rotMtx;
            rotMtx.setRotationDegrees(transform.getRotationDegrees());
            for (uint i = 0; i != tesselation; ++i ){
                f32 angle = angleStep * f32(i);

                v.Pos.X = radius * cosf(angle);
                v.Pos.Y = 0.f;
                v.Pos.Z = radius * sinf(angle);
                v.Normal = v.Pos;
                rotMtx.transformVect(v.Normal);
                transform.transformVect(v.Pos);
                v.TCoords.X=tcx;
                v.TCoords.Y=0.f;
                v.Color.set(255,64,64,16);
                v.Normal.normalize();
                buffer->Vertices.push_back(v);

                angle += angleStepHalf;
                v.Pos.X = radius * cosf(angle);
                v.Pos.Y = 0.f;
                v.Pos.Z = radius * sinf(angle);
                v.Normal = v.Pos;
                rotMtx.transformVect(v.Normal);
                transform.transformVect(v.Pos);
                v.TCoords.X=tcx+recTesselationHalf;
                v.TCoords.Y=0.f;
                v.Color.set(255,64,64,16);
                v.Normal.normalize();
                buffer->Vertices.push_back(v);

                index+=2;
            }
            nowHeight = 0;
            reverseTexture = 0;
        }
        void pushRound(float radius,float height,vec3 &ori){
            irr::video::S3DVertex v;
            uint start = index;
            ori.set(0,0,0);
            f32 tcx = 0.f;
            irr::core::matrix4 rotMtx;
            rotMtx.setRotationDegrees(transform.getRotationDegrees());
            for (uint i = 0; i != tesselation; ++i ){
                f32 angle = angleStep * f32(i);

                v.Pos.X = radius * cosf(angle);
                v.Pos.Y = nowHeight + height;
                v.Pos.Z = radius * sinf(angle);
                v.Normal.set(v.Pos.X,0,v.Pos.Z);
                rotMtx.transformVect(v.Normal);
                transform.transformVect(v.Pos);
                v.TCoords.X=tcx;
                if(reverseTexture)
                    v.TCoords.Y=0.f;
                else
                    v.TCoords.Y=1.f;
                ori+=v.Pos;
                v.Color.set(255,64,64,16);
                v.Normal.normalize();
                buffer->Vertices.push_back(v);

                angle += angleStepHalf;
                v.Pos.X = radius * cosf(angle);
                v.Pos.Y = nowHeight + height;
                v.Pos.Z = radius * sinf(angle);
                v.Normal.set(v.Pos.X,0,v.Pos.Z);
                rotMtx.transformVect(v.Normal);
                transform.transformVect(v.Pos);
                v.TCoords.X=tcx+recTesselationHalf;
                if(reverseTexture)
                    v.TCoords.Y=0.f;
                else
                    v.TCoords.Y=1.f;
                ori+=v.Pos;
                v.Color.set(255,64,64,16);
                v.Normal.normalize();
                buffer->Vertices.push_back(v);

                buffer->Indices.push_back(index);
                buffer->Indices.push_back(index+1);
                buffer->Indices.push_back(index-2*tesselation);

                buffer->Indices.push_back(index+1);
                buffer->Indices.push_back(index-2*tesselation+1);
                buffer->Indices.push_back(index-2*tesselation);

                if(i!=0){
                    buffer->Indices.push_back(index-1);
                    buffer->Indices.push_back(index);
                    buffer->Indices.push_back(index-2*tesselation-1);

                    buffer->Indices.push_back(index);
                    buffer->Indices.push_back(index-2*tesselation);
                    buffer->Indices.push_back(index-2*tesselation-1);
                }
                if(i == tesselation-1){
                    buffer->Indices.push_back(index+1);
                    buffer->Indices.push_back(start);
                    buffer->Indices.push_back(index-2*tesselation+1);

                    buffer->Indices.push_back(start);
                    buffer->Indices.push_back(start-2*tesselation);
                    buffer->Indices.push_back(index-2*tesselation+1);
                }

                index+=2;
            }
            ori/=(tesselation*2);
            nowHeight += height;
            reverseTexture =!reverseTexture;
        }
        void buildTree(float r,float hei,float k,int step,const vec3 & bending,std::function<void(const vec3 & ,float,int)> callback){
            pushBaseRound(r);
            vec3 o;
            float h=0;
            irr::core::matrix4 rt,backup(transform);
            rt.buildRotateFromTo(vec3(0,1,0),bending);
            for(int i=0;i<step;++i){
                h+=hei;
                float rr = 1/exp(h*k)*r;
                pushRound(rr,hei,o);
                callback(o,rr,i);
                transform*=rt;
            }
            transform = backup;
        }
        irr::scene::SMeshBuffer *   buffer;
        irr::core::matrix4          transform;
    private:
        irr::f32 angleStep,angleStepHalf,recTesselation,recTesselationHalf;
        uint tesselation;
        uint index;
        float nowHeight;
        bool reverseTexture;//翻转纹理标识
};

class leavesBuffer{//树叶buffer
    public:
        leavesBuffer() {
            buffer = new irr::scene::SMeshBuffer();
            index = 0;
        }
        ~leavesBuffer(){
            buffer->drop();
        }
        void addLeaves(float len){
            irr::video::S3DVertex v;
            v.Color.set(255,64,128,16);

            //正面
            v.Pos.X = 0;
            v.Pos.Y = 0;
            v.Pos.Z = 0;
            transform.transformVect(v.Pos);
            v.TCoords.X=0.f;
            v.TCoords.Y=0.f;
            v.Normal = v.Pos;
            v.Normal.normalize();
            buffer->Vertices.push_back(v);

            v.Pos.X = 0;
            v.Pos.Y = 0;
            v.Pos.Z = len;
            transform.transformVect(v.Pos);
            v.TCoords.X=0.f;
            v.TCoords.Y=1.f;
            v.Normal = v.Pos;
            v.Normal.normalize();
            buffer->Vertices.push_back(v);

            v.Pos.X = len;
            v.Pos.Y = 0;
            v.Pos.Z = len;
            transform.transformVect(v.Pos);
            v.TCoords.X=1.f;
            v.TCoords.Y=1.f;
            v.Normal = v.Pos;
            v.Normal.normalize();
            buffer->Vertices.push_back(v);

            v.Pos.X = len;
            v.Pos.Y = 0;
            v.Pos.Z = 0;
            transform.transformVect(v.Pos);
            v.TCoords.X=1.f;
            v.TCoords.Y=0.f;
            v.Normal = v.Pos;
            v.Normal.normalize();
            buffer->Vertices.push_back(v);

            buffer->Indices.push_back(index+0);
            buffer->Indices.push_back(index+1);
            buffer->Indices.push_back(index+3);

            buffer->Indices.push_back(index+1);
            buffer->Indices.push_back(index+2);
            buffer->Indices.push_back(index+3);

            index+=4;

            //背面
            v.Pos.X = 0;
            v.Pos.Y = 0;
            v.Pos.Z = 0;
            transform.transformVect(v.Pos);
            v.TCoords.X=0.f;
            v.TCoords.Y=0.f;
            v.Normal = v.Pos;
            v.Normal.normalize();
            buffer->Vertices.push_back(v);

            v.Pos.X = 0;
            v.Pos.Y = 0;
            v.Pos.Z = len;
            transform.transformVect(v.Pos);
            v.TCoords.X=0.f;
            v.TCoords.Y=1.f;
            v.Normal = v.Pos;
            v.Normal.normalize();
            buffer->Vertices.push_back(v);

            v.Pos.X = len;
            v.Pos.Y = 0;
            v.Pos.Z = len;
            transform.transformVect(v.Pos);
            v.TCoords.X=1.f;
            v.TCoords.Y=1.f;
            v.Normal = v.Pos;
            v.Normal.normalize();
            buffer->Vertices.push_back(v);

            v.Pos.X = len;
            v.Pos.Y = 0;
            v.Pos.Z = 0;
            transform.transformVect(v.Pos);
            v.TCoords.X=1.f;
            v.TCoords.Y=0.f;
            v.Normal = v.Pos;
            v.Normal.normalize();
            buffer->Vertices.push_back(v);

            buffer->Indices.push_back(index+1);
            buffer->Indices.push_back(index+0);
            buffer->Indices.push_back(index+2);

            buffer->Indices.push_back(index+2);
            buffer->Indices.push_back(index+0);
            buffer->Indices.push_back(index+3);

            index+=4;
        }
        irr::core::matrix4          transform;
    private:
        uint index;
    public:
        irr::scene::SMeshBuffer *   buffer;
};

irr::scene::ISceneNode * terrain_item::genTree(int seed, btTriangleMesh *&bodyMesh){
    meshGenerator m(4);
    leavesBuffer   g;
    world::terrain::predictableRand randg;
    randg.setSeed(seed);

    auto res = scene->addEmptySceneNode();

    std::vector<std::pair<vec3,float> > oris,oris_child;
    //主干
    m.buildTree(2*randg.frand()+2,8,
                0.01+randg.frand()*0.01,16*randg.frand()+6,
                vec3((randg.frand()-0.5)*0.07,4,(randg.frand()-0.5)*0.07),
                [&](const vec3 & o,float r,int i){
        if(i>3){
            oris.push_back(std::pair<vec3,float>(o,r));
        }
        if(i>5){
            if(randg.frand()>0.5)
                return;
            g.transform.setTranslation(o);
            g.transform.setRotationDegrees(m.transform.getRotationDegrees());
            irr::core::matrix4 rtm;
            rtm.buildRotateFromTo(vec3(0,1,0),vec3((randg.frand()-0.5)*0.3,(randg.frand()-0.5)*0.3,(randg.frand()-0.5)*0.3));
            g.transform*=rtm;
            g.addLeaves(16);
        }
    });
    //一级分支
    if(!oris.empty())
        oris.pop_back();
    for(auto it:oris){
        m.setSteps(3);
        oris_child.clear();
        auto rotation = vec3(90+(randg.frand()-0.5)*30,randg.frand()*360,0);
        m.transform.setRotationDegrees(rotation);
        m.transform.setTranslation(it.first);
        irr::core::matrix4 backup = m.transform;
        m.buildTree(it.second,4,0.05,4*randg.frand()+8,vec3((randg.frand()-0.5)*0.3,4,-(randg.frand()*0.3)),[&](const vec3 & o,float r,int){
            oris_child.push_back(std::pair<vec3,float>(o,r));
        });
        //二级分支
        if(!oris_child.empty())
            oris_child.pop_back();
        m.setSteps(2);
        for(auto it2:oris_child){
            irr::core::matrix4 st,rt;
            st.setRotationDegrees(rotation);
            st.setTranslation(it2.first);
            rt.buildRotateFromTo(vec3(0,1,0),vec3(randg.frand()>0.5?1:-1,0,(randg.frand()-0.5)*0.3));
            m.transform = st*rt;
            m.buildTree(it2.second,4,0.06,3*randg.frand()+3,vec3((randg.frand()-0.5)*0.3,4,-(randg.frand()*0.3)),[&](const vec3 & o,float,int){
                if(randg.frand()>0.5)
                    return;
                g.transform.setTranslation(o);
                g.transform.setRotationDegrees(m.transform.getRotationDegrees());
                irr::core::matrix4 rtm;
                rtm.buildRotateFromTo(vec3(0,1,0),vec3((randg.frand()-0.5)*0.3,(randg.frand()-0.5)*0.3,(randg.frand()-0.5)*0.3));
                g.transform*=rtm;
                g.addLeaves(16);
            });
        }
        m.transform = backup;
    }

    m.buffer->recalculateBoundingBox();
    auto trunk_mesh = new irr::scene::SMesh();
    scene->getMeshManipulator()->recalculateNormals(m.buffer);
    trunk_mesh->addMeshBuffer(m.buffer);
    trunk_mesh->setHardwareMappingHint(irr::scene::EHM_STATIC);
    trunk_mesh->recalculateBoundingBox();
    trunk_mesh->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true );
    auto trunk = scene->addMeshSceneNode(trunk_mesh,res);
    trunk->setMaterialFlag(irr::video::EMF_LIGHTING, true );
    trunk->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true );
    trunk->setMaterialFlag(irr::video::EMF_ANISOTROPIC_FILTER, true );
    trunk->setMaterialType((irr::video::E_MATERIAL_TYPE)shader_tree);
    trunk->setMaterialTexture(0,texture_treeTrunk);
    trunk->getMaterial(0).ZWriteFineControl = irr::video::EZI_ZBUFFER_FLAG;
    trunk->getMaterial(0).BlendOperation=irr::video::EBO_NONE;
    bodyMesh = createBtMesh(trunk_mesh);
    trunk_mesh->drop();

    //添加叶子
    g.buffer->recalculateBoundingBox();
    scene->getMeshManipulator()->recalculateNormals(g.buffer);
    auto leaves_mesh = new irr::scene::SMesh();
    leaves_mesh->addMeshBuffer(g.buffer);
    leaves_mesh->setHardwareMappingHint(irr::scene::EHM_STATIC);
    leaves_mesh->recalculateBoundingBox();
    leaves_mesh->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true );
    auto leaves = scene->addMeshSceneNode(leaves_mesh,res);
    leaves->setMaterialFlag(irr::video::EMF_LIGHTING, true );
    leaves->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, true );
    res->setMaterialFlag(irr::video::EMF_ANISOTROPIC_FILTER, true );
    leaves->setMaterialType((irr::video::E_MATERIAL_TYPE)shader_tree);
    leaves->getMaterial(0).ZWriteFineControl = irr::video::EZI_ZBUFFER_FLAG;
    leaves->setMaterialTexture(0,texture_treeLeaves);
    leaves->getMaterial(0).BlendOperation=irr::video::EBO_NONE;
    leaves_mesh->drop();

    return res;
}

}
