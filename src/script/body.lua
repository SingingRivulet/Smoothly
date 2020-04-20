package.path = package.path .. ";../script/?.lua;"
require("utils.print_r")
function snoutx10kCallback(arg)
    --print_r(arg)
    if arg.status.walk.forward==1 then
        return 80,0,80,true
    elseif arg.status.walk.forward==-1 then
        return -80,0,80,true
    else
        return 1,1,1,true
    end
end
