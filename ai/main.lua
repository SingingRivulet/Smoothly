--[[
自定义ai行为

arg = {
[body] => userdata: 0x7f81e6170760
[onFloor] => true
[haveFollow] => false
[pathFindingTarget] => table: 0x7f81d6155380 {
                         [1] => 0.0
                         [2] => 0.0
                         [3] => 0.0
                       }
[pathFindingMode] => false
[position] => table: 0x7f81d6155380 {
                [1] => -2240.0341796875
                [2] => 26.613721847534
                [3] => 3720.4460449219
              }
[hitTerrainItem] => false
[followTarget] => table: 0x7f81d6155380 {
                    [1] => 0.0
                    [2] => 0.0
                    [3] => 0.0
                  }
[pathFindingEnd] => table: 0x7f81d6155380 {
                      [1] => 0.0
                      [2] => 0.0
                      [3] => 0.0
                    }
[uuid] => "c8786b36-2474-4022-aa40-e2c575fb2f1b"
[hitBuilding] => false
[hitBody] => false
}

bodyAPI = {
[pushCommond] => function: 0x5625cd183de4
[status] => table: 0x7faa9cdb3080 {
              [BM_HAND_LEFT] => 16
              [BM_THROW] => 128
              [BM_ACT_SHOT_L] => 512
              [BM_OPERATE] => 0
              [BM_WALK_F] => 8192
              [BM_ACT_CHOP] => 4096
              [BM_LIEP] => 0
              [BM_SIT] => 4
              [BM_ACT_SHOT_R] => 1024
              [BM_BUILDP] => 0
              [BM_LIE] => 0
              [BM_WALK_L] => 32768
              [BM_RIDE] => 8
              [BM_VANISH] => 1
              [BM_LIFT] => 0
              [BM_WALK] => 122880
              [BM_WALK_R] => 65536
              [BM_WALK_B] => 16384
              [BM_ACT_THROW] => 2048
              [BM_BUILD] => 256
              [BM_SQUAT] => 2
              [BM_HAND_BOTH] => 0
              [BM_AIM] => 64
              [BM_HAND_RIGHT] => 32
            }
[addStatus] => function: 0x5625cd1843c7
[navigation] => function: 0x5625cd18482f
[session] => table: 0x7faa9cdb3080 {
               [del] => function: 0x5625cd184708
               [set] => function: 0x5625cd1845b2
               [get] => function: 0x5625cd18441a
             }
}
]]--
function getLen3DSQ(a,b)
    local deltaX = a[1]-b[1]
    local deltaY = a[2]-b[2]
    local deltaZ = a[3]-b[3]
    return deltaX*deltaX + deltaY*deltaY + deltaZ*deltaZ
end
function defaultAI(arg)
    --print_r(arg)
    --print_r(bodyAPI)
    if arg.pathFindingMode then
        local deltaX = arg.pathFindingTarget[1]-arg.position[1]
        local deltaY = arg.pathFindingTarget[2]-arg.position[2]
        local deltaZ = arg.pathFindingTarget[3]-arg.position[3]
        local pfLenSq3D = deltaX*deltaX + deltaY*deltaY + deltaZ*deltaZ

        local lenToEnd = getLen3DSQ(arg.pathFindingEnd , arg.position)
        if arg.hitBody and lenToEnd<27 then
            --防止终点被占用时卡住
            bodyAPI.pushCommond(arg.body , {
                ["cmd"]="status_remove" ,
                ["int"]=bodyAPI.status.BM_WALK
            })
            return
        end

        if arg.haveFollow then
            local fldeltaX = arg.followTarget[1]-arg.position[1]
            local fldeltaY = arg.followTarget[2]-arg.position[2]
            local fldeltaZ = arg.followTarget[3]-arg.position[3]
            local followLenSQ = fldeltaX*fldeltaX + fldeltaZ*fldeltaZ
            if followLenSQ < 9 then
                bodyAPI.pushCommond(arg.body , {
                    ["cmd"]="status_remove" ,
                    ["int"]=bodyAPI.status.BM_WALK
                })
                return
            elseif followLenSQ >100 then
                --寻路
                bodyAPI.navigation(arg.body , arg.pathFindingEnd)
                bodyAPI.pushCommond(arg.body , {
                    ["cmd"]="status_remove" ,
                    ["int"]=bodyAPI.status.BM_WALK
                })
                return
            end
        end
        if pfLenSq3D >1 then
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

        if deltaX*deltaX + deltaZ*deltaZ>9 then
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
        else
            bodyAPI.pushCommond(arg.body , {
                ["cmd"]="status_remove" ,
                ["int"]=bodyAPI.status.BM_WALK
            })
        end
    else
        bodyAPI.pushCommond(arg.body , {
            ["cmd"]="status_remove" ,
            ["int"]=bodyAPI.status.BM_WALK
        })
    end
end

