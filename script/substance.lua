local function readFile(fileName)
    local f = assert(io.open(fileName,'r'))
    local content = f:read('*all')
    f:close()
    return content
end

function loadServerSubtance(server) -- config for server
    smoothly.addSubsConf(server,1,true,100) -- player
end

function loadClientSubtance(m) -- config for client
    print("load player subtance"..
        smoothly.addSubstance(m,1,{ --player
            ["shape"]   =readFile("./res/model/testplayer/testplayer.subs"),
            ["mesh"]    ="./res/model/testplayer/testplayer.obj",
            ["bodyType"]="character",
            ["active"]  ={
                ["noFallDown"]      =true,
                ["defaultSpeed"]    =0.01,
                ["defaultLiftForce"]=0,
                ["defaultPushForce"]=0,
                ["defaultJumpImp"]  =5
            }
        })
    )
end