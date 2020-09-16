package.path = package.path .. ";../script/?.lua;"
require("voxelDrawer")
print("load building.lua success")
print(_VERSION)
function pathFindingInit(self)
    print("[pathFindingInit]start")

    local board = seekConfig(self,-1)
    for i=-4,4 do
        for j=-4,4 do
            addVoxel(board,i,0,j)
        end
    end

    local board = seekConfig(self,-2)
    addVoxel(board,0,0,0)

    local board = seekConfig(self,-3)
    addVoxel(board,0,0,0)

    local board = seekConfig(self,-4)
    for i=-4,4 do
        for j=-1,8 do
            addVoxel(board,i,j,0)
        end
    end

    local board = seekConfig(self,-5)
    addVoxel(board,0,0,0)

    local board = seekConfig(self,-6)
    addVoxel(board,0,0,0)

end
