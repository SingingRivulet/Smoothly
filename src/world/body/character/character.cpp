#include "character.h"
namespace smoothly{
namespace world{
namespace body{
////////////////
using namespace std;
void character::toString(std::string & s){
    char buf[512];
    snprintf(buf,sizeof(buf),"%f %f %f %f %f %f %f %f %f",
        position.X,position.Y,position.Z,
        rotation.X,rotation.Y,rotation.Z,
        lookAt.X,lookAt.Y,lookAt.Z);
    s = buf;
    for(auto it:wearing){
        if(it){
            snprintf(buf,sizeof(buf)," %d",it);
            s+=buf;
        }
    }
}
void character::loadString(const std::string & s){
    istringstream iss(s);
    iss>>position.X;
    iss>>position.Y;
    iss>>position.Z;
    iss>>rotation.X;
    iss>>rotation.Y;
    iss>>rotation.Z;
    iss>>lookAt.X;
    iss>>lookAt.Y;
    iss>>lookAt.Z;
    int wd;
    wearing.clear();
    while(!iss.eof()){
        wd = 0;
        iss>>wd;
        if(wd!=0)
            wearing.insert(wd);
    }
}
void character::doString(const std::string & s){
    istringstream iss(s);
    string c;
    iss>>c;
    int d;
    vec3 v;
    if(c=="wa"){
        iss>>d;
        wearing_add(d);
    }else
    if(c=="wr"){
        iss>>d;
        wearing_remove(d);
    }else
    if(c=="la"){
        iss>>v.X;
        iss>>v.Y;
        iss>>v.Z;
        setLookAt(v);
    }else
    if(c=="mv"){
        iss>>v.X;
        iss>>v.Y;
        iss>>v.Z;
        setPosition(v);
    }else
    if(c=="rt"){
        iss>>v.X;
        iss>>v.Y;
        iss>>v.Z;
        setRotation(v);
    }else
    if(c=="ia"){
        std::string c;
        iss>>c;
        interactive(c);
    }
}
void character::wearing_add(int d){
    wearing.insert(d);
}
void character::wearing_remove(int d){
    wearing.erase(d);
}
void character::setLookAt(const vec3 & v){
    lookAt = v;
}
void character::setPosition(const vec3 & v){
    position = v;
}
void character::setRotation(const vec3 & v){
    rotation = v;
}
namespace cmd{
    std::string wearing_add(int d){
        char buf[32];
        snprintf(buf,sizeof(buf),"wa %d",d);
        return std::string(buf);
    }
    std::string wearing_remove(int d){
        char buf[512];
        snprintf(buf,sizeof(buf),"wr %d",d);
        return std::string(buf);
    }
    std::string setLookAt(const vec3 & v){
        char buf[512];
        snprintf(buf,sizeof(buf),"la %f %f %f" , v.X , v.Y , v.Z);
        return std::string(buf);
    }
    std::string setPosition(const vec3 & v){
        char buf[512];
        snprintf(buf,sizeof(buf),"mv %f %f %f" , v.X , v.Y , v.Z);
        return std::string(buf);
    }
    std::string setRotation(const vec3 & v){
        char buf[512];
        snprintf(buf,sizeof(buf),"rt %f %f %f" , v.X , v.Y , v.Z);
        return std::string(buf);
    }
    std::string interactive(const std::string & s){
        return std::string("ia ")+s;
    }
}//////cmd
////////////////
}//////body
}//////world
}//////smoothly
