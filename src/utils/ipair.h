#ifndef SMOOTHLY_IPAIR
#define SMOOTHLY_IPAIR

namespace smoothly{

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

}

#endif // SMOOTHLY_IPAIR
