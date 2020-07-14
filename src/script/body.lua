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

        return {["speed"]=80,["start"]=0,["end"]=80,["frame"]=arg.nowFrame,["loop"]=true,["blend"]={1}}

    elseif arg.status.walk.forward==-1 then

        return {["speed"]=-80,["start"]=0,["end"]=80,["frame"]=arg.nowFrame,["loop"]=true,["blend"]={1}}

    else

        return {["speed"]=1,["start"]=1,["end"]=1,["frame"]=1,["loop"]=true,["blend"]={1}}

    end
end

--[[
arg = {
  [target] => table: 0x7f7cfcbb51f0 {
                [1] => 0.0
                [2] => 0.0
                [3] => 0.0
              }
  [haveTarget] => false
  [hitBody] => false
  [position] => table: 0x7f7cfcbb51f0 {
                  [1] => -2251.4809570312
                  [2] => 20.113748550415
                  [3] => 3732.8671875
                }
  [hitBuilding] => false
  [body] => userdata: 0x7f7d0839afd0
  [uuid] => "0c852b93-548c-4190-a17f-a63107eb31ff"
  [pathFindingMode] => false
  [onFloor] => true
  [hitTerrainItem] => false
}

bodyAPI = {
  [addStatus] => function: 0x5640e2cc2f43
  [pushCommond] => function: 0x5640e2cc2960
  [status] => table: 0x7fa0f4da0be0 {
                [BM_RIDE] => 8
                [BM_WALK] => 122880
                [BM_HAND_RIGHT] => 32
                [BM_ACT_SHOT_L] => 512
                [BM_ACT_CHOP] => 4096
                [BM_VANISH] => 1
                [BM_HAND_LEFT] => 16
                [BM_WALK_R] => 65536
                [BM_SIT] => 4
                [BM_AIM] => 64
                [BM_LIFT] => 0
                [BM_LIE] => 0
                [BM_WALK_L] => 32768
                [BM_BUILD] => 256
                [BM_WALK_B] => 16384
                [BM_WALK_F] => 8192
                [BM_ACT_SHOT_R] => 1024
                [BM_THROW] => 128
                [BM_ACT_THROW] => 2048
                [BM_OPERATE] => 0
                [BM_HAND_BOTH] => 0
                [BM_BUILDP] => 0
                [BM_LIEP] => 0
                [BM_SQUAT] => 2
              }
}
]]--
function defaultAI(arg)
    if arg.pathFindingMode then
        local deltaX = arg.pathFindingTarget[1]-arg.position[1]
        local deltaY = arg.pathFindingTarget[2]-arg.position[2]
        local deltaZ = arg.pathFindingTarget[3]-arg.position[3]
        if deltaX*deltaX + deltaY*deltaY + deltaZ*deltaZ >1 then
            bodyAPI.pushCommond(arg.body , {
                ["cmd"]="status_add" ,
                ["int"]=bodyAPI.status.BM_WALK_F
            })
            bodyAPI.pushCommond(arg.body , {
                ["cmd"]="lookat" ,
                ["vec"]={deltaX , deltaY , deltaZ}
            })
            if deltaY>1 then
                bodyAPI.pushCommond(arg.body , {
                    ["cmd"]="jump",
                    ["vec"]={0,1,0}
                })
            end
        end
    elseif arg.haveFollow then
        local deltaX = arg.followTarget[1]-arg.position[1]
        local deltaY = arg.followTarget[2]-arg.position[2]
        local deltaZ = arg.followTarget[3]-arg.position[3]
        if deltaX*deltaX + deltaY*deltaY + deltaZ*deltaZ >27 then
            bodyAPI.pushCommond(arg.body , {
                ["cmd"]="status_add" ,
                ["int"]=bodyAPI.status.BM_WALK_F
            })
            bodyAPI.pushCommond(arg.body , {
                ["cmd"]="lookat" ,
                ["vec"]={deltaX , deltaY , deltaZ}
            })
            if deltaY>1 then
                bodyAPI.pushCommond(arg.body , {
                    ["cmd"]="jump",
                    ["vec"]={0,1,0}
                })
            end
        end
    else
        bodyAPI.pushCommond(arg.body , {
            ["cmd"]="status_remove" ,
            ["int"]=bodyAPI.status.BM_WALK
        })
    end
end
