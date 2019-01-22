
CC=g++ -std=c++11 -ggdb 

./a,out:main.cpp perlin.o terrain.o createTerrainMesh.o
	$(CC) main.cpp perlin.o terrain.o createTerrainMesh.o -lIrrlicht

perlin.o:perlin.cpp terrain.h perlin.h
	$(CC) -c perlin.cpp

createTerrainMesh.o:createTerrainMesh.cpp terrain.h
	$(CC) -c createTerrainMesh.cpp

terrain.o:terrain.cpp terrain.h mods.h
	$(CC) -c terrain.cpp

