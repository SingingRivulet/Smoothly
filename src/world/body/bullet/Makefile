CC=g++ -std=c++11 -ggdb -I /usr/local/include/bullet/ -I ../../../
LIBS= -lIrrlicht -lBulletDynamics -lBulletCollision -lLinearMath -luuid -lpthread -lRakNetDLL -llua -ldl

bullet.o:bullet.cpp bullet.h
	$(CC) bullet.cpp -c $(LIBS)
