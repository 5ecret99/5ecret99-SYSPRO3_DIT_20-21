#ifndef _CHFFUNCTIONS__
#define _CHFFUNCTIONS__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/errno.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>

#include "hash.h"
#include "bloomfilter.h"
#include "skiplist.h"
#include "records.h"
#include "hashtable.h"
#include "sockets.h"
#include "cbuffer.h"
//mutex lock gia tus hashtables etsi wste na min prospa8oun  2 threads na valoun/paroun kti se/apo autous tautoxrona kai na dhmiourgeite provlima
pthread_mutex_t viruses_lock;
pthread_mutex_t records_lock;
pthread_mutex_t	countries_lock;

//global variables
int totalreq;
int accepted;
int rejected;
int done;

//domh gia ta threads
typedef struct thread_data
	{
	hashtable* viruses;
	hashtable* records;
	hashtable* countries;
	cbuffer* cbuff;
	int sizeOfBloom;
	int done;
	}thread_data;

void *myThread(void *data);//sinartisi gia ta threads
//ftiaxnei ta threads kai akolou8ws vazei ena ena ta arxeia sto kikliko buffer mexri ta threads na ta diavasoun olla
void readdirectory(int cyclicBufferSize,int numThreads,char* directory,int sizeOfBloom,hashtable* viruses,hashtable* records,hashtable* countries,hashtable* directories);

void insertCitizenRecord(int bloomSize,hashtable* viruses,hashtable* records,hashtable* countries,char* citizenID,char* firstName,char* lastName,char* country,char* age,char* virus_name,char* yes_no,char* date);//vazei ton citizen stous katallilous hashtable idia me prwti ergasia

char* Child_travelRequest(hashtable* viruses,char* virusName,int citizenID,char* tdate);//epistrefei to katallilo minima gia to travelRequest tu citizen

void sendblooms(int socket_fd,hashtable* viruses,int socketBufferSize,int sizeOfBloom);//stelnei ola ta blooms ana virus ston patera

void travelStats(hashtable* viruses,char* country,char* virusName,time_t a,time_t b,int fda,int bufferSize);//function pu stelnei ta travelStats ston patera

void end_searchVaccinationStatus(int fda,int bufferSize);//stelnei end_searchVaccinationStatus meso pipe ston patera
void searchVaccinationStatus(hashtable* records,hashtable* viruses,int citizenID,int fda,int bufferSize);//psaxnei kai stelnei ola ta stoixeia gia ton citizen pu iparxun se ato to monitor




#endif