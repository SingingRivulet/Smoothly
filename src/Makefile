
CC=g++ -std=c++11 -ggdb -I /usr/local/include/bullet/ 
LIBS= -lIrrlicht -lBulletDynamics -lBulletCollision -lLinearMath -luuid -lpthread -lRakNetDLL -llua -ldl

all:../client ../server ../admin
	

./a,out:main.cpp perlin.o clientNetwork.o createTerrainMesh.o
	$(CC) main.cpp perlin.o terrain.o createTerrainMesh.o building.o hbb.o clientNetwork.o $(LIBS)

../admin:utils.h admin.cpp
	$(CC) admin.cpp -o ../admin $(LIBS)

../client:client.cpp game.o subsapi.o
	$(CC) client.cpp \
	perlin.o \
	SimplexNoise.o \
	terrain.o \
	createTerrainMesh.o \
	remoteGraph.o \
	building.o \
	hbb.o \
	clientNetwork.o \
	view.o \
	control.o \
	mods.o \
	physical.o \
	substance.o \
	subsani.o \
	bone.o \
	subsapi.o \
	-o ../client \
	$(LIBS)

dblist.o:dblist.h dblist.cpp utils.h
	$(CC) -c dblist.cpp

subsServer.o:subsServer.h subsServer.cpp utils.h cache.h dblist.o
	$(CC) -c subsServer.cpp

user.o:user.h user.cpp attaching.o
	$(CC) -c user.cpp

attaching.o:attaching.h attaching.cpp subsServer.o
	$(CC) -c attaching.cpp

RealisticWater.o:RealisticWater.h RealisticWater.cpp
	$(CC) -c RealisticWater.cpp

physical.o:physical.h physical.cpp utils.h
	$(CC) -c physical.cpp

mods.o:mods.h mods.cpp utils.h physical.o
	$(CC) -c mods.cpp

SimplexNoise.o:SimplexNoise.cpp SimplexNoise.h
	$(CC) -c SimplexNoise.cpp

perlin.o:perlin.cpp terrain.h perlin.h SimplexNoise.o
	$(CC) -c perlin.cpp

createTerrainMesh.o:createTerrainMesh.cpp terrain.h
	$(CC) -c createTerrainMesh.cpp

terrain.o:terrain.cpp terrain.h mods.h utils.h mods.o createTerrainMesh.o perlin.o
	$(CC) -c terrain.cpp

remoteGraph.o:remoteGraph.h remoteGraph.cpp
	$(CC) -c remoteGraph.cpp

remoteGraph_test.out:remoteGraph.o remoteGraph_test.cpp
	$(CC) remoteGraph_test.cpp remoteGraph.o -o remoteGraph_test.out $(LIBS)

hbb.o:hbb.h hbb.cpp
	$(CC) -c hbb.cpp

hbb_test.out:hbb_test.cpp hbb.o
	$(CC) hbb_test.cpp hbb.o -o hbb_test.out $(LIBS)

substance.o:substance.cpp substance.h building.o subsani.o
	$(CC) -c substance.cpp

clientNetwork.o:clientNetwork.cpp clientNetwork.h substance.o
	$(CC) -c clientNetwork.cpp

subsani.o:subsani.cpp subsani.h physical.o bone.o
	$(CC) -c subsani.cpp

bone.o:bone.cpp subsani.h physical.o
	$(CC) -c bone.cpp

subsapi.o:subsapi.cpp subsapi.h substance.o
	$(CC) -c subsapi.cpp

view.o:view.h view.cpp clientNetwork.o
	$(CC) -c view.cpp

control.o:control.h control.cpp view.o
	$(CC) -c control.cpp

game.o:game.h game.cpp control.o
	$(CC) -c game.cpp

building.o:terrain.o building.h remoteGraph.o hbb.o building.cpp
	$(CC) -c building.cpp

../server:server.cpp serverNetwork.o ../serverCallback.so
	$(CC) server.cpp \
	serverNetwork.o \
	watch.o \
	serverMod.o \
	removeTable.o \
	graphServer.o \
	hbb.o \
	user.o \
	dblist.o \
	subsServer.o \
	attaching.o \
	./serverCallback.so \
	-lleveldb -o ../server $(LIBS)

serverNetwork.o:watch.o serverNetwork.cpp serverNetwork.h
	$(CC) -c serverNetwork.cpp

watch.o:serverMod.o watch.cpp watch.h
	$(CC) -c watch.cpp

serverMod.o:removeTable.o graphServer.o attaching.o serverMod.cpp serverMod.h
	$(CC) -c serverMod.cpp

removeTable.o:removeTable.cpp removeTable.h
	$(CC) -c removeTable.cpp

graphServer.o:graphServer.cpp graphServer.h
	$(CC) -c graphServer.cpp

../serverCallback.so:serverCallback.cpp
	$(CC) -c serverCallback.cpp -shared -fPIC -o serverCallback.so
	cp serverCallback.so ../serverCallback.so

clean:
	rm -f *.o
	rm -f ../client ../server
	rm -f *.out
	rm -f *.gch
