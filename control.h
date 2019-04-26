#ifndef SMOOTHLY_CLIENT_CONTROL
#define SMOOTHLY_CLIENT_CONTROL
#include "view.h"
#include <queue>
namespace smoothly{
    class control:public view{
        public:
            
            class eventRecv:public irr::IEventReceiver{
                public:
                    control * parent;
                    virtual bool OnEvent (const irr::SEvent &event);
            };
            
            enum gameEvent{
                SET_BUILDING_MODE=1,
                START_BUILDING_MODE=2,
                BUILDING_APPLY=3
            };
            
            typedef std::pair<gameEvent,long> gemeEPair;
            std::queue<gemeEPair> eventQueue;
            
            
            control();
            ~control();
            
            struct Status{
                bool  moveFront,
                      moveBack,
                      moveLeft,
                      moveRight,
                      moveUp,
                      moveDown;
                inline Status(){
                    moveFront=false;
                    moveBack=false;
                    moveLeft=false;
                    moveRight=false;
                    moveUp=false;
                    moveDown=false;
                }
            }status;
            
            void activeApply();
            
            void moveUp();//飞行模式是升，普通模式是跳
            void moveDown();//飞行模式是降，普通模式是蹲
            
            bool flyMode;
            
            eventRecv receiver;
            void addCamera();
            void addEventRecv();
            
            void loop();
            
            inline bool ok(){
                return (device->run() && driver && dynamicsWorld);
            }
            
            long buildingSelection[7];
            int buildingSelected;
            inline void buildingSelectionInit(){
                buildingSelected=0;
                for(int i=0;i<7;i++)
                    buildingSelection[i]=m->defaultBuildingList[i];
            }
            inline void buildingModeBegin(){
                if(buildingSelected>=7 || buildingSelected<0)
                    return;
                if(buildingSelection[buildingSelected]==-1)
                    return;
                doBuildBegin(buildingSelection[buildingSelected]);
            }
    };
}
#endif