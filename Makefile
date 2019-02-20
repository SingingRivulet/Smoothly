
CC=g++ -std=c++11 -ggdb -I /usr/local/include/bullet/ 
LIBS= -lIrrlicht -lBulletDynamics -lBulletCollision -lLinearMath -luuid -lpthread -lRakNetDLL

./a,out:main.cpp perlin.o clientNetwork.o createTerrainMesh.o
	$(CC) main.cpp perlin.o terrain.o createTerrainMesh.o building.o hbb.o clientNetwork.o $(LIBS)

perlin.o:perlin.cpp terrain.h perlin.h
	$(CC) -c perlin.cpp

createTerrainMesh.o:createTerrainMesh.cpp terrain.h
	$(CC) -c createTerrainMesh.cpp

terrain.o:terrain.cpp terrain.h mods.h utils.h
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

building.o:terrain.o building.h remoteGraph.o hbb.o building.cpp
	$(CC) -c building.cpp
