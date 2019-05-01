package.path="./script/?.lua;"..package.path

require("building")
require("admin")
require("substance")
function modInit(server)
    print("Smoothly Server")
    print("by cgoxopx")
    loadBuilding(server)
    loadAdmin(server)
    loadServerSubtance(server)
    smoothly.setViewRange(server,128)
    print("init finished")
end