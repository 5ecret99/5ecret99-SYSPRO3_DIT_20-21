#include "child_functions.h"
//sinartisi gia ta threads
void *myThread(void *data)
	{
	thread_data* th_data=(thread_data*) data;
	//record
	char citizenID[5];
	char firstName[30];
	char lastName[30];
	char country[30];
	char age[5];
	char virus_name[30];
	char yes_no[5];
	char date[15];

	while(done==-1){sleep(1);}//oso koitaei gia arxeia to main thread (gia na min spatalw porous  prin na ektelesei tin arxiki diadikasia to main thread)

	while(1)
		{
		if(done==1)//an i main thread exi teliwsei
			{
			if(cbuffer_empty(th_data->cbuff))//an den iparxun alla arxeia sto cyclic buffer
				{pthread_exit(0);}//termatizei
			}
		centry*  entry=cbuffer_pop(th_data->cbuff);//pernei ena entry apto cbuffer

		if(entry==NULL){waiting();continue;}

		char* filename=entry->data;free(entry);
		FILE* citizenRecordsFile=fopen(filename,"r");
		if (citizenRecordsFile==NULL){perror("citizenRecordsFile==NULL");continue;}
		//diavasma twn egrafwn tu arxeiou
		char* line=NULL;
		size_t len=0;
		int linecount=0;
		while(getline(&line,&len,citizenRecordsFile)!=EOF)
			{
			linecount++;
			strcpy(date,"");strcpy(virus_name,"");strcpy(country,"");
			sscanf(line,"%s %s %s %s %s %s %s %s",citizenID,firstName,lastName,country,age,virus_name,yes_no,date);//breaking the line into words

			insertCitizenRecord(th_data->sizeOfBloom,th_data->viruses,th_data->records,th_data->countries,citizenID,firstName,lastName,country,age,virus_name,yes_no,date);
			}
		fclose(citizenRecordsFile);
		}
	}
void readdirectory(int cyclicBufferSize,int numThreads,char* directory,int sizeOfBloom,hashtable* viruses,hashtable* records,hashtable* countries,hashtable* directories)
	{
	thread_data* th_data=(thread_data*) malloc(sizeof(thread_data));
	th_data->viruses=viruses;
	th_data->records=records;
	th_data->countries=countries;
	cbuffer* cbuff=(cbuffer*) malloc(sizeof(cbuffer));cbuffer_init(cbuff,cyclicBufferSize);
	th_data->cbuff=cbuff;
	th_data->sizeOfBloom=sizeOfBloom;
	done=-1;
	char directory_copy[strlen(directory)];strcpy(directory_copy,directory);

	pthread_t tid[numThreads];

	for (int i = 0; i < numThreads; i++)
        pthread_create(&tid[i], NULL, myThread, (void *)th_data);

	char* words= strtok (directory_copy," \0\n");
	int word_count=0;
	while(words!=NULL)
		{
		
		word_count++;
		//printf("%s %d\n",words, word_count);
		
		struct dirent **dnamelist;
		int num_files=scandir(words, &dnamelist, 0, alphasort)-2;

		for(int i=0;i<num_files;i++)
			{
			//ftiaxnw to directory
			char* fdirectory=(char*)malloc(200*sizeof(char));
			strcpy(fdirectory,words);
			strcat(fdirectory,"/");
			strcat(fdirectory,dnamelist[i+2]->d_name);

			unsigned long directory_hash=hash_i(fdirectory,HVALUE);//HVALUE is a universal integer .i use for hashing located in hash.h
			bucket* dbucket=hashtable_insert(directories,directory_hash);

			if(dbucket==NULL){perror("error with hashtable bucket\n");}
			else if(dbucket->data!=NULL)//exei 3anaelextei ayto to directory-arxeio
				{free(fdirectory);continue;}

			else{dbucket->data=(void*)fdirectory;}

			while(cbuffer_full(cbuff)){waiting();}//oso o kiklikos buffer ine gematos perimenei

			while(cbuffer_insert(cbuff,fdirectory)){waiting();printf("cbuffer_insert\n" );}
			done=0;
			}

		for (int i = 0; i < num_files+2; i++) 
			{
            //printf("removing dnamelist[%d] : %s\n",i,dnamelist[i]->d_name);
            free(dnamelist[i]);
            }free(dnamelist);
            
			
		words= strtok(NULL," \n\0");
		}
	done=1;
	if(words!=NULL){free(words);words=NULL;}
	for (int i = 0; i < numThreads; i++)
		{
		pthread_join(tid[i], NULL);//perimenei na termatisoun ta ptthtread
		}

	if(cbuff!=NULL){free(cbuff);cbuff=NULL;}
	if(th_data!=NULL){free(th_data);th_data=NULL;}
	}

