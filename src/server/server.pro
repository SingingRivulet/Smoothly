QT -= gui
QT += xml

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp \
    body.cpp \
    bullet.cpp \
    connection.cpp \
    controllers.cpp \
    handlers.cpp \
    map.cpp \
    removeTable.cpp \
    server.cpp \
    admin.cpp \
    admin_handler.cpp \
    buildinggraph.cpp \
    ../utils/cJSON.c \
    ../utils/dbvt2d.cpp \
    building.cpp \
    bag.cpp \
    package.cpp \
    technology.cpp \
    making.cpp \
    mission.cpp \
    mailpackage.cpp \
    dig.cpp

HEADERS += \
    body.h \
    bullet.h \
    connection.h \
    controllers.h \
    db.h \
    handlers.h \
    map.h \
    removeTable.h \
    server.h \
    fontcolor.h \
    admin.h \
    ../utils/uuid.h \
    ../utils/cJSON.h \
    ../utils/uuid.h \
    heartbeat.h \
    ../utils/dbvt2d.h \
    ../utils/mempool.h \
    cache.h \
    buildinggraph.h \
    ../utils/ipair.h \
    ../utils/cyqueue.h \
    building.h \
    bag.h \
    package.h \
    technology.h \
    making.h \
    mission.h \
    mailpackage.h \
    dig.h

INCLUDEPATH += ../../dep/ /usr/local/include/bullet/

LIBS += -luuid -lleveldb -lpthread -lRakNetDLL -lsqlite3 -llua -ldl

DISTFILES +=
