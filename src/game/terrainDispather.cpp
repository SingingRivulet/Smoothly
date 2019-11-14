#include "terrainDispather.h"
namespace smoothly{

terrainDispather::terrainDispather(){
    b->parent = this;
    v->parent = this;
}
terrainDispather::~terrainDispather(){
    
}

void terrainDispather::bDs::createChunk(int x , int y){
    createChunk(x,y);
}
void terrainDispather::bDs::removeChunk(int x , int y){
    releaseChunk(x,y);
}

void terrainDispather::vDs::createChunk(int x , int y){
    showChunk(x,y);
}
void terrainDispather::vDs::removeChunk(int x , int y){
    hideChunk(x,y);
}

}