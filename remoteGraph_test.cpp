#include "remoteGraph.h"
using namespace std;
using namespace smoothly;
class rgh:public remoteGraph{
    public:
        virtual void onGenBuilding(remoteGraph::item * i){
            printf("gen:%s\n",i->uuid.c_str());
        }
        virtual void onFreeBuilding(remoteGraph::item * i){
            printf("free:%s\n",i->uuid.c_str());
        }
        virtual void onDestroyBuilding(remoteGraph::item * i){
            printf("destroy:%s\n",i->uuid.c_str());
        }
        virtual void onCreateBuilding(remoteGraph::item * i){
            printf("create:%s\n",i->uuid.c_str());
        }
        virtual void downloadBuilding(int x,int y){
            printf("download:%d %d\n",x,y);
        }
        virtual void uploadBuilding(remoteGraph::item * i,uploadMode m){
            printf("upload:%s ",i->uuid.c_str());
            if(m==ADD_ITEM)
                printf("add\n");
            else
                printf("remove\n");
        }
        virtual item * downloadBuilding(const std::string & uuid){
            printf("download:%s\n",uuid.c_str());
            return NULL;
        }
        virtual void uploadAttack(const std::string & uuid , int hurt){
            printf("upload attack:%s %d\n",uuid.c_str(),hurt);
        }
};
int main(){
    rgh R;
    set<string> buildIn;
    printf("update\n");
    R.updateBuildingChunks(0,0,2);
    buildIn.clear();
    printf("add1\n");
    auto p=R.addNode(irr::core::vector3df(0,0,0),irr::core::vector3df(0,0,0),buildIn,1,1);
    R.buildingApplay();
    buildIn.insert(p->uuid);
    printf("add2\n");
    R.addNode(irr::core::vector3df(0,0,1),irr::core::vector3df(0,0,0),buildIn,1,1);
    R.buildingApplay();
    printf("attack1\n");
    //R.attackNode(p->uuid,10);
    R.buildingApplay();
    printf("size:%lu\n",R.items.size());
    printf("free\n");
    R.destroy();
}