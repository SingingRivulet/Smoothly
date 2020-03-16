#include "terrainDispather.h"
namespace smoothly{

terrainDispather::terrainDispather(){
    b.parent     = this;
    b.range      = 2;
    setCharNum   = 0;
}
terrainDispather::~terrainDispather(){
    
}

void terrainDispather::bDs::createChunk(int x , int y){
    parent->createChunk(x,y);
}
void terrainDispather::bDs::removeChunk(int x , int y){
    parent->releaseChunk(x,y);
}

bool terrainDispather::chunkLoaded(int x,int y){
    return b.chunkLoaded(x,y);
}

void terrainDispather::setVisualRange(int r){
    b.range      = r;
    visualRange  = r;
}

int terrainDispather::getVisualRange(){
    return visualRange;
}

}
