#include "body.h"
namespace smoothly{

void body::loop(){
    terrainDispather::loop();
    for(auto it:bodies){
        bodyItem * b = it.second;
        auto p = b->node->getPosition();
        auto cp=camera->getPosition();
        int l = std::max(abs(p.X-cp.X),abs(p.Z-cp.Z));
        if(l<getVisualRange()){
            //if(mainControlBody==b)
            //    b->node->setVisible(false);
            //else
            //    b->node->setVisible(true);
        }else{
            b->node->setVisible(false);
        }
        if(b->uncreatedChunk){
            int cx = floor(b->lastPosition.X/32);
            int cy = floor(b->lastPosition.Z/32);
            if(chunkCreated(cx,cy)){
                b->m_character.setGravity(gravity);
                b->uncreatedChunk = false;
            }else{
                b->m_character.setPosition(b->lastPosition);
                b->updateFromWorld();
                continue;
            }
        }
        b->walk(b->status.walk_forward , b->status.walk_leftOrRight,b->config->walkVelocity);
        b->updateFromWorld();
        auto h = getRealHight(p.X,p.Z);
        if(p.Y<h){
            if(b->lastPosition.Y>=h){
                p.Y = b->lastPosition.Y;
            }else{
                p.Y = (b->config->height+b->config->width)*0.5+h;//防止掉出地图
            }
            b->m_character.setPosition(p);
        }
    }
    for(auto it:myBodies){
        bodyItem * b = it.second;
        if(b->owner == myUUID && (!myUUID.empty())){//是自己拥有的

            //设置行走
            //b->walk(b->status.walk_forward , b->status.walk_leftOrRight,b->config->walkVelocity);
            //转移到上面了
            //b->walk(b->status.walk_forward , b->status.walk_leftOrRight,b->config->walkVelocity*timer->getRealTime());

            //设置dispather
            btTransform transform;
            b->m_character.getTransform(transform);

            btVector3 btPos;
            btPos = transform.getOrigin();
            setCharacterChunk(b->uuid,floor(btPos.x()/32),floor(btPos.z()/32));

            if(b->firing)
                b->doFire();

            //ai控制
            if(mainControlBody!=b){
                commond cmd;//命令结构体
                cmd.uuid = b->uuid;
                while(!b->autoWalk.empty()){
                    auto target = b->autoWalk.front();
                    vec3 posi(btPos.x(), btPos.y(), btPos.z());
                    vec3 dir = target-posi;

                    if(dir.getLengthSQ()<2*2){
                        b->autoWalk.pop_front();//到达目标，删除任务
                    }else{

                        //行走
                        cmd.data_int = BM_WALK_F;
                        cmd.cmd = CMD_STATUS_ADD;
                        pushCommond(cmd);

                        //更新旋转
                        cmd.cmd = CMD_SET_LOOKAT;
                        cmd.data_vec = dir;
                        pushCommond(cmd);

                        if(dir.Y>0){//目标点高于现在，跳跃
                            cmd.cmd = CMD_JUMP;
                            cmd.data_vec.set(dir);
                            pushCommond(cmd);
                        }
                    }
                }
                if(b->autoWalk.empty()){//停止自动行走
                    cmd.data_int = BM_WALK_F|BM_WALK_B|BM_WALK_L|BM_WALK_R;
                    cmd.cmd = CMD_STATUS_REMOVE;
                    pushCommond(cmd);
                }
            }
        }
    }
    doCommonds();
}

}