void insertCitizenRecord(int bloomSize,hashtable* viruses,hashtable* records,hashtable* countries,char* citizenID,char* firstName,char* lastName,char* country,char* age,char* virus_name,char* yes_no,char* date)
	{
	totalreq++;
	if(viruses==NULL||records==NULL||countries==NULL||citizenID==NULL||firstName==NULL||lastName==NULL||country==NULL||age==NULL||virus_name==NULL||yes_no==NULL){rejected++;printf("ERROR something is missing from this record cant insert");return;}
	int id=atoi(citizenID);
	int aa=atoi(age);
	int yn=-1;
	time_t dd=0;
	
	//checking if he is vaccinated
	if(strcmp(yes_no,"YES")==0)
		{
		yn=1;
		if(strcmp(date,"")==0)
			{
			rejected++;
			printf("date is yes but theres no date ERROR IN RECORD  %s %s %s %s %s %s %s %s \n",citizenID,firstName,lastName,country,age,virus_name,yes_no,date);//theres isnt a  date next to a YES
			return;
			}
		dd=dateconverter(date);
		}
	else
		{
		yn=0;
		if(strcmp(date,"")!=0)
			{
			rejected++;
			printf("date is no but theres a date ERROR IN RECORD  %s %s %s %s %s %s %s %s \n",citizenID,firstName,lastName,country,age,virus_name,yes_no,date);//theres a date next to a NO
			return;printf("error return command skipped\n");
			}
		}
	if (dd==-1)
		{
		rejected++;
		printf("date is wrong ERROR IN RECORD  %s %s %s %s %s %s %s %s \n",citizenID,firstName,lastName,country,age,virus_name,yes_no,date);//date is wrong
		return;
		}
	pthread_mutex_lock(&countries_lock);
	unsigned long country_hash=hash_i(country,HVALUE);//HVALUE is a universal integer .i use for hashing located in hash.h
	char* country_data;
	bucket* cbucket=hashtable_insert(countries,country_hash);
	
	if(cbucket->data==NULL)
		{
		country_data=(char*) malloc(((int)strlen(country))*sizeof(char));//printf("%s with len %ld \n",country,strlen(country));
		strcpy(country_data,country);
		cbucket->data=country_data;
		}
	else
		{
		country_data=(char*) cbucket->data;
		if (strcmp(country_data,country)!=0)
			{
			rejected++;
			printf(" country ERROR IN RECORD  %s %s %s %s %s %s %s %s \n",citizenID,firstName,lastName,country,age,virus_name,yes_no,date);
			pthread_mutex_unlock(&countries_lock);
			return;printf("error return command skipped\n");
			}
		}
	pthread_mutex_unlock(&countries_lock);
	

	pthread_mutex_lock(&records_lock);
	record* rec;
	bucket* rbucket=hashtable_insert(records,id);
	
	if(rbucket->data==NULL)
		{
		rec=(record*)malloc(sizeof(record));
		rec->citizenID=id;
		rec->age=aa;
		rec->country=country_data;
		rec->firstName=(char*)malloc(strlen(firstName)*sizeof(char));strcpy(rec->firstName,firstName);
		rec->lastName=(char*)malloc(strlen(lastName)*sizeof(char));strcpy(rec->lastName,lastName);
		//printf("record :%d %s %s %s %d inserted succesful\n",id,rec->firstName,rec->lastName,rec->country,aa);
		rbucket->data=rec;
		}
	else
		{
		rec=(record*) rbucket->data;
		if(rec->age!=aa ||strcmp(rec->country,country_data)||strcmp(rec->firstName,firstName)!=0||strcmp(rec->lastName,lastName)!=0)
			{
			rejected++;
			printf("ERROR IN RECORD  %s %s %s %s %s %s %s %s \n",citizenID,firstName,lastName,country,age,virus_name,yes_no,date);
			pthread_mutex_unlock(&records_lock);

			return;printf("error return command skipped\n");
			}
		}
	pthread_mutex_unlock(&records_lock);

	pthread_mutex_lock(&viruses_lock);
	bucket* vbucket=hashtable_insert(viruses,hash_i(virus_name,HVALUE));
	virus_child* vv_data;
	
	if(vbucket->data==NULL)
		{
		vv_data=(virus_child*) malloc(sizeof(virus_child));
		vv_data->virus_name=(char*)malloc(strlen(virus_name)*sizeof(char));strcpy(vv_data->virus_name,virus_name);
		vv_data->not_vaccinated_bloom=(bloom*) malloc(sizeof(bloom));bloom_init(vv_data->not_vaccinated_bloom,bloomSize);
		vv_data->vaccinated_persons=(skiplist*) malloc(sizeof(skiplist));skiplist_init(vv_data->vaccinated_persons);
		vv_data->not_vaccinated_persons=(skiplist*) malloc(sizeof(skiplist));skiplist_init(vv_data->not_vaccinated_persons);
		vbucket->data=vv_data;
		}
	else
		vv_data=(virus_child*) vbucket->data;

	
	if(yn==0)
		{
		if(skiplist_checking(vv_data->vaccinated_persons,id))
			{
			if(!skiplist_insert(vv_data->not_vaccinated_persons,id,rec,NULL)){rejected++;pthread_mutex_unlock(&viruses_lock);return;}
			}
		else
			{
			rejected++;pthread_mutex_unlock(&viruses_lock);return;
			}
		}
	else if(yn==1)
		{
		if(skiplist_checking(vv_data->not_vaccinated_persons,id))//if he is not in the not_vaccinated_persons skiplist 
			{
			time_t* day=(time_t*) malloc(sizeof(time_t));*day=dd;
			if(skiplist_insert(vv_data->vaccinated_persons,id,rec,day))
				{
				bloom_add(vv_data->not_vaccinated_bloom,citizenID);
				}
			else
				{
				free(day);
				rejected++;pthread_mutex_unlock(&viruses_lock);return;
				}
			}
		else
			{
			rejected++;pthread_mutex_unlock(&viruses_lock);return;
			}
		}
	pthread_mutex_unlock(&viruses_lock);
	

	accepted++;
	}
