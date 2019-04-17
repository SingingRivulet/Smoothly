#include "physical.h"
#include <vector>
namespace smoothly{

float physical::getKEnergy(btRigidBody * r){
    float l=r->getLinearVelocity().length2();
    auto a=r->getAngularVelocity();
    auto im=r->getInvMass();
    auto ia=r->getInvInertiaDiagLocal();
    //0.5*lv2*m
    float le=0;
    float ae=0;
    if(im!=0)
        le=l*(1.0f/im);
    
    if(ia.getX())
        ae+=a.getX()*a.getX()*(1.0f/ia.getX());
    
    if(ia.getY())
        ae+=a.getY()*a.getY()*(1.0f/ia.getY());
    
    if(ia.getZ())
        ae+=a.getZ()*a.getZ()*(1.0f/ia.getZ());
    
    return (le+ae)*0.5f;
}

static btVector3 toBtVector( const irr::core::vector3df & vec ){
    btVector3 bt( vec.X, vec.Y, vec.Z );
    return bt;
}
btTriangleMesh * physical::createBtMesh(irr::scene::IMesh * mesh){
    const size_t buffercount = mesh->getMeshBufferCount();
    std::vector<irr::video::S3DVertex> verticesList;
    std::vector<int> indicesList;
    
    btTriangleMesh * btmesh = new btTriangleMesh();
    
    for ( size_t i=0; i<buffercount; ++i ){
        // Current meshbuffer
        irr::scene::IMeshBuffer * buffer = mesh->getMeshBuffer( i );
        // EVT_STANDARD -> video::S3DVertex
        // EVT_2TCOORDS -> video::S3DVertex2TCoords
        // EVT_TANGENTS -> video::S3DVertexTangents
        const irr::video::E_VERTEX_TYPE vertexType = buffer->getVertexType();
        // EIT_16BIT
        // EIT_32BIT
        const irr::video::E_INDEX_TYPE  indexType  = buffer->getIndexType();

        // Get working data
        const size_t numVerts    = buffer->getVertexCount();
        const size_t numInd      = buffer->getIndexCount();

        void * vertices = buffer->getVertices();
        void * indices  = buffer->getIndices();

        irr::video::S3DVertex            * standard   = reinterpret_cast<irr::video::S3DVertex*>( vertices );
        irr::video::S3DVertex2TCoords    * two2coords = reinterpret_cast<irr::video::S3DVertex2TCoords*>( vertices );
        irr::video::S3DVertexTangents    * tangents   = reinterpret_cast<irr::video::S3DVertexTangents*>( vertices );

        int16_t * ind16= reinterpret_cast<int16_t*>( indices );
        int32_t * ind32= reinterpret_cast<int32_t*>( indices );
        
        switch ( vertexType ){
            case irr::video::EVT_STANDARD:
                if(indexType==irr::video::EIT_32BIT){
                    for(size_t n=0;n<numInd; n+=3){
                        auto index1=ind32[n];   if(index1<0 || index1>=numVerts)return btmesh;
                        auto index2=ind32[n+1]; if(index2<0 || index2>=numVerts)return btmesh;
                        auto index3=ind32[n+2]; if(index3<0 || index3>=numVerts)return btmesh;
                        const btVector3 A = toBtVector(standard[index1].Pos);
                        const btVector3 B = toBtVector(standard[index2].Pos);
                        const btVector3 C = toBtVector(standard[index3].Pos);
                        btmesh->addTriangle(A,B,C,true);
                    }
                }else
                if(indexType==irr::video::EIT_16BIT){
                    for(size_t n=0;n<numInd; n+=3){
                        auto index1=ind16[n];   if(index1<0 || index1>=numVerts)return btmesh;
                        auto index2=ind16[n+1]; if(index2<0 || index2>=numVerts)return btmesh;
                        auto index3=ind16[n+2]; if(index3<0 || index3>=numVerts)return btmesh;
                        const btVector3 A = toBtVector(standard[index1].Pos);
                        const btVector3 B = toBtVector(standard[index2].Pos);
                        const btVector3 C = toBtVector(standard[index3].Pos);
                        btmesh->addTriangle(A,B,C,true);
                    }
                }
            break;
            case irr::video::EVT_2TCOORDS:
                if(indexType==irr::video::EIT_32BIT){
                    for(size_t n=0;n<numInd; n+=3){
                        auto index1=ind32[n];   if(index1<0 || index1>=numVerts)return btmesh;
                        auto index2=ind32[n+1]; if(index2<0 || index2>=numVerts)return btmesh;
                        auto index3=ind32[n+2]; if(index3<0 || index3>=numVerts)return btmesh;
                        const btVector3 A = toBtVector(two2coords[index1].Pos);
                        const btVector3 B = toBtVector(two2coords[index2].Pos);
                        const btVector3 C = toBtVector(two2coords[index3].Pos);
                        btmesh->addTriangle(A,B,C,true);
                    }
                }else
                if(indexType==irr::video::EIT_16BIT){
                    for(size_t n=0;n<numInd; n+=3){
                        auto index1=ind16[n];   if(index1<0 || index1>=numVerts)return btmesh;
                        auto index2=ind16[n+1]; if(index2<0 || index2>=numVerts)return btmesh;
                        auto index3=ind16[n+2]; if(index3<0 || index3>=numVerts)return btmesh;
                        const btVector3 A = toBtVector(two2coords[index1].Pos);
                        const btVector3 B = toBtVector(two2coords[index2].Pos);
                        const btVector3 C = toBtVector(two2coords[index3].Pos);
                        btmesh->addTriangle(A,B,C,true);
                    }
                }
            break;
            case irr::video::EVT_TANGENTS:
                if(indexType==irr::video::EIT_32BIT){
                    for(size_t n=0;n<numInd; n+=3){
                        auto index1=ind32[n];   if(index1<0 || index1>=numVerts)return btmesh;
                        auto index2=ind32[n+1]; if(index2<0 || index2>=numVerts)return btmesh;
                        auto index3=ind32[n+2]; if(index3<0 || index3>=numVerts)return btmesh;
                        const btVector3 A = toBtVector(tangents[index1].Pos);
                        const btVector3 B = toBtVector(tangents[index2].Pos);
                        const btVector3 C = toBtVector(tangents[index3].Pos);
                        btmesh->addTriangle(A,B,C,true);
                    }
                }else
                if(indexType==irr::video::EIT_16BIT){
                    for(size_t n=0;n<numInd; n+=3){
                        auto index1=ind16[n];   if(index1<0 || index1>=numVerts)return btmesh;
                        auto index2=ind16[n+1]; if(index2<0 || index2>=numVerts)return btmesh;
                        auto index3=ind16[n+2]; if(index3<0 || index3>=numVerts)return btmesh;
                        const btVector3 A = toBtVector(tangents[index1].Pos);
                        const btVector3 B = toBtVector(tangents[index2].Pos);
                        const btVector3 C = toBtVector(tangents[index3].Pos);
                        btmesh->addTriangle(A,B,C,true);
                    }
                }
            break;
            default:
                printf("unkown index type\n" );
            break;
        }
        
    }
    return btmesh;
}
btMotionState * physical::setMotionState(const float * mtx){
    btTransform transform;
    transform.setFromOpenGLMatrix(mtx);
    return new btDefaultMotionState(transform);
}
void physical::setMotionState(btMotionState * motionState,const float * mtx){
    btTransform transform;
    transform.setFromOpenGLMatrix(mtx);
    motionState->setWorldTransform(transform);
}
btCollisionShape * physical::createShape(btTriangleMesh * mesh){
    return new btBvhTriangleMeshShape( mesh, true );
}
btRigidBody * physical::createBody(
    btCollisionShape * shape , 
    btMotionState * motionState , 
    btScalar mass , 
    const btVector3 & localInertia
){
    return new btRigidBody( mass , motionState, shape );
}
void physical::getMotionState(btMotionState * motionState,float * mtx){
    btTransform transform;
    motionState->getWorldTransform(transform);
    transform.getOpenGLMatrix(mtx);
}

void physical::bodyGroup::init(const std::string & conf){
    compound = new btCompoundShape();
    std::istringstream iss(conf);
    char buf[1024];
    while(!iss.eof()){
        iss.getline(buf,1024);
        parseLine(buf);
    }
}

void physical::bodyGroup::release(){
    if(compound){
        delete compound;
        compound=NULL;
    }
    for(auto it:children){
        if(it){
            delete it;
        }
    }
    children.clear();
}
void physical::bodyGroup::parseLine(const char * str){
    if(str[0]=='+'){
        std::istringstream iss(str+2);
        float px=0,py=0,pz=0,rx=0,ry=0,rz=0,rw=0;
        
        iss>>px;
        iss>>py;
        iss>>pz;
        
        iss>>rx;
        iss>>ry;
        iss>>rz;
        iss>>rw;
        
        btCollisionShape * obj=NULL;
        
        float bx=0,by=0,bz=0,rad=0,radius=0,height=0,hx=0,hy=0,hz=0;
        
        switch(str[1]){
            case 'b'://box
                iss>>bx;
                iss>>by;
                iss>>bz;
                obj=new btBoxShape(btVector3(bx,by,bz));
            break;
            case 's'://sphere
                iss>>rad;
                obj=new btSphereShape(rad);
            break;
            case 'c'://capsule
                iss>>radius;
                iss>>height;
                obj=new btCapsuleShape(radius,height);
            break;
            case 'y'://cylinder
                iss>>hx;
                iss>>hy;
                iss>>hz;
                obj=new btCylinderShape(btVector3(hx,hy,hz));
            break;
            case 'o'://cone
                iss>>radius;
                iss>>height;
                obj=new btConeShape(radius,height);
            break;
            default:
                return;
        }
        if(obj){
            add(obj,btVector3(px,py,pz),btQuaternion(rx,ry,rz,rw));
        }
    }else
    if(str[0]=='M'){
        float m=0,ix=0,iy=0,iz=0;
        std::istringstream iss(str+1);
        
        iss>>m;
        iss>>ix;
        iss>>iy;
        iss>>iz;
        
        btScalar mass(m);
        btVector3 localInertia(ix, iy, iz);
        compound->calculateLocalInertia(mass, localInertia);
    }
}
void physical::bodyGroup::add(btCollisionShape * obj,const btVector3& position,const btQuaternion& rotation){
    btTransform t;
    t.setIdentity();
    t.setOrigin(position);
    t.setRotation(rotation);
    
    children.push_back(obj);
    compound->addChildShape(t, obj);
}

}