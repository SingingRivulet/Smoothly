#include "physical.h"
#include <vector>
namespace smoothly{

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
btRigidBody * physical::createBody(btCollisionShape * shape , btMotionState * motionState){
    return new btRigidBody( 0.0f , motionState, shape );
}
void physical::getMotionState(btMotionState * motionState,float * mtx){
    btTransform transform;
    motionState->getWorldTransform(transform);
    transform.getOpenGLMatrix(mtx);
}

}