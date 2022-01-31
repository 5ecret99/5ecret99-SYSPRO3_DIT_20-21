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
#include "parent_functions.h"
#include "hash.h"
#include "sockets.h"
#include "signal.h"

#define FPORT 8080
int main(int argc, char const *argv[])
	{
	//checking command line arguments
	if(argc!=13){printf("./travelMonitorClient -m numMonitors -b socketBufferSize -c cyclicBufferSize -s sizeOfBloom -i input_dir -t numThreads\n");return 1;}
	if (argv[1][1]!= 'm' || argv[3][1] != 'b'|| argv[5][1]!= 'c'|| argv[7][1]!= 's'|| argv[9][1]!= 'i'|| argv[11][1]!= 't'){printf("wrong -symbol the right order is:\n-m-b-c-s-i-t\n");return 1;}
	//initializing variables from command line arguments
	int numMonitors=atoi(argv[2]);

	int socketBufferSize=atoi(argv[4]);const char* socketBufferSize_str=argv[4];

	int cyclicBufferSize=atoi(argv[6]);const char* cyclicBufferSize_str=argv[6];

	int sizeOfBloom=atoi(argv[8]);const char* sizeOfBloom_str=argv[8];

	char input_dir[100];strcpy(input_dir,argv[10]);

	int numThreads=atoi(argv[12]);const char* numThreads_str=argv[12];
	
 	//assignig directories for each monitor
 	char** namelist;//sto namelist[i] exw ola ta directories pu anate8ontai ston monitor i me round robin alphabetically
 	hashtable* cp;//hashtable pou leitourgei ews dictionary  exwntas (xwra,monitor)
 	int numCountries=dir(&namelist,&cp,input_dir,numMonitors);//sinartisi des directory.h,directory.c
 	if(numMonitors>numCountries){numMonitors=numCountries;}//an oi monitor einai perissoteroi aptis xwres tote tus miwnw ston ari8mo twn xwrwn kai den tha tus dhmiourgisw giati einai spatali porwn

	printf("numCountries %d \n",numCountries);
	printf("_______________________________________________________________________________\n");

	pid_t child[numMonitors];
	int PORT[numMonitors];
	int socket_fd[numMonitors];

	for (int i = 0; i < numMonitors; ++i)
		{
		PORT[i]=FPORT+i;
		int pid=fork();

		if(pid==-1)//to fork apetyxe teliwnw to programma
			{
			perror("fork failed\n");
			return 1;
			}
		else if (pid!=0)
			{
			child[i]=pid;
			socket_fd[i]=socket_init(PORT[i],1);
			}
		else if (pid==0)//ine paidi eleftherwnw tus porous kai kalw thn execl me orismata ta named pipes
			{
			waiting();
			hashtable_free(cp);
			char countries[9999];strcpy(countries,namelist[i]);
			remove_namelist(&namelist,numMonitors);
			
			char port_str[10];sprintf(port_str,"%d",PORT[i]);
			execl("./monitorServer","./monitorServer","-p",port_str,"-t",numThreads_str,"-b",socketBufferSize_str,"-c",cyclicBufferSize_str,"-s",sizeOfBloom_str,countries,(char*) 0);
			perror("error execl was called\n");exit(1);
			}
	
		}
	

		
	//ftiaxnw ena pinaka apo hashtables opou sto bloomhasharr[i] vriskete ena hashtable me ola ta blooms gia to monitor i 
	hashtable** bloomhasharr=(hashtable**) malloc(numMonitors*sizeof(hashtable*));
	
	for (int i = 0; i < numMonitors; ++i)
		{

		bloomhasharr[i]=NULL;
		receiveblooms(socket_fd[i],i,socketBufferSize,sizeOfBloom,bloomhasharr);//sinartisi pu xrhsimopoiw gia na diavasw ta blooms des parent_functions.h/.c

		//printf("%d bloom received succesfully\n",i );
		}
	
	printf("_______________________________________________________________________________\n");
	printf("received bloomfilters from all the Monitors give one of the following commands:\n");
	printf("/travelRequest citizenID date countryFrom countryTo virusName\n");
	printf("/travelStats virusName date1 date2 [country]\n");
	printf("/addVaccinationRecords country\n");
	printf("/searchVaccinationStatus citizenID\n/exit\n");

	char a[7][50];
	int word_count=0;
	char* words;
	size_t len;
	char* line=NULL;
	while(1)
		{
		printf("_______________________________________________________________________________\n");
		printf("write command:\n");
		word_count=0;
		//spazw to command se lekseis
		getline(&line,&len,stdin);
		words= strtok (line," \n\0");
		while(words!=NULL)
			{
			if(word_count<8)//the biggest command has 8 words
				strcpy(a[word_count],words);
			else
				{break;}//den iparxei command me perissoteres apo 7 lekseis ara ine axriasto na tis diavasw
			words= strtok (NULL," \n\0");
			word_count++;
			}


		if (strcmp(a[0],"/exit")==0)//exit command
			{
			int total_accepted=0;int total_rejected=0;
			if (word_count!=1){printf("given less or more arguments for %s command\n",a[0]);break;}
			
			for (int i = 0; i < numMonitors; ++i)
				{

				//stelnw thn entolh
				encode(socket_fd[i],"/exit",socketBufferSize);

				char* accepted=decode(socket_fd[i],socketBufferSize);
				total_accepted+=atoi(accepted);free(accepted);

				char* rejected=decode(socket_fd[i],socketBufferSize);
				total_rejected+=atoi(rejected);free(rejected);
				//pernw ta accepted kai rejected apo ka8e paidi kai ta poros8etw sto oliko mou

				}
			//ftiaxnw to logfile tu patera des signal.h/.c
			logfile(getpid(),cp,total_accepted+total_rejected,total_accepted,total_rejected,1);

			//waiting for monitors to terminate 
			for (int i = 0; i < numMonitors; ++i)
				{
				int status;
    			waitpid(child[i], &status, 0);
				}

			break;
			}
		else if (strcmp(a[0],"/travelRequest")==0)//travelRequest command
			{
			if (word_count!=6){printf("given less or more arguments for %s command\n",a[0]);break;}
			char* countryFrom=a[3];
			char* virus_name=a[5];
			char* citizenID=a[1];
			char* date=a[2];

			//des  parent_functions.h/.c
			bucket* cpbucket=hashtable_check(cp,hash_i(countryFrom,HVALUE));//vriskw ton monitor gia thn xwra
			if(cpbucket!=NULL)//ean den iparxei kai einai NULL shmainei den exoume thn xwra auth
				{
				cm* cpdata=(cm*) cpbucket->data;

				if(cpdata!=NULL && strcmp(cpdata->country,countryFrom)==0)//ean teriazei h xwra
					{
					int monitor=cpdata->monitor;
					if(bloomhasharr[monitor]!=NULL)
						{
						bucket* bha_bucket =hashtable_check(bloomhasharr[monitor],hash_i(virus_name,HVALUE));//vriskw to katallilo bloom gia to sigkekrimeno virus tu sigkekrimenou monitor
						if (bha_bucket!=NULL)
							{
							virusparent* bha_data=(virusparent*) bha_bucket->data;
							if(bha_data!=NULL && strcmp(bha_data->virus_name,virus_name)==0)
								{
								if(bloom_check(bha_data->not_vaccinated_bloom,citizenID))
									printf("REQUEST REJECTED – YOU ARE NOT VACCINATED\n");
								else //ean to bloom_check ine arnitiko elegxw ston katallilo monitor
									{
							
									encode(socket_fd[monitor],"/travelRequest",socketBufferSize);

							
									encode(socket_fd[monitor],virus_name,socketBufferSize);
	
									encode(socket_fd[monitor],citizenID,socketBufferSize);
				
									encode(socket_fd[monitor],date,socketBufferSize);

									char* message =decode(socket_fd[monitor],socketBufferSize);
									printf("%s\n",message);if(message!=NULL){free(message);}
									}
								}
							}
						}
					}
				}
			}
		else if (strcmp(a[0],"/travelStats")==0)//travelStats command
			{
			if (word_count!=4 && word_count!=5){printf("given less or more arguments for %s command\n",a[0]);break;}

			char* date2=a[3];
			char* date1=a[2];
			char* virusName=a[1];
			if(word_count==5)//shmainei oti 8elei gia sigkekrimeni xwra
				{
				char* country=a[4];
				bucket* cpbucket=hashtable_check(cp,hash_i(country,HVALUE));
				if(cpbucket!=NULL)
					{
					cm* cpdata=(cm*) cpbucket->data;
					if(cpdata!=NULL && strcmp(cpdata->country,country)==0)
						{
						int monitor=cpdata->monitor;
					
						//stelnw thn entolh
						encode(socket_fd[monitor],"/travelStatsC",socketBufferSize);

						//stelnw tis aparetites plirofories		
						encode(socket_fd[monitor],virusName,socketBufferSize);

						encode(socket_fd[monitor],date1,socketBufferSize);

						encode(socket_fd[monitor],date2,socketBufferSize);

						encode(socket_fd[monitor],country,socketBufferSize);


						//pernw ton ari8mo twn accepted
						char* accepted=decode(socket_fd[monitor],socketBufferSize);

						//pernw ton ari8mo twn rejected
						char* rejected=decode(socket_fd[monitor],socketBufferSize);

						printf("TOTAL REQUESTS %d\n",atoi(accepted)+atoi(rejected));// to total ine to a8roisma autwn twn duo
						printf("ACCEPTED %s\n",accepted);free(accepted);
						printf("REJECTED %s\n",rejected);free(rejected);
						}
					}

				}
			else if(word_count==4)//8elei gia oles tis xwres
				{
				int total_accepted=0;
				int total_rejected=0;
				for (int i = 0; i < numMonitors; ++i)//se ka8e monitor
					{
					encode(socket_fd[i],"/travelStats",socketBufferSize);
									
					encode(socket_fd[i],virusName,socketBufferSize);

					encode(socket_fd[i],date1,socketBufferSize);

					encode(socket_fd[i],date2,socketBufferSize);


					char* accepted=decode(socket_fd[i],socketBufferSize);
					total_accepted+=atoi(accepted);free(accepted);//pros8etw ka8e fora sto oliko ton accepted

					char* rejected=decode(socket_fd[i],socketBufferSize);
					total_rejected+=atoi(rejected);free(rejected);//pros8etw ka8e fora sto oliko ton rejected
					}
				printf("TOTAL REQUESTS %d\n",total_accepted+total_rejected);
				printf("ACCEPTED %d\n",total_accepted);
				printf("REJECTED %d\n",total_rejected);
				}
			}
		else if (strcmp(a[0],"/addVaccinationRecords")==0)//command addVaccinationRecords
			{
			if (word_count!=2){printf("given less or more arguments for %s command\n",a[0]);break;}
			char* country=a[1];
			bucket* cpbucket=hashtable_check(cp,hash_i(country,HVALUE));
			if(cpbucket!=NULL)
				{
				cm* cpdata=(cm*) cpbucket->data;
				if(cpdata!=NULL && strcmp(cpdata->country,country)==0)
					{

					int monitor=cpdata->monitor;


					encode(socket_fd[monitor],"/addVaccinationRecords",socketBufferSize);

					receiveblooms(socket_fd[monitor],monitor,socketBufferSize,sizeOfBloom,bloomhasharr);//vlepe parent_functions.h/.c
					}
				}
			}
		else if (strcmp(a[0],"/searchVaccinationStatus")==0)//command searchVaccinationStatus
			{
			if (word_count!=2){printf("given less or more arguments for %s command\n",a[0]);break;}
			char* citizenID=a[1];

			char test[30];sprintf(test,"%d",atoi(citizenID));

			if(strcmp(test,citizenID)!=0){printf("citizenID contains non numeric characters\n");break;}

			for (int i = 0; i < numMonitors; ++i)
					{


					encode(socket_fd[i],"/searchVaccinationStatus",socketBufferSize);

					encode(socket_fd[i],citizenID,socketBufferSize);

					char* buff=decode(socket_fd[i],socketBufferSize);

					int flags=0;
					//otan den exei perissoteres plhrofories ena paidi gia to sigkekrimeno id stelnei "end_searchVaccinationStatus" mesw tou named pipe
					//ta paidia pu den exoun to id stelnoun e3arxhs to end_searchVaccinationStatus
					while(strcmp(buff,"end_searchVaccinationStatus")!=0)
						{
						flags=1;
						printf("%s\n",buff);
						free(buff);

						buff=decode(socket_fd[i],socketBufferSize);
						}
					free(buff);
					if(flags){break;}//epeidh to ka8e id einai mono se ena monitor an stal8un plhrofories pavei to psaksimo sta ipoloipa paidia
					}

			}
		else
			{printf("command given isnt recognised try again\n");}//self explained


		if(line!=NULL){free(line);line=NULL;}//elefthrwnw thn grammi
		if(words!=NULL){free(words);words=NULL;}//elefthrwnw ta words
		}

	printf("freeing parent\n");
	//freeing memory and closing sockets
	for (int i = 0; i < numMonitors; ++i)
		{
		close(socket_fd[i]);
		if(bloomhasharr[i]!=NULL)
			{
			hashtable_free(bloomhasharr[i]);
			}
		}
	remove_namelist(&namelist,numMonitors);
	free(bloomhasharr);
	hashtable_free(cp);

	return 0;
	sleep(5);
	printf("exiting parent\n");
	}