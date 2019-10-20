CC=g++ -std=c++11 -ggdb -I /usr/local/include/bullet/ -I ../ -I ../../dep/
LIBS= -luuid -lleveldb -lpthread -lRakNetDLL -llua -ldl

server:main.cpp server.o
	$(CC) main.cpp -o server \
	server.o \
	controllers.o \
	handlers.o \
	connection.o \
	bullet.o \
	body.o \
	removeTable.o \
	map.o \
	$(LIBS)
clean:
	-rm *.o
	-rm server
	-rm -rf data
	-rm -rf log
server.o:server.h server.cpp controllers.o
	$(CC) server.cpp -c $(LIBS)

controllers.o:controllers.h controllers.cpp handlers.o
	$(CC) controllers.cpp -c $(LIBS)

handlers.o:handlers.h handlers.cpp connection.o
	$(CC) handlers.cpp -c $(LIBS)

connection.o:connection.h connection.cpp bullet.o
	$(CC) connection.cpp -c $(LIBS)

bullet.o:bullet.h bullet.cpp body.o
	$(CC) bullet.cpp -c $(LIBS)

body.o:body.h body.cpp removeTable.o
	$(CC) body.cpp -c $(LIBS)

removeTable.o:removeTable.h removeTable.cpp map.o
	$(CC) removeTable.cpp -c $(LIBS)

map.o:map.h map.cpp db.h
	$(CC) map.cpp -c $(LIBS)
