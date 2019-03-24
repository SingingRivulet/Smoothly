#ifndef SMOOTHLY_BUILDING
#define SMOOTHLY_BUILDING
#include "terrain.h"
#include "hbb.h"
#include "remoteGraph.h"
namespace smoothly{
    class buildings:public terrain,public remoteGraph{
        public:
            
            HBB buildingHBB;
            int buildingDamage;
            
            class building{
                public:
                    
                    enum buildingType{
                        BUILDING_ITEM,BUILDING_TERRAIN
                    };
                    
                    buildingType type;
                    
                    building * next;
                    
                    void * data;
                    HBB::AABB * node;
                    
            };
            
            std::map<void*,building*> allBuildings;
            
            virtual void onGenChunk(terrain::chunk *);
            virtual void onFreeChunk(terrain::chunk *);
            
            virtual void onGenBuilding(remoteGraph::item *);
            virtual void onFreeBuilding(remoteGraph::item *);
            virtual void onDestroyBuilding(remoteGraph::item *);
            virtual void onCreateBuilding(remoteGraph::item *);
            
            virtual void onFreeTerrainItem(terrain::item *);
            virtual void onGenTerrainItem(terrain::item *);
            
            bool getNormalByRay(const irr::core::line3d<irr::f32> & ray,irr::core::line3d<irr::f32> &normal);
            void transformByNormal(irr::scene::IMeshSceneNode * n,const irr::core::line3d<irr::f32> & normal);
            bool canBuild(irr::scene::IMeshSceneNode * n,long type);
            
            void doBuildBegin(long type);
            void doBuildEnd();
            void doBuildUpdate(const irr::core::line3d<irr::f32> & ray);
            void doBuildApply();
            
            building * selectByRay(const irr::core::line3d<irr::f32> & ray);
            void attackByRay(const irr::core::line3d<irr::f32> & ray);
            
            virtual bool collisionWithObject(irr::scene::IMeshSceneNode * n);
            
            inline static bool intersectionTriangle(
                const irr::core::line3d<irr::f32> & ray,
                const irr::core::triangle3df & triangle,
                irr::core::vector3df & out
            ){
                return triangle.getIntersectionOfPlaneWithLine(ray.start,ray.end-ray.start,out);
            }
            
            inline static void getTriangleNormal(
                const irr::core::triangle3df & triangle,
                const irr::core::vector3df & near,
                irr::core::vector3df & out
            ){
                auto n=triangle.getNormal();
                auto ov=near-triangle.pointA;
                if(ov.X==0 && ov.Y==0 && ov.Z==0){
                    out=n;
                    return;
                }
                auto d=ov.dotProduct(n);
                if(d<=0)
                    out=-n;
                else
                    out=n;
            }
            
            virtual void buildOnFloor(
                long type,
                const irr::core::vector3df &,
                const irr::core::vector3df &
            )=0;
            virtual void buildOn(
                long type,
                const irr::core::vector3df &,
                const irr::core::vector3df &,
                const std::list<std::string> &
            )=0;
            
            buildings();
            ~buildings();
            
            float meshSize;
        private:
            
            building * collisionWithBuildings(const irr::core::line3d<irr::f32> & ray,irr::core::line3d<irr::f32> &normal);
            bool collisionWithBuildings(irr::scene::IMeshSceneNode * n,long type,std::list<building*> & b);
            bool collisionWithTerrain(irr::scene::IMeshSceneNode * n,long type);
            
            void * cBuildPool;
            building * createCBuild();
            void delCBuilding(building*);
            
            bool buildingMode;
            irr::core::line3d<irr::f32> cameraRay;
            long buildingType;
            irr::scene::IMeshSceneNode * buildingNode;
            mods::building * buildingConfig;
            bool allowBuild;
    };
}
#endif