#include "user.h"
namespace smoothly{

bool users::getUUIDByAddr(std::string & uuid , const RakNet::SystemAddress & addr){
    
}
bool users::getAddrByUUID(RakNet::SystemAddress & addr , const std::string & uuid){
    
}
void users::setOwner(const std::string & subsuuid,const std::string & useruuid){
    
}
void users::removeFromOwner(const std::string & subsuuid,const std::string & useruuid){
    
}
bool users::userCanPutSubs(const std::string & useruuid){
    
}
void users::moveUserPosition(const std::string & uuid,const std::string & muuid,const irr::core::vector3df & posi){
    
}

void users::login(const std::string & uuid,RakNet::SystemAddress address,const std::string & pwd){
    
}
void users::logout(const RakNet::SystemAddress & addr){
    
}
void users::createUser(const std::string & uuid,const std::string & pwd){
    
}
void users::changePwd(const std::string & uuid,const std::string & pwd){
    
}
void users::getChunkSubs(const std::string & uuid, const RakNet::SystemAddress address , int x,int y){
    
}

}