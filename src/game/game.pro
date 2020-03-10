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

INCLUDEPATH += ../../dep/ /usr/local/include/bullet/ /usr/include/irrlicht

LIBS += -luuid -lleveldb -lpthread -lRakNetDLL -llua -ldl -lIrrlicht -lBulletDynamics -lBulletCollision -lLinearMath

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
    ../utils/dbvt3d.h

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
    ../utils/dbvt3d.cpp

FORMS += \
    ui/login.ui

DISTFILES += \
    ../shader/terrain.ps.glsl \
    ../shader/terrain.vs.glsl
