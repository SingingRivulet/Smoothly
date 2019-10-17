CC=g++ -std=c++11 -ggdb -I /usr/local/include/bullet/ -I ../../../
LIBS= -lIrrlicht -lBulletDynamics -lBulletCollision -lLinearMath -luuid -lpthread -lRakNetDLL -llua -ldl

character.o:character.cpp character.h
	$(CC) character.cpp -c $(LIBS)
