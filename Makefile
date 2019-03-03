
CC=g++ -std=c++11 -ggdb -I /usr/local/include/bullet/ 
LIBS= -lIrrlicht -lBulletDynamics -lBulletCollision -lLinearMath -luuid -lpthread -lRakNetDLL -llua

./a,out:main.cpp perlin.o clientNetwork.o createTerrainMesh.o
	$(CC) main.cpp perlin.o terrain.o createTerrainMesh.o building.o hbb.o clientNetwork.o $(LIBS)

client:client.cpp game.o
	$(CC) client.cpp \
	perlin.o \
	terrain.o \
	createTerrainMesh.o \
	remoteGraph.o \
	building.o \
	hbb.o \
	clientNetwork.o \
	view.o \
	control.o \
	mods.o \
	-o client \
	$(LIBS)

mods.o:mods.h mods.cpp utils.h
	$(CC) -c mods.cpp

perlin.o:perlin.cpp terrain.h perlin.h
	$(CC) -c perlin.cpp

createTerrainMesh.o:createTerrainMesh.cpp terrain.h
	$(CC) -c createTerrainMesh.cpp

terrain.o:terrain.cpp terrain.h mods.h utils.h mods.o
	$(CC) -c terrain.cpp

remoteGraph.o:remoteGraph.h remoteGraph.cpp
	$(CC) -c remoteGraph.cpp

remoteGraph_test.out:remoteGraph.o remoteGraph_test.cpp
	$(CC) remoteGraph_test.cpp remoteGraph.o -o remoteGraph_test.out $(LIBS)

hbb.o:hbb.h hbb.cpp
	$(CC) -c hbb.cpp

hbb_test.out:hbb_test.cpp hbb.o
	$(CC) hbb_test.cpp hbb.o -o hbb_test.out $(LIBS)

clientNetwork.o:clientNetwork.cpp clientNetwork.h building.o
	$(CC) -c clientNetwork.cpp

view.o:view.h view.cpp clientNetwork.o
	$(CC) -c view.cpp

control.o:control.h control.cpp view.o
	$(CC) -c control.cpp

game.o:game.h game.cpp control.o
	$(CC) -c game.cpp

building.o:terrain.o building.h remoteGraph.o hbb.o building.cpp
	$(CC) -c building.cpp

server:server.cpp serverNetwork.o
	$(CC) server.cpp serverNetwork.o watch.o serverMod.o removeTable.o graphServer.o hbb.o -lleveldb -o server $(LIBS)

serverNetwork.o:watch.o serverNetwork.cpp serverNetwork.h
	$(CC) -c serverNetwork.cpp

watch.o:serverMod.o watch.cpp watch.h
	$(CC) -c watch.cpp

serverMod.o:removeTable.o graphServer.o serverMod.cpp serverMod.h
	$(CC) -c serverMod.cpp

removeTable.o:removeTable.cpp removeTable.h
	$(CC) -c removeTable.cpp

graphServer.o:graphServer.cpp graphServer.h
	$(CC) -c graphServer.cpp
