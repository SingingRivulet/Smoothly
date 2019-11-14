#include "physical.h"
#include <vector>
namespace smoothly{

static btVector3 toBtVector( const irr::core::vector3df & vec ){
    btVector3 bt( vec.X, vec.Y, vec.Z );
    return bt;
}
void physical::addQuadIntoMesh(btTriangleMesh * mesh,
    const btVector3 & A,
    const btVector3 & B,
    const btVector3 & C,
    const btVector3 & D
){
    mesh->addTriangle(A,B,C,true);
    mesh->addTriangle(A,D,C,true);
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
    //motionState->m_centerOfMassOffset.setIdentity();
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

void physical::shapeGroup::init(const std::string & conf){
    compound = new btCompoundShape();
    char buf[1024];
    std::istringstream reader(conf);
    while(!reader.eof()){
        if(reader.getline(buf,1024)){
            parseLine(buf);
            buf[0]='\0';//往buf第一字节写入0，防止下一次读到相同值
        }
    }
}

void physical::shapeGroup::release(){
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
void physical::shapeGroup::parseLine(const char * str){
    if(str[0]=='+'){
        
        if(str[1]=='\0' || str[2]=='\0')
            return;
        
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
        if(str[1]=='\0')
            return;
        float m=0,ix=0,iy=0,iz=0;
        std::istringstream iss(str+1);
        
        iss>>m;
        iss>>ix;
        iss>>iy;
        iss>>iz;
        
        mass=m;
        localInertia=btVector3(ix, iy, iz);
        compound->calculateLocalInertia(mass, localInertia);
    }else
    if(str[0]=='F'){
        
        if(str[1]=='\0')
            return;
            
        float f=0;
        std::istringstream iss(str+1);
        
        iss>>f;
        
        setFric(f);
    }else
    if(str[0]=='E'){
        
        if(str[1]=='\0')
            return;
            
        float e=0;
        std::istringstream iss(str+1);
        
        iss>>e;
        
        setResti(e);
    }
}
void physical::shapeGroup::add(btCollisionShape * obj,const btVector3& position,const btQuaternion& rotation){
    btTransform t;
    t.setIdentity();
    t.setOrigin(position);
    t.setRotation(rotation);
    
    children.push_back(obj);
    compound->addChildShape(t, obj);
}
void physical::shapeGroup::setFric(float f){}
void physical::shapeGroup::setResti(float r){}

void physical::bodyGroup::init(const std::string & conf){
    shapeGroup::init(conf);
    
    btTransform startTransform;
    startTransform.setIdentity();
    motion=new btDefaultMotionState(startTransform);
    
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motion, compound, localInertia);
    body = new btRigidBody(rbInfo);
    body->setFriction(this->f);
    body->setRestitution(this->e);
}
void physical::bodyGroup::release(){
    delete body;
    delete motion;
    
    shapeGroup::release();
}
void physical::bodyGroup::setFric(float fr){
    this->f=fr;
}
void physical::bodyGroup::setResti(float r){
    this->e=r;
}

void physical::bodyBase::setPosition(const irr::core::vector3df & p){
    //printf("[bodyBase]setPosition\n");
    teleport(p);
}
void physical::bodyBase::teleport(const irr::core::vector3df & p){
    btTransform t;
    getTransform(t);
    
    t.setOrigin(btVector3(p.X , p.Y , p.Z));
    
    setTransform(t);
}
void physical::bodyBase::setRotation(const irr::core::vector3df & r){
    btTransform t;
    getTransform(t);
    
    btQuaternion rq;
    euler2quaternion(r,rq);
    t.setRotation(rq);
    
    setTransform(t);
}
void physical::bodyBase::getStatus(irr::core::vector3df & irrPos,irr::core::vector3df & irrRot){
    btTransform transform;
    getTransform(transform);
    
    btVector3 btPos;
    btVector3 btRot;
    
    btPos = transform.getOrigin();
    irrPos.set(btPos.x(), btPos.y(), btPos.z());
    
    btMatrix3x3 & btM = transform.getBasis();
    btM.getEulerZYX(btRot.m_floats[2], btRot.m_floats[1], btRot.m_floats[0]);
    irrRot.X = irr::core::radToDeg(btRot.x());
    irrRot.Y = irr::core::radToDeg(btRot.y());
    irrRot.Z = irr::core::radToDeg(btRot.z());
}
irr::core::vector3df physical::bodyBase::getDir(){
    return direction;
}
void physical::bodyBase::setDir(const irr::core::vector3df & d){
    //printf("[physical::bodyBase]setDir direction=(%f,%f,%f)\n",d.X,d.Y,d.Z);
    direction=d;
    irr::core::vector3df rotate=d.getHorizontalAngle();
    setRotation(rotate);
}

physical::character::character(btScalar w,btScalar h,const btVector3 & position,bool wis,bool jis,btScalar stepHeight){
    //printf("[body]create character\n");
    
    walkInSky=wis;
    jumpInSky=jis;
    
    shape = new btCapsuleShape(w,h);
    
    btTransform m_trans;
    m_trans.setIdentity();
    m_trans.setOrigin(position);
 
    m_ghostObject = new btPairCachingGhostObject();
    m_ghostObject->setWorldTransform(m_trans);
    
    m_ghostObject->setCollisionShape (shape);
    m_ghostObject->setCollisionFlags (btCollisionObject::CF_CHARACTER_OBJECT);
    
    controller = new btKinematicCharacterController (m_ghostObject,shape,stepHeight);
    
    controller->setGravity(btVector3(0, -10, 0));
    
    firstUpdate=true;
}
void physical::character::destruct(){
    delete controller;
    delete m_ghostObject;
    delete shape;
    delete this;
}
void physical::character::getDeltaL(irr::core::vector3df & out){
    btTransform transform;
    getTransform(transform);
    
    btVector3 btPos;
    btVector3 btRot;
    
    btPos = transform.getOrigin();
    irr::core::vector3df irrPos(btPos.x(), btPos.y(), btPos.z());
    
    if(firstUpdate){
        out.set(0,0,0);
    }else{
        out=irrPos-lastPosition;
    }
    
    lastPosition=irrPos;
    firstUpdate=false;
}
irr::core::vector3df physical::character::getDir(){
    return direction;
}
void physical::character::setDir(const irr::core::vector3df & d){
    //return;
    //printf("[character]setDir (%f,%f,%f)\n",d.X,d.Y,d.Z);
    direction=d;//real direction
    irr::core::vector3df rotate=d.getHorizontalAngle();
    //注意：这里必须用弧度，不能用角度
    float yaw=irr::core::degToRad(rotate.Y);
    btQuaternion quaternion(btVector3(0,1,0),yaw);
    m_ghostObject->getWorldTransform().setRotation(quaternion);
}
void physical::character::setTransform(btTransform & t){
    //printf("[character]setTransform\n");
    m_ghostObject->setWorldTransform(t);
}
void physical::character::getTransform(btTransform & t){
    t=m_ghostObject->getWorldTransform();
}
void physical::character::addIntoWorld(){
    world->addCollisionObject(
        m_ghostObject,
        btBroadphaseProxy::CharacterFilter, 
        btBroadphaseProxy::StaticFilter|btBroadphaseProxy::DefaultFilter);
    world->addAction(controller);
}
void physical::character::removeFromWorld(){
    world->removeAction(controller);
    world->removeCollisionObject(m_ghostObject);
}
void physical::character::setWalkDirection(const btVector3& walkDirection){
    //printf("[character]walk (%f,%f,%f)\n",walkDirection.getX(),walkDirection.getY(),walkDirection.getZ());
    if(!walkInSky && !controller->onGround())
        return;
    controller->setWalkDirection(walkDirection);
}
void physical::character::jump(const btVector3& dir){
    if(!jumpInSky && !controller->onGround())
        return;
    controller->jump(dir);
}
void physical::character::setUserPointer(void * p){
    m_ghostObject->setUserPointer(p);
}
void physical::character::setAngularFactor(const btVector3 & a){
    //m_ghostObject->setAngularFactor(a);
}

physical::rigidBody::rigidBody(
    btCollisionShape * bodyShape,
    btTransform & transform,
    btScalar mass , 
    const btVector3& localInertia
){
    //printf("[body]create rigid body\n");
    bodyState=new btDefaultMotionState(transform);
    body=createBody(bodyShape , bodyState , mass , localInertia);
}

void physical::rigidBody::destruct(){
    delete body;
    delete bodyState;
    delete this;
}

void physical::rigidBody::addIntoWorld(){
    world->addRigidBody(body);
}

void physical::rigidBody::removeFromWorld(){
    world->removeRigidBody(body);
}

void physical::rigidBody::setTransform(btTransform & t){
    body->setWorldTransform(t);
    bodyState->m_graphicsWorldTrans=t;
    bodyState->m_startWorldTrans=t;
    bodyState->m_centerOfMassOffset.setIdentity();
    //我也不知道为什么，刚体必须同时设置这个四个
}

void physical::rigidBody::getTransform(btTransform & t){
    bodyState->getWorldTransform(t);
}

void physical::rigidBody::applyImpulse(const btVector3& impluse,const btVector3& point){
    body->applyImpulse(impluse,point);
}
void physical::rigidBody::applyForce(const btVector3& force,const btVector3& point){
    body->applyForce(force,point);
}
void physical::rigidBody::clearForces(){
    body->clearForces();
}

void physical::rigidBody::setLinearVelocity(const btVector3& lin_vel){
    body->setLinearVelocity(lin_vel);
}
void physical::rigidBody::setAngularVelocity(const btVector3& ang_vel){
    body->setAngularVelocity(ang_vel);
}
void physical::rigidBody::setFriction(btScalar f){
    body->setFriction(f);
}
void physical::rigidBody::setRestitution(btScalar r){
    body->setRestitution(r);
}
btVector3 physical::rigidBody::getAngularVelocity(){
    return body->getAngularVelocity();
}
btVector3 physical::rigidBody::getLinearVelocity(){
    return body->getLinearVelocity();
}
void physical::rigidBody::setAngularFactor(const btVector3 & a){
    body->setAngularFactor(a);
}
void physical::rigidBody::setUserPointer(void * p){
    body->setUserPointer(p);
}

}