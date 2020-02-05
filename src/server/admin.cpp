#include "admin.h"
#include <QString>
namespace smoothly{
namespace server{

admin::admin(){
    adminMap_init();
    printf(L_GREEN "[status]" NONE "get admin keys\n" );
    auto fp = fopen("../config/adminkey.txt" , "r");
    if(fp){
        keys.clear();
        char buf[128];
        while(!feof(fp)){
            bzero(buf,sizeof(buf));
            fgets(buf,sizeof(buf),fp);
            if(strlen(buf)>1){
                QString s=buf;
                s = s.trimmed();
                keys.insert(s.toStdString());
            }
        }
        int num = keys.size();
        printf(L_BLUE "[success]" NONE "load %d keys\n" , num);
        fclose(fp);
    }else{
        printf(L_RED "[error]" NONE "fail to read ../config/adminkey.txt\n" );
        logger->error("fail to read ../config/adminkey.txt");
    }
}

void admin::adminMessage(RakNet::BitStream * data, const RakNet::SystemAddress & addr){
    printf(L_BLUE "[admin]" NONE "access admin\n");
    RakNet::RakString key,activity;
    data->Read(key);
    if(!checkKey(key.C_String())){
        char addrStr[64];
        addr.ToString(true,addrStr);
        logger->error("use admin fail:{} key={}",addrStr,key.C_String());
        return;
    }
    data->Read(activity);
    admin_handler(activity.C_String() , data , addr);
}

bool admin::checkKey(const std::string &k){
    keyLocker.lock();
    auto it = keys.find(k);
    if(it==keys.end()){
        keyLocker.unlock();
        return false;
    }else{
        keyLocker.unlock();
        return true;
    }
}

}//////server
}//////smoothly
