#ifndef _SOCKETS__
#define	_SOCKETS__

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>

 #include <poll.h>
#include <string.h>
#include <math.h>
#include <sys/wait.h>			/* For wait */
#include <sys/types.h>			/* For sockets */
#include <sys/socket.h>			/* For sockets */
#include <netinet/in.h>			/* For Internet sockets */
#include <netdb.h>			/* For gethostbyname */

struct sockaddr_in address;
#define AVAL 999999
#define WTIME 999999//o megistos xronos p perimenoun ta polls
#define MAXCOUNT 100//megistos arithmos prospathiwn gia poll
void waiting();

struct in_addr getip();//epistrefi to topiko ip se struct in_addr
int socket_init(int PORT,int parent);//ftiaxnei ta sockets an to int parent=1 ine gia ton patera an parent=0 ine gia ta ta pedia

void en_poll(int fd);//sinartisi gia xrisi poll gia na stalei to minima
void encode(int fd,char* message,int bufferSize);//sinartisi gia na stalthei to minima meso socket

void dec_poll(int fd);//sinartisi gia xrisi poll gia na paroume to minima
char* decode(int fd,int bufferSize);//sinartisi gia na paroume to minima meso socket

void encode_bloom(int fd,unsigned char* message,int bufferSize,int sizeOfBloom);//apostoli bloomfilter
unsigned char* decode_bloom(int fd,int bufferSize,int sizeOfBloom);//paralavi bloomfilter


#endif