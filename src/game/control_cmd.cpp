#include "control.h"

namespace smoothly{

void control::processCmd(){
    auto ws = menu_cmd_line->getText();
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    auto str = conv.to_bytes(ws);
    std::cout<<"cmd:"<<str<<std::endl;
    std::istringstream iss(str);
    std::string key;
    iss>>key;

    if(key=="mission"){
        processCmd_mission(iss);
    }else if(key=="mail"){
        processCmd_mail(iss);
    }else if(key=="ik"){
        processCmd_ik(iss);
    }else if(key=="test"){
        processCmd_test(iss);

    }

    menu_cmd_line->setText(L"");
}

void control::processCmd_mission(std::istringstream & iss){
    std::string method;
    iss>>method;
    if(method=="goParent"){
        goParentMission();
    }else if(method=="remove"){
        std::string uuid;
        iss>>uuid;
        cmd_removeMission(uuid.c_str());
    }
}

void control::processCmd_mail(std::istringstream & iss){
    std::string method;
    iss>>method;
    if(method=="pickup"){
        std::string uuid;
        iss>>uuid;
        cmd_pickupMailPackage(mainControl.c_str() , uuid.c_str());
    }
}

void control::processCmd_ik(std::istringstream & iss){
    std::string method;
    iss>>method;
    if(mainControlBody){
        if(method=="add"){
            {
                int bid;
                std::string name;
                iss>>name;
                iss>>bid;
                mainControlBody->getIKEffector(bid,name);
            }
        }else if(method=="remove"){
            {
                std::string effname;
                iss>>effname;
                mainControlBody->removeIKEffector(effname);
            }
        }else if(method=="set"){
            {
                std::string method;
                iss>>method;
                std::string effname;
                iss>>effname;
                float x,y,z,w;
                auto it = mainControlBody->ik_effectors.find(effname);
                if(it!=mainControlBody->ik_effectors.end()){
                    ik_effector_t * ik_eff = it->second.first;
                    if(method=="position"){
                        iss>>x;
                        iss>>y;
                        iss>>z;
                        ik_eff->target_position = ik.vec3.vec3(x,y,z);
                    }else if(method=="rotation"){
                        iss>>x;
                        iss>>y;
                        iss>>z;
                        iss>>w;
                        ik_eff->target_rotation = ik.quat.quat(x,y,z,w);
                    }
                }
            }
        }
    }
}

void control::processCmd_test(std::istringstream & iss){
    std::string act;
    iss>>act;

    if(act=="dig"){
        if(mainControlBody){
            auto p = mainControlBody->node->getPosition();
            std::vector<std::pair<std::pair<int32_t, int32_t>, int16_t> > d;
            d.push_back(std::pair<std::pair<int32_t, int32_t>, int16_t>(std::pair<int32_t, int32_t>(p.X,p.Z),-10));
            cmd_setDig(d);
        }
    }
}

}
