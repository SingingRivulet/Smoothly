#ifndef SMOOTHLY_CLIENT_CONTROL
#define SMOOTHLY_CLIENT_CONTROL
namespace smoothly{
    class control:public clientNetwork{
        public:
            void loadScene();
            void destroyScene();
            void loadWorld();
            void destroyWorld();
            
            void moveFront();
            void moveBack();
            void moveLeft();
            void moveRight();
            
            bool canFly;
            void moveUp();//飞行模式是升，普通模式是跳
            void moveDown();//飞行模式是降，普通模式是蹲
            
            void clickLeft();
            void clickRight();
    };
}
#endif