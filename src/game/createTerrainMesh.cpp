#include "terrain.h"
#include <vector>
namespace smoothly{
using namespace irr;
using namespace irr::scene;
scene::IMesh * terrain::createTerrainMesh(
    video::ITexture* texture,
    float ** heightmap,
    u32 hx,u32 hy,
    const core::dimension2d<f32>& stretchSize,
    const core::dimension2d<u32>& maxVtxBlockSize, //网眼大小。官方文档没写
    bool debugBorders
){
    if (!heightmap)
		return 0;

	// debug border
	const s32 borderSkip = debugBorders ? 0 : 1;

	video::S3DVertex vtx;
	vtx.Color.set(255,255,255,255);

	SMesh* mesh = new SMesh();

	
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

            if (buffer->Vertices.size() && texture)
			{
				
				buffer->Material.setTexture(0, texture);

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

}//namespace smoothly

