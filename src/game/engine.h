#ifndef SMOOTHLY_ENGINE
#define SMOOTHLY_ENGINE
#include "physical.h"
#include "../libclient/terrainGen.h"
#include "RealisticWater.h"
#include "CGUITTFont.h"
#include <vector>
#include <AL/alut.h>
#include "graphbase.h"
#include "localLight.h"

namespace smoothly{
    inline void rotate2d(irr::core::vector2df & v,double a){
        auto cosa=cos(a);
        auto sina=sin(a);
        auto x=v.X*cosa - v.Y*sina;
        auto y=v.X*sina + v.Y*cosa;
        v.X=x;
        v.Y=y;
        //v.normalize();
    }
    inline float mhtDist(const irr::core::vector3df & A,const irr::core::vector3df & B){
        irr::core::vector3df d=A-B;
        return fabs(d.X)+fabs(d.Y)+fabs(d.Z);
    }
    class engine:public physical,public smoothly::world::terrain::terrainGen,public graphBase{
        public:
            irr::gui::CGUITTFont * ttf;
            irr::IrrlichtDevice       * device;
            irr::scene::ISceneManager * scene;//场景
            irr::gui::IGUIEnvironment * gui;
            irr::ITimer               * timer;
            irr::video::IVideoDriver * driver;
            irr::scene::ICameraSceneNode * camera;
            irr::scene::ISceneCollisionManager * collisionManager;
            irr::core::stringc          vendor;

            irr::s32 shadowMapSize,shadowArea;

            irr::core::vector2di screenCenter;

            localLight lightManager;

            btDiscreteDynamicsWorld   * dynamicsWorld;
            btDefaultCollisionConfiguration* collisionConfiguration;
            btCollisionDispatcher* dispatcher;
            btBroadphaseInterface* overlappingPairCache;
            btSequentialImpulseConstraintSolver* solver;

            engine();
            ~engine();
            virtual void sceneLoop();
            virtual void worldLoop();
            virtual void onDraw();
            void deltaTimeUpdate();

            inline float getDeltaTime(){
                return deltaTime;
            }

            virtual void onCollision(btPersistentManifold * contactManifold)=0;

            irr::f32 waterLevel;

            irr::s32 width,height;

            bool running;

            irr::gui::IGUIFont * font;

            void drawArcProgressBar(core::position2d<s32> center,
                f32 radius, video::SColor color, s32 count,f32 progress);

            //音频
            ALCcontext * audioContext;
            ALCdevice *  audioDevice;

            struct audioBuffer:public irr::IReferenceCounted{//音频buffer
                    ALuint alBuffer;
                    inline audioBuffer(const char * path){
                       alBuffer = alutCreateBufferFromFile(path);
                    }
                    inline ~audioBuffer(){
                        alDeleteBuffers(1,&alBuffer);
                    }
            };
            std::map<std::string,audioBuffer*> audioBuffers;
            audioBuffer * getAudioBuffer(const char * path);
            struct audioSource:public irr::IReferenceCounted{//发声地点
                    ALuint alSource;
                    audioBuffer * playingBuffer;

                    inline audioSource(){
                        alGenSources(1, &alSource);
                        playingBuffer = NULL;
                    }
                    inline ~audioSource(){
                        stop();//先停止，再删除
                        alDeleteSources(1,&alSource);
                    }
                    inline void play(audioBuffer * buffer,ALboolean loop){
                        stop();//先停止
                        playingBuffer = buffer;
                        buffer->grab();
                        alSourcei(alSource, AL_BUFFER, buffer->alBuffer);//buffer
                        alSourcei(alSource, AL_LOOPING, loop);           //循环
                        alSourcePlay(alSource);                          //播放
                    }
                    inline void stop(){
                        if(playingBuffer){
                            alSourceStop(alSource);
                            playingBuffer->drop();
                            playingBuffer=NULL;
                        }
                    }
                    inline bool isPlaying(){
                        ALint sourceState;
                        alGetSourcei(alSource, AL_SOURCE_STATE, &sourceState);
                        return (sourceState == AL_PLAYING);
                    }
                    inline void setPosition(const irr::core::vector3df & posi){
                        alSource3f(alSource,AL_POSITION, posi.X, posi.Y, posi.Z);
                    }
                    inline void setVelocity(const irr::core::vector3df & velo){
                        alSource3f(alSource,AL_VELOCITY, velo.X, velo.Y, velo.Z);
                    }
            };
            void playAudioPosition(const irr::core::vector3df & posi,audioBuffer * buf);

        public:
            irr::u32 scan_animation_time;
            irr::s32 scan_animation_showing;
            irr::f32 scan_animation_size;

        public:
            irr::video::ITexture * post_tex , * post_depth , * post_mat , * post_normal , * post_ssao , * post_posi , * post_ssrt;
            irr::video::IRenderTarget * post;//后期
            irr::video::SMaterial postMaterial;//最终后期
            irr::video::SMaterial lightMaterial;//延迟光照
            irr::video::SMaterial ssaoMaterial;//ssao
            irr::video::SMaterial ssrtMaterial;//ssrt

            bool haveSSAO;
            bool haveSSRTGI;

            class PostShaderCallback:public irr::video::IShaderConstantSetCallBack{//shader回调
                public:
                    engine * parent;
                    void OnSetConstants(irr::video::IMaterialRendererServices * services, irr::s32 userData)override;
                    irr::core::vector3df lightPos;
                    irr::core::vector3df lightColor;
                    irr::f32 lightRange;
                    bool lightMode;
                    bool finalPass;
            }postShaderCallback;

        private:
            std::list<audioSource*> playingSources;//正在播放的声音（由系统接管）
            void updateListener();

        private:
            float deltaTime,lastTime;
            bool deltaTimeUpdateFirst;
            RealisticWaterSceneNode * water;
            int lastFPS;
            virtual void renderSky()=0;
            virtual void renderShadow()=0;
            virtual void renderMiniMap()=0;

        private:
            void loadConfig();

    };
}
#endif