void sendblooms(int socket_fd,hashtable* viruses,int socketBufferSize,int sizeOfBloom)
	{
	

	//stelnei ton ari8mo twn virus ston patera

	int virus_count=viruses->count;
	char buff[30];sprintf(buff,"%d",virus_count);encode(socket_fd,buff,socketBufferSize);
	int bucket_count=viruses->bucket_num;

	for (int i = 0; i < bucket_count; ++i)//gia ka8e kouva tu hashtable
		{
		bucket* vbucket=viruses->buckets[i];

		while(vbucket!=NULL)//diavazi olus tus buckets tu linked list
			{
			virus_child* vdata=(virus_child*)vbucket->data;
			virus_count--;
			encode(socket_fd,vdata->virus_name,socketBufferSize);//stelnei to onoma tu iou

			encode_bloom(socket_fd,vdata->not_vaccinated_bloom->buff,socketBufferSize,sizeOfBloom);//stelnei to bloomfilter
			vbucket=vbucket->next;
			}
		if(virus_count<1){break;}//an exun stal8ei oloi oi ioi termatizei
		}


	}
char* Child_travelRequest(hashtable* viruses,char* virusName,int citizenID,char* tdate)
	{
	char* rejected=malloc(sizeof("REQUEST REJECTED – YOU ARE NOT VACCINATED"));
	if(viruses==NULL|| virusName==NULL){return "REQUEST REJECTED – YOU ARE NOT VACCINATED";}

	bucket* vbucket=hashtable_check(viruses,hash_i(virusName,HVALUE));
	if(vbucket==NULL){return "REQUEST REJECTED – YOU ARE NOT VACCINATED";}
	if(vbucket->data==NULL){return "REQUEST REJECTED – YOU ARE NOT VACCINATED";}

	virus_child* vdata=(virus_child*) vbucket->data;
	skiplist* vaccinated_persons=vdata->vaccinated_persons;
	if (vaccinated_persons==NULL|| strcmp(virusName,(char*)vdata->virus_name)){return "REQUEST REJECTED – YOU ARE NOT VACCINATED";}
	return skiplist_req(vaccinated_persons,citizenID,tdate);//des sikiplis.h/.c
	}

