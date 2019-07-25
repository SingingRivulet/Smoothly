#include "cJSON.h"
namespace smoothly{
    struct attachingStatus{
        int attOn,objId;
        std::string uuid;
        inline attachingStatus(){
            attOn=0;
            objId=0;
            uuid.clear();
        }
        inline attachingStatus(const attachingStatus & i){
            
        }
        inline attachingStatus(int iattOn,int iobjId,const std::string & iuuid){
            attOn=iattOn;
            objId=iobjId;
            uuid =iuuid;
        }
        inline const attachingStatus & operator=(const attachingStatus & i){
            attOn=i.attOn;
            objId=i.objId;
            uuid =i.uuid;
            return *this;
        }
        inline bool operator!=(const attachingStatus & i)const{
            return (attOn!=i.attOn || objId!=i.objId || uuid!=i.uuid);
        }
        inline bool operator==(const attachingStatus & i)const{
            return (attOn==i.attOn && objId==i.objId && uuid==i.uuid);
        }
        inline bool operator<(const attachingStatus & i)const{
            if(attOn<i.attOn)
                return true;
            else
            if(attOn==i.attOn){
                if(objId<i.objId)
                    return true;
                else
                if(objId==i.objId){
                    if(uuid<i.uuid)
                        return true;
                }
            }
            return false;
        }
        inline bool loadBitStream(RakNet::BitStream * data){
            RakNet::RakString u;
            int32_t b1,b2;
            if(!data->Read(b1))return false;
            if(!data->Read(b2))return false;
            if(!data->Read(u)) return false;
            attOn=b1;
            objId=b2;
            uuid=u.C_String();
            return true;
        }
        inline void toBitStream(RakNet::BitStream * data)const{
            RakNet::RakString u=uuid.c_str();
            data->Write((int32_t)attOn);
            data->Write((int32_t)objId);
            data->Write(u);
        }
    };
    struct bodyAttaching{
        //角色状态机
        //发送接收，比较更新差异
        private:
        std::set<attachingStatus> attachings;
        
        public:
        
        inline void add(int attOn,int objId,const std::string & uuid){
            attachings.insert(attachingStatus(attOn,objId,uuid));
        }
        
        inline void clear(){
            attachings.clear();
        }
        
        inline void loadString(const std::string & str){
            cJSON * json=cJSON_Parse(str.c_str());
            if(json){
                cJSON * item;
                cJSON * c=json->child;
                while (c){
                    if(c->type==cJSON_Array){
                        cJSON * p=c->child;
                        
                        if(p==NULL)                 goto end;
                        if(p->type!=cJSON_Number)   goto end;
                        int attOn=p->valueint;
                        p=p->next;
                        
                        if(p==NULL)                 goto end;
                        if(p->type!=cJSON_Number)   goto end;
                        int objId=p->valueint;
                        p=p->next;
                        
                        if(p==NULL)                 goto end;
                        if(p->type!=cJSON_String)   goto end;
                        const char * uuid=p->valuestring;
                        if(uuid==NULL)              goto end;
                        
                        attachings.insert(attachingStatus(attOn,objId,uuid));
                    }
                    end:
                    c=c->next;
                }
                cJSON_Delete(json);
            }
        }
        
        inline void toString(std::string & str)const{
            cJSON *json=cJSON_CreateArray();
            char * out;
            for(auto it:attachings){
                cJSON * p=cJSON_CreateArray();
                cJSON_AddItemToArray(p,cJSON_CreateNumber(it.attOn));
                cJSON_AddItemToArray(p,cJSON_CreateNumber(it.objId));
                cJSON_AddItemToArray(p,cJSON_CreateString(it.uuid.c_str()));
                cJSON_AddItemToArray(json,p);
            }
            out=cJSON_PrintUnformatted(json);
            str=out;
    
            cJSON_Delete(json);
            free(out);
        }
        
        inline bool loadBitStream(RakNet::BitStream * data){
            int32_t num;
            attachingStatus buf;
            if(!data->Read(num))return false;
            for(auto i=0;i<num;++i){
                if(!buf.loadBitStream(data))return true;
                attachings.insert(buf);
            }
            return true;
        }
        
        inline void toBitStream(RakNet::BitStream * data)const{
            data->Write((int32_t)attachings.size());
            for(auto it:attachings){
                it.toBitStream(data);
            }
        }
        
        inline void diff(const bodyAttaching & b,std::list<attachingStatus> & added,std::list<attachingStatus> & removed){
            //从自身变化至目标，需要的变化
            std::set_difference(
                //removed=self-b
                //自身有，目标没有，需要删除
                attachings.begin() , attachings.end(),
                b.attachings.begin() , b.attachings.end(),
                back_inserter(removed)
            );
            std::set_difference(
                //added=b-self
                //目标有，自身没有，需要添加
                b.attachings.begin() , b.attachings.end(),
                attachings.begin() , attachings.end(),
                back_inserter(added)
            );
        }
        
        inline bodyAttaching(const bodyAttaching & b):attachings(b.attachings){}
        inline bodyAttaching():attachings(){}
        inline ~bodyAttaching(){}
        
        inline bool operator=(const bodyAttaching & b){
            attachings=b.attachings;
        }
        
        inline const std::set<attachingStatus> & getList()const{
            return attachings;
        }
    };
}