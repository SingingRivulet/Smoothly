#include "terrain.h"
#include <vector>
namespace smoothly{
using namespace irr;
using namespace irr::scene;
scene::IMesh * terrain::createTerrainMesh(
    video::IImage* texture,
    float ** heightmap, 
    u32 hx,u32 hy,
    const core::dimension2d<f32>& stretchSize,
    video::IVideoDriver* driver,
    const core::dimension2d<u32>& maxVtxBlockSize, //网眼大小。官方文档没写
    bool debugBorders
)const{
    if (!texture || !heightmap)
		return 0;

	// debug border
	const s32 borderSkip = debugBorders ? 0 : 1;

	video::S3DVertex vtx;
	vtx.Color.set(255,255,255,255);

	SMesh* mesh = new SMesh();

	const core::dimension2d<u32> tMapSize= texture->getDimension();
	const core::position2d<f32> thRel(static_cast<f32>(tMapSize.Width) / hx, static_cast<f32>(tMapSize.Height) / hy);
	
	core::position2d<u32> processed(0,0);
	while (processed.Y<hy)
	{
		while(processed.X<hx)
		{
			core::dimension2d<u32> blockSize = maxVtxBlockSize;
			if (processed.X + blockSize.Width > hx)
				blockSize.Width = hx - processed.X;
			if (processed.Y + blockSize.Height > hy)
				blockSize.Height = hy - processed.Y;

			SMeshBuffer* buffer = new SMeshBuffer();
			buffer->setHardwareMappingHint(scene::EHM_STATIC);
			buffer->Vertices.reallocate(blockSize.getArea());
			// add vertices of vertex block
			u32 y;
			core::vector2df pos(0.f, processed.Y*stretchSize.Height);
			const core::vector2df bs(1.f/blockSize.Width, 1.f/blockSize.Height);
			core::vector2df tc(0.f, 0.5f*bs.Y);
			for (y=0; y<blockSize.Height; ++y)
			{
				pos.X=processed.X*stretchSize.Width;
				tc.X=0.5f*bs.X;
				for (u32 x=0; x<blockSize.Width; ++x)
				{
					const f32 height = heightmap[x+processed.X][y+processed.Y];

					vtx.Pos.set(pos.X, height, pos.Y);
					vtx.TCoords.set(tc);
					buffer->Vertices.push_back(vtx);
					pos.X += stretchSize.Width;
					tc.X += bs.X;
				}
				pos.Y += stretchSize.Height;
				tc.Y += bs.Y;
			}

			buffer->Indices.reallocate((blockSize.Height-1)*(blockSize.Width-1)*6);
			// add indices of vertex block
			s32 c1 = 0;
			for (y=0; y<blockSize.Height-1; ++y)
			{
				for (u32 x=0; x<blockSize.Width-1; ++x)
				{
					const s32 c = c1 + x;

					buffer->Indices.push_back(c);
					buffer->Indices.push_back(c + blockSize.Width);
					buffer->Indices.push_back(c + 1);

					buffer->Indices.push_back(c + 1);
					buffer->Indices.push_back(c + blockSize.Width);
					buffer->Indices.push_back(c + 1 + blockSize.Width);
				}
				c1 += blockSize.Width;
			}

			// recalculate normals
			for (u32 i=0; i<buffer->Indices.size(); i+=3)
			{
				const core::vector3df normal = core::plane3d<f32>(
					buffer->Vertices[buffer->Indices[i+0]].Pos,
					buffer->Vertices[buffer->Indices[i+1]].Pos,
					buffer->Vertices[buffer->Indices[i+2]].Pos).Normal;

				buffer->Vertices[buffer->Indices[i+0]].Normal = normal;
				buffer->Vertices[buffer->Indices[i+1]].Normal = normal;
				buffer->Vertices[buffer->Indices[i+2]].Normal = normal;
			}

			if (buffer->Vertices.size())
			{
				c8 textureName[64];
				// create texture for this block
				video::IImage* img = driver->createImage(texture->getColorFormat(), core::dimension2d<u32>(core::floor32(blockSize.Width*thRel.X), core::floor32(blockSize.Height*thRel.Y)));
				texture->copyTo(img, core::position2di(0,0), core::recti(
					core::position2d<s32>(core::floor32(processed.X*thRel.X), core::floor32(processed.Y*thRel.Y)),
					core::dimension2d<u32>(core::floor32(blockSize.Width*thRel.X), core::floor32(blockSize.Height*thRel.Y))), 0);


				buffer->Material.setTexture(0, driver->addTexture(textureName, img));

				if (buffer->Material.getTexture(0))
				{
					
				}

				img->drop();
			}

			buffer->recalculateBoundingBox();
			mesh->addMeshBuffer(buffer);
			buffer->drop();

			// keep on processing
			processed.X += maxVtxBlockSize.Width - borderSkip;
		}

		// keep on processing
		processed.X = 0;
		processed.Y += maxVtxBlockSize.Height - borderSkip;
	}

	mesh->recalculateBoundingBox();
	return mesh;
}

btRigidBody * makeBulletMeshFromIrrlichtNode( const irr::scene::ISceneNode * node )
{
	using irr::core::vector2df;
	using irr::core::vector3df;
	using irr::scene::IMesh;
	using irr::scene::IMeshBuffer;
	using irr::scene::IMeshSceneNode;

	// Handy lambda for converting from irr::vector to btVector
	auto toBtVector = [ &]( const vector3df & vec ) -> btVector3
	{
		btVector3 bt( vec.X, vec.Y, vec.Z );

		return bt;
	};

	irr::scene::ESCENE_NODE_TYPE type = node->getType();

	IMeshSceneNode * meshnode = nullptr;
	btRigidBody	* body = nullptr;

	switch ( type )
	{
		case irr::scene::ESNT_MESH:
			{
				meshnode = (IMeshSceneNode *)node;
			}
			break;
	}

	if ( meshnode )
	{
		const vector3df nodescale = meshnode->getScale();

		IMesh * mesh = meshnode->getMesh();
		const size_t buffercount = mesh->getMeshBufferCount();

		// Save position
		btVector3 position = toBtVector( meshnode->getPosition() );
        btQuaternion rotation;
        euler2quaternion(meshnode->getRotation() , rotation);

		// Save data here
		std::vector<irr::video::S3DVertex>	verticesList;
		std::vector<int>					indicesList;

		for ( size_t i=0; i<buffercount; ++i )
		{
			// Current meshbuffer
			IMeshBuffer * buffer = mesh->getMeshBuffer( i );

			// EVT_STANDARD -> video::S3DVertex
			// EVT_2TCOORDS -> video::S3DVertex2TCoords
			// EVT_TANGENTS -> video::S3DVertexTangents
			const irr::video::E_VERTEX_TYPE vertexType		= buffer->getVertexType();

			// EIT_16BIT
			// EIT_32BIT
			const irr::video::E_INDEX_TYPE	indexType		= buffer->getIndexType();

			// Get working data
			const size_t numVerts		= buffer->getVertexCount();
			const size_t numInd			= buffer->getIndexCount();

			// Resize save buffers
			verticesList.resize( verticesList.size() + numVerts );
			indicesList.resize( indicesList.size() + numInd );

			void * vertices				= buffer->getVertices();
			void * indices				= buffer->getIndices();

			irr::video::S3DVertex			* standard		= reinterpret_cast<irr::video::S3DVertex*>( vertices );
			irr::video::S3DVertex2TCoords	* two2coords	= reinterpret_cast<irr::video::S3DVertex2TCoords*>( vertices );
			irr::video::S3DVertexTangents	* tangents		= reinterpret_cast<irr::video::S3DVertexTangents*>( vertices );

			int16_t	* ind16		= reinterpret_cast<int16_t*>( indices );
			int32_t	* ind32		= reinterpret_cast<int32_t*>( indices );

			for ( size_t v = 0; v < numVerts; ++v )
			{
				auto & vert = verticesList[ v ];

				switch ( vertexType )
				{
					case irr::video::EVT_STANDARD:
						{
							const auto & irrv = standard[ v ];

							vert = irrv;
						}
						break;
					case irr::video::EVT_2TCOORDS:
						{
							const auto & irrv = two2coords[ v ];
							(void)irrv;

							// Not implemented
						}
						//break;
					case irr::video::EVT_TANGENTS:
						{
							const auto & irrv = tangents[ v ];
							(void)irrv;

							// Not implemented
						}
						//break;
					default:
						printf("unknown vertex type\n" );
						return NULL;
				}

			}

			for ( size_t n = 0; n < numInd; ++n )
			{
				auto & index = indicesList[ n ];

				switch ( indexType )
				{
					case irr::video::EIT_16BIT:
					{
						index = ind16[ n ];
					}
						break;
					case irr::video::EIT_32BIT:
					{
						index = ind32[ n ];
					}
						break;
					default:
						printf("unkown index type\n" );
						return NULL;
				}

			}

		}

		// Make bullet rigid body
		if ( ! verticesList.empty() && ! indicesList.empty() )
		{
			// Working numbers
			const size_t numIndices		= indicesList.size();
			const size_t numTriangles	= numIndices / 3;

			// Error checking
			if( numTriangles * 3 != numIndices){
			    printf("Number of indices does not make complete triangles\n" );
			    return NULL;
			}

			// Create triangles
			btTriangleMesh * btmesh = new btTriangleMesh();

			// Build btTriangleMesh
			for ( size_t i=0; i<numIndices; i+=3 )
			{
				const btVector3 &A = toBtVector( verticesList[ indicesList[ i+0 ] ].Pos );
				const btVector3 &B = toBtVector( verticesList[ indicesList[ i+1 ] ].Pos );
				const btVector3 &C = toBtVector( verticesList[ indicesList[ i+2 ] ].Pos );

				bool removeDuplicateVertices = true;
				btmesh->addTriangle( A, B, C, removeDuplicateVertices );
			}

			// Give it a default MotionState
			btTransform transform;
			transform.setIdentity();
            //set position
			transform.setOrigin( position );
            //set rotation
            transform.setRotation( rotation );
            
			btDefaultMotionState *motionState = new btDefaultMotionState( transform );

			// Create the shape
			btCollisionShape *btShape = new btBvhTriangleMeshShape( btmesh, true );
			btShape->setMargin( 0.05f );

			// Create the rigid body object
			btScalar mass = 0.0f;
			//printf("create btRigidBody\n" );
			body = new btRigidBody( mass, motionState, btShape );

		}

	}

	// Return Bullet rigid body
	return body;
}

}//namespace smoothly

