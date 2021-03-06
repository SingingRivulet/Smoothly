TRANSLATIONS = lang/lang_English.ts \
               lang/lang_Chinses.ts

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += ../../dep/ /usr/local/include/bullet/
INCLUDEPATH += ../../dep/irrlicht/include/
INCLUDEPATH += ../../dep/freetype2/include/
INCLUDEPATH += ../../dep/RakNet/include/

INCLUDEPATH += ../../dep/ik/ik/include/public
INCLUDEPATH += ../../dep/ik/ik/include/private
INCLUDEPATH += ../../dep/ik/build/ik/include/public
INCLUDEPATH += ../../dep/ik/build/ik/include/private

LIBS += -luuid -lleveldb -lpthread -llua -ldl
LIBS += ../../dep/irrlicht/source/Irrlicht/libIrrlicht.a -lBulletDynamics -lBulletCollision -lLinearMath
LIBS += ../../dep/ik/build/ik/ik.a
LIBS += -L/usr/X11R6/lib$(LIBSELECT) -L../../lib/Linux
LIBS += -lGL -lXxf86vm -lXext -lX11 -lSDL2 -lSDL2_ttf -lfreetype -lalut -lopenal
LIBS += ../../dep/RakNet/build/Lib/LibStatic/libRakNetLibStatic.a

HEADERS += \
    engine.h \
    myCharacter.h \
    physical.h \
    terrain.h \
    terrain_item.h \
    terrainDispather.h \
    ../libclient/connection.h \
    ../libclient/SimplexNoise.h \
    ../libclient/terrain.h \
    ../libclient/terrain_item.h \
    ../libclient/terrainDispather.h \
    ../libclient/terrainGen.h \
    ../utils/cJSON.h \
    ../utils/uuid.h \
    body.h \
    interactive.h \
    bone.h \
    fire.h \
    control.h \
    game.h \
    ui/login.h \
    dbvtdispather.h \
    building.h \
    ../utils/dbvt3d.h \
    RealisticWater.h \
    cloud.h \
    weather.h \
    pathfinding.h \
    package.h \
    technology.h \
    shadow.h \
    carto.h \
    mission.h \
    CGUITTFont.h \
    ui/missioneditor.h \
    mail.h \
    graphbase.h \
    localLight.h \
    texturepool.h \
    group_ai.h

SOURCES += \
    createTerrainMesh.cpp \
    engine.cpp \
    physical.cpp \
    terrain.cpp \
    terrain_item.cpp \
    terrainDispather.cpp \
    ../libclient/SimplexNoise.cpp \
    ../libclient/terrainGen.cpp \
    ../utils/cJSON.c \
    body.cpp \
    body_handler.cpp \
    body_config.cpp \
    interactive.cpp \
    body_control.cpp \
    body_loop.cpp \
    ../bone.cpp \
    fire.cpp \
    fire_config.cpp \
    fire_bullet.cpp \
    fire_collision.cpp \
    fire_attack.cpp \
    control.cpp \
    game.cpp \
    ui/login.cpp \
    main.cpp \
    dbvtdispather.cpp \
    building.cpp \
    ../utils/dbvt3d.cpp \
    RealisticWater.cpp \
    cloud.cpp \
    weather.cpp \
    meshGenerator/tree.cpp \
    meshGenerator/grass.cpp \
    building_default.cpp \
    building_road.cpp \
    pathfinding.cpp \
    fire_api.cpp \
    package.cpp \
    body_bag.cpp \
    technology.cpp \
    shadow.cpp \
    carto.cpp \
    mission.cpp \
    CGUITTFont.cpp \
    ui/missioneditor.cpp \
    control_cmd.cpp \
    mail.cpp \
    body_ai.cpp \
    body_api.cpp \
    localLight.cpp \
    body_ik.cpp \
    group_ai.cpp

FORMS += \
    ui/login.ui \
    ui/missioneditor.ui

DISTFILES += \
    ../shader/terrain.vs.glsl \
    ../shader/terrain_lod1.ps.glsl \
    ../shader/terrain_lod2.ps.glsl \
    ../shader/terrain_lod3.ps.glsl \
    ../shader/terrain_lod4.ps.glsl \
    ../shader/Water.vs.glsl \
    ../shader/Water.ps.glsl \
    ../shader/cloud.ps.glsl \
    ../shader/cloud.vs.glsl \
    ../shader/sky.ps.glsl \
    ../shader/sky.vs.glsl \
    ../shader/tree.ps.glsl \
    ../shader/tree.vs.glsl \
    ../shader/building_default.ps.glsl \
    ../shader/building_default.vs.glsl \
    ../script/body.lua \
    ../script/building.lua \
    ../script/terrainItem.lua \
    ../script/utils/print_r.lua \
    ../script/fire.lua \
    ../shader/shadow.ps.glsl \
    ../shader/shadow.vs.glsl \
    ../script/voxelDrawer.lua \
    ../../ai/main.lua \
    ../script/body_animation.lua