void travelStats(hashtable* viruses,char* country,char* virusName,time_t a,time_t b,int fda,int bufferSize)
	{
	if(viruses==NULL||virusName==NULL){perror("viruses==NULL||virusName==NULL");return;}

	int country_flag=0;
	if(country==NULL){country_flag=1;}//elegxei an 8elei sigkekrimenh

	bucket* vbucket=hashtable_check(viruses,hash_i(virusName,HVALUE));
	if(vbucket==NULL || vbucket->data==NULL)
		{
		char buff[30];sprintf(buff,"%d",0);//stelnei ton ari8mo 0 ews accepted kai rejected ean den iparxun plhrofories gia ton io

		encode(fda,buff,bufferSize);

		encode(fda,buff,bufferSize);
		return;
		}

	int accepted_count=0;
	int rejected_count=0;

	virus_child* vdata=(virus_child*)vbucket->data;

	skiplist* positive_skiplist=vdata->vaccinated_persons;
	snode* positive_node=positive_skiplist->header;while(positive_node->below!=NULL){positive_node=positive_node->below;}
	skiplist* negative_skiplist=vdata->not_vaccinated_persons;
	snode* negative_node=negative_skiplist->header;while(negative_node->below!=NULL){negative_node=negative_node->below;}

	int max=positive_skiplist->size;if(negative_skiplist->size>max){max=negative_skiplist->size;}

	for (int i = 0; i < max; ++i)
		{
		if(positive_node->next!=NULL)//gia ka8e vaccinated
			{
			positive_node=positive_node->next;
			if(country_flag || strcmp(positive_node->rec->country,country)==0)//an ine  gia tin swsti xwra h den exei periorismo
				{
				if(date_between(a,*positive_node->date,b))//elegxei thn im/nia
					accepted_count++;
				else
					rejected_count++;
				}
			}
		
		if (negative_node->next!=NULL)//gia ka8e not vaccinated
			{
			negative_node=negative_node->next;
			if(country_flag || strcmp(negative_node->rec->country,country)==0)
				rejected_count++;
			}
		
		}
	//stelnei tus accepted
	char buff[30];sprintf(buff,"%d",accepted_count);
	encode(fda,buff,bufferSize);

	//stelnei tous rjected
	sprintf(buff,"%d",rejected_count);
	encode(fda,buff,bufferSize);
	}

void end_searchVaccinationStatus(int fda,int bufferSize)
	{

	encode(fda,"end_searchVaccinationStatus",bufferSize);

	return;
	}

void searchVaccinationStatus(hashtable* records,hashtable* viruses,int citizenID,int fda,int bufferSize)
	{
	if(records==NULL||viruses==NULL||citizenID>9999||citizenID<0){end_searchVaccinationStatus(fda,bufferSize);return;}
	bucket* rbucket=hashtable_check(records,citizenID);
	if(rbucket==NULL||rbucket->data==NULL){end_searchVaccinationStatus(fda,bufferSize);return;}
	record* rec=(record*)rbucket->data;
	if(rec->citizenID!=citizenID){end_searchVaccinationStatus(fda,bufferSize);return;}

	char buff[100];sprintf(buff,"%d ",rec->citizenID);
	strcat(buff,rec->firstName);strcat(buff," ");strcat(buff,rec->lastName);strcat(buff," ");
	strcat(buff,rec->country);

	encode(fda,buff,bufferSize);

	char age[8];sprintf(age,"AGE %d",rec->age);

	encode(fda,age,bufferSize);


	int virus_count=viruses->count;//printf("%d\n",virus_count );
	int bucket_num=viruses->bucket_num;

	while(bucket_num && virus_count)
		{
		bucket_num--;
		bucket* vbucket=viruses->buckets[bucket_num];
		while(vbucket!=NULL)
			{
			if (vbucket->data!=NULL)
				{
				virus_child* vdata=(virus_child*) vbucket->data;
				if (vdata->virus_name!=NULL && vdata->vaccinated_persons!=NULL && vdata->not_vaccinated_persons!=NULL)
					{
					char* str=skiplist_VaccinationStatus(vdata->vaccinated_persons,citizenID,vdata->virus_name);//des skiplist.h/.c
					if(str==NULL)
						str=skiplist_VaccinationStatus(vdata->not_vaccinated_persons,citizenID,vdata->virus_name);
					if (str!=NULL)
						{

						encode(fda,str,bufferSize);free(str);

						}
					}
				
				else{perror("hashtable  data  virus_child something is NULL error\n");}
				}
			vbucket=vbucket->next;
			virus_count--;
			}
		}

	end_searchVaccinationStatus(fda,bufferSize);
	}
