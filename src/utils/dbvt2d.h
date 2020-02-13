#ifndef SMOOTHLY_DBVT2D
#define SMOOTHLY_DBVT2D

#include <math.h>
#include <cmath>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>

namespace smoothly {

class dbvt2d{
    public:
        struct vec{
            int X,Y;
            vec(){
                X=0;
                Y=0;
            }
            vec(const vec & i){
                X=i.X;
                Y=i.Y;
            }
            vec(float iX , float iY){
                X=iX;
                Y=iY;
            }
            const vec & operator=(const vec & i){
                X=i.X;
                Y=i.Y;
                return * this;
            }

            void set(float iX , float iY){
                X=iX;
                Y=iY;
            }
            vec operator+(const vec & i)const{
                vec tmp;
                tmp.X = X+i.X;
                tmp.Y = Y+i.Y;
                return tmp;
            }

            vec operator-(const vec & i)const{
                vec tmp;
                tmp.X = X-i.X;
                tmp.Y = Y-i.Y;
                return tmp;
            }

            vec operator*(float i)const{
                vec tmp;
                tmp.X = X*i;
                tmp.Y = Y*i;
                return tmp;
            }

            vec operator/(float i)const{
                vec tmp;
                tmp.X = X/i;
                tmp.Y = Y/i;
                return tmp;
            }
        };
        class AABB{
            public:
                AABB * left ,
                * right ,
                * parent ,
                * next;

                dbvt2d * bb;

                vec from,to;

                void * data;

                bool insertFlag;

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
                    vec tf,tt;
                    tf.X = std::min(from.X, other->from.X);
                    tf.Y = std::min(from.Y, other->from.Y);

                    tt.X = std::max(to.X, other->to.X);
                    tt.Y = std::max(to.Y, other->to.Y);

                    auto l=tt-tf;
                    return (l.X*l.X + l.Y*l.Y);
                }

                inline void merge(const AABB * other , AABB * out)const{
                    out->from.X = std::min(from.X, other->from.X);
                    out->from.Y = std::min(from.Y, other->from.Y);

                    out->to.X = std::max(to.X, other->to.X);
                    out->to.Y = std::max(to.Y, other->to.Y);
                }

                inline bool isEmpty()const{
                    return from.X > to.X || from.Y > to.Y;
                }

                inline bool inBox(const vec & point)const{
                    return (
                                (point.X>=from.X && point.X<=to.X) &&
                                (point.Y>=from.Y && point.Y<=to.Y)
                                );
                }

                inline bool onStep(float p)const{
                    return (p>=from.X) && (p<=to.X);
                }

                inline bool intersects(const AABB * in)const{
                    return (
                                (from.X >= in->from.X && from.X <= in->to.X) ||
                                (in->from.X >= from.X && in->from.X <= to.X)
                                ) &&(
                                (from.Y >= in->from.Y && from.Y <= in->to.Y) ||
                                (in->from.Y >= from.Y && in->from.Y <= to.Y)
                                );
                }

                inline bool inBox(const AABB * in)const{
                    return (
                                ((from.X >= in->from.X) && (to.X <= in->to.X)) &&
                                ((from.Y >= in->from.Y) && (to.Y <= in->to.Y))
                                );
                }

                inline float getSizeSq()const{
                    auto l=to-from;
                    return (l.X*l.X + l.Y*l.Y);
                }

                inline vec getCenter() const{
                    return (from + to) / 2;
                }

                inline vec getExtent() const{
                    return to - from;
                }

                void construct(){
                    left=NULL;
                    right=NULL;
                    parent=NULL;
                    data=NULL;
                    insertFlag=false;
                    from.set(0,0);
                    to.set(0,0);
                }

                void collisionTest(
                        const AABB * in,
                        void(*callback)(AABB *,void *),
                        void * arg=NULL
                        );

                void fetchByPoint(
                        const vec & point,
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
        AABB * add(const vec & from,const vec & to,void * data);

        inline void collisionTest(
                const AABB * in,
                void(*callback)(AABB *,void *),
                void * arg=NULL
                ){
            root->collisionTest(in,callback,arg);
        }
        inline void fetchByPoint(
                const vec & point,
                void(*callback)(AABB *,void *),
                void * arg=NULL
                ){
            root->fetchByPoint(point,callback,arg);
        }
        dbvt2d();
        ~dbvt2d();

    private:
        void poolInit();
        void poolDestroy();
        void * pool;
};

}

#endif // SMOOTHLY_SERVER_DBVT2D
