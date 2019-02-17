
CC=g++ -std=c++11 -ggdb -I /usr/local/include/bullet/ 
LIBS= -lIrrlicht -lBulletDynamics -lBulletCollision -lLinearMath -luuid

./a,out:main.cpp perlin.o building.o createTerrainMesh.o
	$(CC) main.cpp perlin.o terrain.o createTerrainMesh.o building.o hbb.o $(LIBS)

perlin.o:perlin.cpp terrain.h perlin.h
	$(CC) -c perlin.cpp

createTerrainMesh.o:createTerrainMesh.cpp terrain.h
	$(CC) -c createTerrainMesh.cpp

terrain.o:terrain.cpp terrain.h mods.h
	$(CC) -c terrain.cpp

remoteGraph.o:remoteGraph.h remoteGraph.cpp
	$(CC) -c remoteGraph.cpp

remoteGraph_test.out:remoteGraph.o remoteGraph_test.cpp
	$(CC) remoteGraph_test.cpp remoteGraph.o -o remoteGraph_test.out $(LIBS)

hbb.o:hbb.h hbb.cpp
	$(CC) -c hbb.cpp

building.o:terrain.o building.h remoteGraph.o hbb.o
	$(CC) -c building.cpp
