CC=g++
CFLAGS= -Wall -c -Wno-unused-but-set-variable
LDLIBS= -Wall -pthread

all: sysim

sysim: main.o objects.o neurone.o network.o rains.o annexe.o dclist.o Ncq.o
	$(CC) $(LDLIBS) main.o objects.o neurone.o network.o rains.o annexe.o dclist.o Ncq.o -o sysim

main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp

objects.o: objects.cpp
	$(CC) $(CFLAGS) objects.cpp

neurone.o: neurone.cpp
	$(CC) $(CFLAGS) neurone.cpp

network.o: network.cpp
	$(CC) $(CFLAGS) network.cpp

rains.o: rains.cpp
	$(CC) $(CFLAGS) rains.cpp

annexe.o: annexe.cpp
	$(CC) $(CFLAGS) annexe.cpp

dclist.o: dclist.cpp
	$(CC) $(CFLAGS) dclist.cpp

Ncq.o: Ncq.cpp
	$(CC) $(CFLAGS) Ncq.cpp

clean:
	rm -rf *o *gch sysim

