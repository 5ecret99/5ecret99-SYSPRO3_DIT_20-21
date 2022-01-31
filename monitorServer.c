#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/errno.h>
#include <dirent.h>

#include <sys/wait.h>			/* For wait */
#include <sys/types.h>			/* For sockets */
#include <sys/socket.h>			/* For sockets */
#include <netinet/in.h>			/* For Internet sockets */
#include <netdb.h>			/* For gethostbyname */

#include <string.h>    /* For strerror */
#include <stdlib.h>    /* For exit     */
#include <pthread.h>   /* For threads  */

#include "directory.h"
#include "hashtable.h"
#include "bloomfilter.h"
#include "child_functions.h"
#include "hash.h"
#include "sockets.h"
#include "signal.h"

int main(int argc, char const *argv[])
	{
	//checking command line arguments
	if(argc!=12){printf("monitorServer -p port -t numThreads -b socketBufferSize -c cyclicBufferSize -s sizeOfBloom path1 path2 ... pathn\n");return 1;}
	if (argv[1][1]!= 'p' || argv[3][1] != 't'|| argv[5][1]!= 'b'|| argv[7][1]!= 'c'|| argv[9][1]!= 's'){printf("wrong -symbol the right order is:\n-p-t-b-c-s\n");return 1;}
	printf("child with pid %d created\n",getpid());

	int port=atoi(argv[2]);
	int socket_fd=socket_init(port,0);

	int numThreads=atoi(argv[4]);

	int socketBufferSize=atoi(argv[6]);

	int cyclicBufferSize=atoi(argv[8]);

	int sizeOfBloom=atoi(argv[10]);

	char directories_str[9999];strcpy(directories_str,argv[11]);
	
	//arxikopoiw ta aparetita hashtables
	hashtable* viruses=(hashtable*)malloc(sizeof(hashtable));viruses=hashtable_init(viruses,50,0);//me tus ious (2 skiplist kai 1 bloom opws thn prwth ergasia) des hashtable.h/.c gia tin domh
	hashtable* records=(hashtable*)malloc(sizeof(hashtable));records=hashtable_init(records,500,2);//me ta records h domh vriskete sto records.h/.c
	hashtable* countries=(hashtable*)malloc(sizeof(hashtable));countries=hashtable_init(countries,20,-1);//me tis countries den xriazete domh to void* data = char* country
	hashtable* directories=(hashtable*)malloc(sizeof(hashtable));directories=hashtable_init(directories,100,-1);//me ta directories den xriazete domh to void* data = char* directory
	
	//sinartisi pu diavazei ola ta arxeia se ka8e directory kai ta vazei stus analogous hashtables
	//des child_functions.h/.c

	readdirectory(cyclicBufferSize,numThreads,directories_str,sizeOfBloom,viruses,records,countries,directories);

	sendblooms(socket_fd,viruses,socketBufferSize,sizeOfBloom);


	while(1)
		{
		
		//diavazei thn entolh
		char* command=decode(socket_fd,socketBufferSize);

		if (strcmp(command,"/exit")==0)//command exit
			{
			char buff[30];sprintf(buff,"%d",accepted);
			//stelnei ton ari8mo twn accepted
			encode(socket_fd,buff,socketBufferSize);

			//stelnei ton ari8mo twn rejected
			sprintf(buff,"%d",rejected);
			encode(socket_fd,buff,socketBufferSize);

			logfile(getpid(),countries,totalreq,accepted,rejected,0);
			if(command!=NULL){free(command);command=NULL;}
			break;

			}
		else if (strcmp(command,"/addVaccinationRecords")==0)//command addVaccinationRecords
		//pros8e8ikan arxeia sta directories
			{
			//des child_functions.h/.c
			//3anadiavazw ta directories an vrw nea arxeia pros8etw tis plirofories tus
			printf("%s\n",directories_str );
			readdirectory(cyclicBufferSize,numThreads,directories_str,sizeOfBloom,viruses,records,countries,directories);
			printf("sendblooms\n");
			//stelnw ta blooms des child_functions.h/.c
			sendblooms(socket_fd,viruses,socketBufferSize,sizeOfBloom);
			}
		else if (strcmp(command,"/travelRequest")==0)//command travelRequest
			{

			char* virusName=decode(socket_fd,socketBufferSize);

			char* citizenID = decode(socket_fd,socketBufferSize);

			char* tdate=decode(socket_fd,socketBufferSize);

			char* message =Child_travelRequest(viruses,virusName,atoi(citizenID),tdate);//des child_functions.h/,c


			encode(socket_fd,message,socketBufferSize);
			free(virusName);free(citizenID);free(tdate);
			}
		else if (strcmp(command,"/travelStats")==0||strcmp(command,"/travelStatsC")==0)//command travelStats
			{				

			char*virusName=decode(socket_fd,socketBufferSize);

			char*date1=decode(socket_fd,socketBufferSize);
			time_t dd1=dateconverter(date1);free(date1);

			char*date2=decode(socket_fd,socketBufferSize);
			time_t dd2=dateconverter(date2);free(date2);
			char* country=NULL;

			if(strcmp(command,"/travelStatsC")==0)//an ine gia sigkekrimeni xwra tin diavazei apto pipe
				{
				country=decode(socket_fd,socketBufferSize);
				}

			travelStats(viruses,country,virusName,dd1,dd2,socket_fd,socketBufferSize);//des child_functions.h/,c
			free(virusName);if(country!=NULL){free(country);}
			}
		else if (strcmp(command,"/searchVaccinationStatus")==0)//command searchVaccinationStatus
			{

			char*citizenID=decode(socket_fd,socketBufferSize);

			searchVaccinationStatus(records,viruses,atoi(citizenID),socket_fd,socketBufferSize);//des child_functions.h/,c
			free(citizenID);
			}

		if(command!=NULL){free(command);command=NULL;}
			
		}
	//apelefthrwsi mnimis
	close(socket_fd);
	hashtable_free(directories);
	hashtable_free(countries);
	hashtable_free(records);
	hashtable_free(viruses);


	return 0;
	}