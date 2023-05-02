CC=g++
CFLAGS=-c -Wall

all: wserver clean
wserver: wserver.o serversocket.o clientsocket.o socketconfig.o
	$(CC) wserver.o serversocket.o clientsocket.o socketconfig.o -o wserver -lpthread
wserver.o: wserver.cpp serversocket.h clientsocket.h socketconfig.h
	$(CC) $(CFLAGS) wserver.cpp
serversocket.o: serversocket.h
clientsocket.o: clientsocket.h
socketconfig.o: socketconfig.h
clean:
	rm -rf *.o
