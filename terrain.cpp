#include "terrain.h"
#include "mempool.h"
namespace smoothly{

typedef mempool<terrain::chunk> chunkpool;
terrain::terrain(){
    this->pool=new chunkpool;
    for(int i=0;i<7;i++){
        for(int j=0;j<7;j++){
            visualChunk[i][j]=NULL;
        }
    }
}

terrain::~terrain(){
    for(int i=0;i<7;i++){
        for(int j=0;j<7;j++){
            if(visualChunk[i][j]){
                visualChunk[i][j]->remove();
                removecChunk(visualChunk[i][j]);
            }
        }
    }
    delete (chunkpool*)this->pool;
}

terrain::chunk * terrain::createChunk(){
    auto ptr=((chunkpool*)this->pool)->get();
    ptr->scene=this->scene;
    ptr->device=this->device;
    ptr->T=new float*[pointNum];
    for(int i=0;i<pointNum;i++){
        ptr->T[i]=new float[pointNum];
    }
}

void terrain::removecChunk(terrain::chunk * ptr){
    for(int i=0;i<pointNum;i++){
        delete [] ptr->T[i];
    }
    delete [] ptr->T;
    ((chunkpool*)this->pool)->del(ptr);
}

void terrain::visualChunkUpdate(irr::u32 x , irr::u32 y , bool force){
    if(x==px && y==py && !force)
        return;
    terrain::chunk * tmp[7][7];
    int i,j;
    for(i=0;i<7;i++){
        for(j=0;j<7;j++){
            tmp[i][j]=visualChunk[i][j];
        }
    }
    
    int relativeX=x-px;
    int relativeY=y-py;
    
    for(i=0;i<7;i++){
        for(j=0;j<7;j++){
            int xindex=i-relativeX;
            int yindex=j-relativeY;
            if(force || xindex<0 || xindex>6 || yindex<0 || yindex>6 || tmp[xindex][yindex]==NULL){
                auto ptr=createChunk();
                //2号是中心
                updateChunk(ptr , x+i-3 , y+i-3);
                visualChunk[i][j]=ptr;
            }else{
                visualChunk[i][j]=tmp[xindex][yindex];
                tmp[xindex][yindex]=NULL;
            }
        }
    }
    
    for(i=0;i<7;i++){
        for(j=0;j<7;j++){
            if(tmp[i][j]){
                tmp[i][j]->remove();
                removecChunk(tmp[i][j]);
            }
        }
    }
}

void terrain::genTerrain(
    float ** img,  //高度图边长=chunk边长+1
    irr::u32 x , irr::u32 y //chunk坐标，真实坐标/128
){
    int h;
    irr::video::SColor c;
    c.set(255);
    float ix,iy;
    float len=129.0f/(float)pointNum;
    
    for(int i=0;i<pointNum;i++){
        for(int j=0;j<pointNum;j++){
            ix=len*i;
            iy=len*j;
            h=getHillHight(ix,iy)+getAltitude(ix,iy);
            img[i][j]=h;
        }
    }
}
void terrain::chunk::add(
    int id,
    const irr::core::vector3df & p,
    const irr::core::vector3df & r,
    const irr::core::vector3df & s
){

}

void terrain::getItems(irr::u32 x , irr::u32 y , terrain::chunk * ch){
    for(auto it:m->mapGenFuncs){
        if(it)
            it(x,y,getTemperature(x,y),getHumidity(x,y),getHight(x,y),ch);
    }
}

void terrain::updateChunk(terrain::chunk * ch, irr::u32 x , irr::u32 y){
    auto driver=this->scene->getVideoDriver();
    
    genTerrain(ch->T , x ,y);
    
    float len=129.0f/(float)pointNum;
    ch->mesh=this->createTerrainMesh(
        this->texture ,
        ch->T , pointNum , pointNum ,
        irr::core::dimension2d<irr::f32>(len , len),
        this->scene->getVideoDriver(),
        irr::core::dimension2d<irr::u32>(pointNum , pointNum),
        true
    );
    ch->node=scene->addMeshSceneNode(
        ch->mesh,
        0,-1,
        irr::core::vector3df(x*128 , y*128 , 0)
    );
}

}//namespace smoothly

