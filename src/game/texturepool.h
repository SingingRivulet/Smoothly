#ifndef SMOOTHLY_TEXTUREPOOL
#define SMOOTHLY_TEXTUREPOOL
#include <irrlicht.h>
#include <list>
#include <string>
namespace smoothly{

class texturepool{
    public:
        irr::video::IVideoDriver * driver;
        irr::video::ITexture * get(){
            if(textures.empty()){
                return createTexture();
            }else{
                auto it = textures.begin();
                auto ptr = *it;
                textures.erase(it);
                return ptr;
            }
        }
        void del(irr::video::ITexture * t){
            textures.push_back(t);
        }
        texturepool(int w,int h,const std::string & n){
            width = w;
            height = h;
            name = n;
            id = 0;
        }
        ~texturepool(){
        }
    private:
        std::list<irr::video::ITexture*> textures;
        int width;
        int height;
        std::string name;
        int id;

        irr::video::ITexture * createTexture(){
            char namebuf[128];
            snprintf(namebuf,128,"%s-%d",this->name.c_str(),++(this->id));
            return driver->addRenderTargetTexture(
                        irr::core::dimension2d<irr::u32>(this->width,this->height),
                        namebuf,
                        irr::video::ECF_A8R8G8B8);
        }
};

}
#endif // SMOOTHLY_TEXTUREPOOL
