#ifndef SMOOTHLY_HBB
#define SMOOTHLY_HBB
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
namespace smoothly{
    class HBB{
        public:
            
            typedef irr::core::vector3df vec3;
            
            class AABB{
                public:
                    
                    AABB * left ,
                         * right ,
                         * parent ,
                         * next;
                    
                    HBB * hbb;
                    
                    vec3 from,to;
                    
                    void * data;
                    
                    inline bool isDataNode(){
                        return data!=NULL;
                    }
                    
                    inline void setLeft(AABB * in){
                        left=in;
                        in->parent=this;
                    }
                    inline void setRight(AABB * in){
                        right=in;
                        in->parent=this;
                    }
                    
                    inline float getMergeSizeSq(const AABB * other)const{
                        vec3 tf,tt;
                        tf.X = std::min(from.X, other->from.X);
                        tf.Y = std::min(from.Y, other->from.Y);
                        tf.Z = std::min(from.Z, other->from.Z);
 
                        tt.X = std::max(to.X, other->to.X);
                        tt.Y = std::max(to.Y, other->to.Y);
                        tt.Z = std::max(to.Z, other->to.Z);
                        
                        auto l=tt-tf;
                        return (l.X*l.X + l.Y*l.Y + l.Z*l.Z);
                    }
                    
                    inline void merge(const AABB * other , AABB * out)const{
                        out->from.X = std::min(from.X, other->from.X);
                        out->from.Y = std::min(from.Y, other->from.Y);
                        out->from.Z = std::min(from.Z, other->from.Z);
 
                        out->to.X = std::max(to.X, other->to.X);
                        out->to.Y = std::max(to.Y, other->to.Y);
                        out->to.Z = std::max(to.Z, other->to.Z);
                    }
                    
                    inline bool isEmpty()const{
                        return from.X > to.X || from.Y > to.Y || from.Z > to.Z;
                    }
                    
                    inline bool inBox(const vec3 & point)const{
                        return (
                            (point.X>=from.X && point.X<=to.X) &&
                            (point.Y>=from.Y && point.Y<=to.Y) &&
                            (point.Z>=from.Z && point.Z<=to.Z)
                        );
                    }
                    
                    inline bool intersects(const AABB * in)const{
                        return (
                            (from.X >= in->from.X && from.X <= in->to.X) || 
                            (in->from.X >= from.X && in->from.X <= to.X)
                        ) &&(
                            (from.Y >= in->from.Y && from.Y <= in->to.Y) || 
                            (in->from.Y >= from.Y && in->from.Y <= to.Y)
                        ) &&(
                            (from.Z >= in->from.Z && from.Z <= in->to.Z) || 
                            (in->from.Z >= from.Z && in->from.Z <= to.Z)
                        );
                    }
                    
                    inline bool inBox(const AABB * in)const{
                        return (
                            ((from.X >= in->from.X) && (to.X <= in->to.X)) &&
                            ((from.Y >= in->from.Y) && (to.Y <= in->to.Y)) &&
                            ((from.Z >= in->from.Z) && (to.Z <= in->to.Z))
                        );
                    }
                    
                    inline float getSizeSq()const{
                        auto l=to-from;
                        return (l.X*l.X + l.Y*l.Y + l.Z*l.Z);
                    }
                    
                    inline vec3 getCenter() const{
                        return (from + to) / 2;
                    }
                    
                    inline vec3 getExtent() const{
                        return to - from;
                    }
                    
                    inline bool intersects(
                        const vec3& linemiddle,
                        const vec3& linevect,
                        float halflength
                    ) const{
                        const vec3 e = getExtent() * 0.5f;
                        const vec3 t = getCenter() - linemiddle;

                        if ((fabs(t.X) > e.X + halflength * fabs(linevect.X)) ||
                            (fabs(t.Y) > e.Y + halflength * fabs(linevect.Y)) ||
                            (fabs(t.Z) > e.Z + halflength * fabs(linevect.Z)) 
                        )
                        return false;

                        auto r = e.Y * fabs(linevect.Z) + e.Z * fabs(linevect.Y);
                        if (fabs(t.Y*linevect.Z - t.Z*linevect.Y) > r )
                            return false;

                        r = e.X * fabs(linevect.Z) + e.Z * fabs(linevect.X);
                        if (fabs(t.Z*linevect.X - t.X*linevect.Z) > r )
                            return false;

                        r = e.X * fabs(linevect.Y) + e.Y * fabs(linevect.X);
                        if (fabs(t.X*linevect.Y - t.Y*linevect.X) > r)
                            return false;

                        return true;
                    }
                    
                    inline bool intersects(const irr::core::line3d<irr::f32> & line)const{
                        return intersects(
                            line.getMiddle(),
                            line.getVector().normalize(),
                            line.getLength() * 0.5f
                        );
                    }
                    
                    void construct(){
                        left=NULL;
                        right=NULL;
                        parent=NULL;
                        data=NULL;
                        from.set(0,0,0);
                        to.set(0,0,0);
                    }
                    
                    void rayTest(
                        const irr::core::line3d<irr::f32> & ray,
                        void(*callback)(AABB *,void *),
                        void * arg=NULL
                    );
                    
                    void collisionTest(
                        const AABB * in,
                        void(*callback)(AABB *,void *),
                        void * arg=NULL
                    );
                    
                    void fetchByPoint(
                        const vec3 & point,
                        void(*callback)(AABB *,void *),
                        void * arg=NULL
                    );
                    
                    void autoclean();
                    void autodrop();
                    void add(AABB * in);
                    void remove();
                    void drop();
            };
            
            AABB * root;
            
            AABB * createAABB();
            void   delAABB(AABB *);
            
            void add(AABB * in);
            void remove(AABB * in);
            AABB * add(const vec3 & from,const vec3 & to,void * data);
            
            inline void rayTest(
                const irr::core::line3d<irr::f32> & ray,
                void(*callback)(AABB *,void *),
                void * arg=NULL
            ){
                root->rayTest(ray,callback,arg);
            }
            
            inline void collisionTest(
                const AABB * in,
                void(*callback)(AABB *,void *),
                void * arg=NULL
            ){
                root->collisionTest(in,callback,arg);
            }
            inline void fetchByPoint(
                const vec3 & point,
                void(*callback)(AABB *,void *),
                void * arg=NULL
            ){
                root->fetchByPoint(point,callback,arg);
            }
            HBB();
            ~HBB();
            
        private:
            
            void poolInit();
            void poolDestroy();
            void * pool;
            
    };
}
#endif