all:		travelMonitorClient monitorServer

child_functions.o: 	child_functions.c
		gcc -c child_functions.c

parent_functions.o: 	parent_functions.c
		gcc -c parent_functions.c

signal.o:	signal.c
		gcc -c signal.c

records.o: 	records.c
		gcc -c records.c

hash.o: 	hash.c
		gcc -c hash.c

skiplist.o: 	skiplist.c
		gcc -c skiplist.c

bloomfilter.o: 	bloomfilter.c
		gcc -c bloomfilter.c

hashtable.o: 	hashtable.c
		gcc -c hashtable.c

sockets.o:	sockets.c
		gcc -c sockets.c

directory.o:	directory.c
		gcc -c directory.c

cbuffer.o:	cbuffer.c
		gcc -c cbuffer.c

travelMonitorClient.o:	travelMonitorClient.c
		gcc -c travelMonitorClient.c

monitorServer.o:	monitorServer.c
		gcc -c monitorServer.c

travelMonitorClient:	sockets.o directory.o travelMonitorClient.o hashtable.o skiplist.o bloomfilter.o hash.o records.o signal.o parent_functions.o 
	gcc sockets.o directory.o travelMonitorClient.o hashtable.o skiplist.o bloomfilter.o hash.o records.o parent_functions.o signal.o -o travelMonitorClient

monitorServer: monitorServer.o sockets.o child_functions.o records.o hash.o skiplist.o bloomfilter.o hashtable.o signal.o cbuffer.o
	gcc monitorServer.o sockets.o child_functions.o records.o hash.o skiplist.o bloomfilter.o  hashtable.o  signal.o cbuffer.o  -pthread -o monitorServer

clean:
	rm *o travelMonitorClient monitorServer