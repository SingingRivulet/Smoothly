CC=g++ -std=c++11 -ggdb -I/usr/include/irrlicht -I/usr/local/include/bullet/ -lIrrlicht -lBulletDynamics -lBulletCollision -lLinearMath -luuid -lpthread -lRakNetDLL -llua -ldl

terrainDispather.o:terrainDispather.h terrainDispather.cpp terrain_item.o
	$(CC) terrainDispather.cpp -c

terrain_item.o:terrain_item.h terrain_item.cpp terrain.o
	$(CC) terrain_item.cpp -c

terrain.o:terrain.h terrain.cpp engine.o
	$(CC) terrain.cpp -c

engine.o:engine.h engine.cpp physical.o
	$(CC) engine.cpp -c

physical.o:physical.h physical.cpp
	$(CC) physical.cpp -c