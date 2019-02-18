#include <stdio.h>
#include "hbb.h"
using namespace smoothly;
int main(){
    HBB h;
    int a=1,b=2,c=3;
    auto p=h.add(HBB::vec3(-1,-1,-1),HBB::vec3(1,1,1),&a);
    p->autodrop();
    
    p=h.add(HBB::vec3(-1,-1,-1),HBB::vec3(1,1,1),&a);
    p=h.add(HBB::vec3(1,1,1),HBB::vec3(2,2,2),&b);
    p=h.add(HBB::vec3(1,-1,-1),HBB::vec3(2,1,1),&c);
    p=h.add(HBB::vec3(2,-1,-1),HBB::vec3(3,1,1),&c);
    p=h.add(HBB::vec3(2,1,-1),HBB::vec3(3,2,1),&c);
    
    irr::core::line3d<irr::f32> ray;
    ray.start.set(-1,0,0);
    ray.end.set(1,0,0);
    
    h.rayTest(ray,[](HBB::AABB * p,void *){
        auto ptr=(int*)(p->data);
        printf("search:%d\n",*ptr);
    });
    
    p->autodrop();
}