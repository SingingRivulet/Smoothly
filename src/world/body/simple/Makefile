CC=g++ -std=c++11 -ggdb -I /usr/local/include/bullet/ -I ../../../
LIBS= -lIrrlicht -lBulletDynamics -lBulletCollision -lLinearMath -luuid -lpthread -lRakNetDLL -llua -ldl

simple.o:simple.cpp simple.h
	$(CC) simple.cpp -c $(LIBS)
