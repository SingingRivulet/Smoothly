function mapGenerator(x,y,tem,hu,h)
    return {
        {128,0.1}
    }
end

function gameInit(m)
    smoothly.addAutoGen(m,mapGenerator)
end

function sceneInit(m)
    smoothly.addTerrainMesh(m,128,{
        ["mesh"]    ="./res/model/test1.obj",
        ["havebody"]=true,
        ["maxnum"]  =5
    })
end