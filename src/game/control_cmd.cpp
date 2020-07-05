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
    }

    menu_cmd_line->setText(L"");
}

void control::processCmd_mission(std::istringstream & iss){
    std::string method;
    iss>>method;
    if(method=="goParent"){
        goParentMission();
    }
}

}
