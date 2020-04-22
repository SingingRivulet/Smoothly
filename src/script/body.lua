package.path = package.path .. ";../script/?.lua;"
require("utils.print_r")
--[[
在此定义函数，用于设置角色动画
格式：
function callback(arg)
参数格式：
arg={
  [status] => {
                [posture] => "stand"            姿势
                [mask] => 0                     status的掩码
                [hand] => "none"                手的状态
                [useLeft] => false              使用左手
                [useRight] => false             使用右手
                [walk] => {                     行走状态
                            [leftOrRight] => 0  左右
                            [forward] => 0      前后
                          }
                [pitchAngle] => -23             俯仰角
              }
  [nowFrame] => 1.0                             当前所在的帧
  [finish] => false                             是否因为结束而触发的动画
}
返回值格式
    返回0个参数：继续之前的动画
    返回1个参数：{播放速度，起始帧，结束帧，当前帧，是否循环}
]]--
function snoutx10kCallback(arg)
    --print_r(arg)
    if arg.status.walk.forward==1 then

        return {["speed"]=80,["start"]=0,["end"]=80,["frame"]=arg.nowFrame,["loop"]=true}

    elseif arg.status.walk.forward==-1 then

        return {["speed"]=-80,["start"]=0,["end"]=80,["frame"]=arg.nowFrame,["loop"]=true}

    else

        return {["speed"]=1,["start"]=1,["end"]=1,["frame"]=1,["loop"]=true}

    end
end
