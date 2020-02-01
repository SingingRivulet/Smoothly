#include "terrainDispather.h"
namespace smoothly{

terrainDispather::terrainDispather(){
    b.parent = this;
    v.parent = this;
}
terrainDispather::~terrainDispather(){
    
}

void terrainDispather::bDs::createChunk(int x , int y){
    parent->createChunk(x,y);
}
void terrainDispather::bDs::removeChunk(int x , int y){
    parent->releaseChunk(x,y);
}

void terrainDispather::vDs::createChunk(int x , int y){
    parent->showChunk(x,y);
}
void terrainDispather::vDs::removeChunk(int x , int y){
    parent->hideChunk(x,y);
}

bool terrainDispather::chunkLoaded(int x,int y){
    return b.chunkLoaded(x,y);
}

}
