#ifndef SMOOTHLY_WORLD_TERRAIN_DISPATHER
#define SMOOTHLY_WORLD_TERRAIN_DISPATHER
#include <map>
#include <set>
#include <list>
#include <vector>
#include <queue>
#include <chrono>
#include <functional>
namespace smoothly{
namespace world{
namespace terrain{
class ipair{
    public:
        int32_t x,y;
        inline bool operator==(const ipair & i)const{
            return (x==i.x) && (y==i.y);
        }
        inline bool operator<(const ipair & i)const{
            if(x<i.x)
                return true;
            else
            if(x==i.x){
                if(y<i.y)
                    return true;
            }
                return false;
        }
        inline ipair & operator=(const ipair & i){
            x=i.x;
            y=i.y;
            return *this;
        }
        inline ipair(const ipair & i){
            x=i.x;
            y=i.y;
        }
        inline ipair(const int & ix , const int & iy){
            x=ix;
            y=iy;
        }
        inline ipair(){
            x=0;
            y=0;
        }
};
class dispatherQueue{
    private:
        std::queue<std::pair<bool,ipair> > datas;
        int startTime;
    public:
        dispatherQueue(){
            startTime = time(0);
        }
        void process(){
            clock_t starts,ends;
            starts=clock();
            while(1){
                ends = clock();
                auto dtm = (ends-starts)/(CLOCKS_PER_SEC/1000);
                if(dtm>1)
                    break;
                
                if(datas.empty())
                    break;
                
                auto t = datas.front();
                
                if(t.first)
                    createChunk(t.second.x , t.second.y);
                else
                    removeChunk(t.second.x , t.second.y);
                
                datas.pop();
            }
            
        }
        inline void createChunk_q(int x , int y){
            datas.push(std::pair<bool,ipair>(true,ipair(x,y)));
        }
        inline void removeChunk_q(int x , int y){
            datas.push(std::pair<bool,ipair>(false,ipair(x,y)));
        }
        
        virtual void createChunk(int x , int y)=0;
        virtual void removeChunk(int x , int y)=0;
};
class bodyDispather:public dispatherQueue{
    public:
        int range;
        inline void setPositionBegin(){
            cList.clear();
            rList.clear();
            cSet.clear();
            rSet.clear();
        }
        inline void removePosition(const std::string & name){
            auto it = watchPoint.find(name);
            if(it!=watchPoint.end()){
                pushRound(it->second.x , it->second.y , name , -1);
                watchPoint.erase(name);
            }
        }
        inline void setPosition(const std::string & name , int x,int y){
            ipair p(x,y);
            auto it = watchPoint.find(name);
            if(it==watchPoint.end()){
                //添加物体
                watchPoint[name] = p;
                createChunk(x , y);//先创建脚下的，不然会掉下去
                pushRound(x,y,name,1);
            }else{
                //更新物体
                if(x==it->second.x && y==it->second.y){
                    
                }else{
                    pushRound(it->second.x , it->second.y , name , -1);
                    createChunk(x , y);//先创建脚下的，不然会掉下去
                    pushRound(x,y,name,1);
                    watchPoint[name] = p;
                }
            }
        }
        inline void setPositionEnd(){
            listOutput();
            cList.clear();
            rList.clear();
            cSet.clear();
            rSet.clear();
        }
        inline bool chunkLoaded(int x,int y){
            ipair p(x,y);
            auto it = chunks.find(p);
            if(it == chunks.end()){
                return false;
            }else{
                return (it->second > 0);
            }
        }
    private:
        std::map<std::string,ipair> watchPoint;
        std::map<ipair,int> chunks;
        
        std::vector<ipair> cList,rList;
        std::set<ipair> cSet,rSet;
        inline void listOutput(){
            for(auto it:rList){
                if(rSet.find(it)!=rSet.end()){
                    removeChunk_q(it.x , it.y);
                }
            }
            for(auto it:cList){
                if(cSet.find(it)!=cSet.end()){
                    createChunk_q(it.x , it.y);
                }
            }
        }
        inline void incPosi(int x,int y,const std::string & ,int dt){
            ipair p(x,y);
            auto it = chunks.find(p);
            if(it==chunks.end()){//未观察的区域
                if(dt>0){
                    chunks[p]=dt;
                    cSet.insert(p);//防止多个物体同时移动时，造成资源浪费
                    rSet.erase(p);
                    cList.push_back(p);
                }
            }else{
                it->second+=dt;
                if(it->second<=0){
                    chunks.erase(it);//离开区域
                    rSet.insert(p);
                    cSet.erase(p);
                    rList.push_back(p);
                }
            }
        }
        inline void pushRound(int x,int y,const std::string & name,int dt){
            incPosi(x,y,name,dt);
            for(int i=1;i<=range;++i){
                {//a
                    int a=-i+1;
                    int A=i;
                    int ax=-i;
                    for(int j=a;j<=A;j++)
                        incPosi(ax+x,j+y,name,dt);
                }
                {//b
                    int b=-i+1;
                    int B=i;
                    int by=i;
                    for(int j=b;j<=B;j++)
                        incPosi(j+x,by+y,name,dt);
                }
                {//c
                    int c=i-1;
                    int C=-i;
                    int cx=i;
                    for(int j=c;j>=C;j--)
                        incPosi(cx+x,j+y,name,dt);
                }
                {//d
                    int d=i-1;
                    int D=-i;
                    int dy=-i;
                    for(int j=d;j>=D;j--)
                        incPosi(j+x,dy+y,name,dt);
                }
            }
        }
};
////////////////
}//////terrain
}//////world
}//////smoothly
#endif
