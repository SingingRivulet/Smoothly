package.path="./script/?.lua;"..package.path
require("substance")
function mapGenerator(x,y,tem,hu,h)
    return {
        {128,0.1},
        {129,0.1}
    }
end

function gameInit(m)
    smoothly.addAutoGen(m,mapGenerator)
end

function sceneInit(m)
    print("load mesh:"..
        smoothly.addTerrainMesh(m,128,{
            ["mesh"]     ="./res/model/test1/test1-lodv1.obj",
            ["lodMeshV2"]="./res/model/test1/test1-lodv3.obj",
            --["lodMeshV3"]="./res/model/test1/test1-lodv4.obj",
            --["lodMeshV4"]="./res/model/test1/test1-lodv4.obj",
            ["havebody"]=true,
            ["maxnum"]  =3
        })
    )
    print("load mesh:"..
        smoothly.addTerrainMesh(m,129,{
            ["mesh"]     ="./res/model/tree1/tree_foliage_lod0.obj",
            ["lodMeshV2"]="./res/model/tree1/tree_foliage_lod2.obj",
            --["lodMeshV3"]="./res/model/tree1/tree_foliage_lod2.obj",
            --["lodMeshV4"]="./res/model/tree1/tree_foliage_lod2.obj",
            ["texture"]  ="./res/model/tree1/tree_foliage.tga",
            ["havebody"]=true,
            ["useAlpha"]=true,
            ["maxnum"]  =2
        })
    )
    print("load subtance")
    loadClientSubtance(m)
end