#include "sockets.h"

void waiting()
	{
	int a=AVAL; while(a--);//des AVAL sto sockets.h
	}

struct in_addr getip()
	{
	//finding local ip adress
	//getting host name
	char hostbuffer[256];
	int hostname=gethostname(hostbuffer, sizeof(hostbuffer));
	//host_entry
	struct hostent *host_entry= gethostbyname(hostbuffer);
    if(host_entry==NULL){perror("host_entry==NULL\n");exit(1);}
    //converting ip into ASCII
	char *IPbuffer = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
	//elegxo an einai valid
	if(IPbuffer==NULL){perror("IPbuffer==NULL\n");exit(1);}

	return *((struct in_addr*) host_entry->h_addr_list[0]);
	}

int socket_init(int PORT,int parent)
	{
	//creating socket
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd==0){perror("error with socket()\n");exit(1);}

	//parent
	if(parent)
		{
		int opt=1;
		if(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){perror("error with setsockopt()\n");exit(1);}
		}

	//both
	address.sin_family = AF_INET;
    address.sin_port = htons( PORT );//printf("%d\n",address.sin_port);
    address.sin_addr=getip();
    address.sin_addr.s_addr = INADDR_ANY;

    //parent
    if(parent)
    	{

    	if(bind(socket_fd, (struct sockaddr *)&address, sizeof(address))<0){perror("error with bind()\n");exit(1);}

    	if (listen(socket_fd, 3) < 0){perror("error with listen()\n");exit(1);}

    	int addrlen = sizeof(address);
    	socket_fd =accept(socket_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    	if (socket<0){perror("error with accept()\n");exit(1);}
    	}
    //child
    else
    	{
    	waiting();
    	if (connect(socket_fd, (struct sockaddr *)&address, sizeof(address)) < 0){perror("error with connect()\n");exit(1);}
    	}
    return socket_fd;
	}

void en_poll(int fd)
	{
	int count=0;
	for(;;)
		{
		count++;
		//domh poll gia na perimeni
		struct pollfd fdarray [1];
		int rc;
		fdarray [0]. fd = fd;
		fdarray [0]. events = POLLOUT;
		rc = poll( fdarray , 1 , WTIME);
		if ( rc == 0) {perror("Poll timed - out\n");exit(1);}
		else if ( ( rc == 1) && (fdarray [0].revents == POLLOUT))
    		{
    		if ( fdarray [0]. fd == fd )
    			{
    			return;
    			}
    		else{perror("fdarray [0]. fd != fd\n");}
    		}

    	if(count==MAXCOUNT){perror("MAX TRIES COUNT REACHED FOR SENDING A message");exit(1);}
    	}
	}
void encode(int fd,char* message,int bufferSize)
	{
	en_poll(fd);
	char msg[30]="";
	int message_len=strlen(message);
	int msg_len=30;

	//semding the message length
	sprintf(msg,"%d",message_len);
	for (int i = 0; i < msg_len/bufferSize; ++i)
		{
		while(send(fd,msg+(i*bufferSize), bufferSize,0)<bufferSize){};
		}

	if(msg_len%bufferSize)
		{
		while(send(fd,msg+(msg_len-msg_len%bufferSize), bufferSize,0)<bufferSize);
		}
	//sending the actual message
	for (int i = 0; i < message_len/bufferSize; ++i)
		{
		while(send(fd,message+(i*bufferSize), bufferSize,0)<bufferSize){}
		}
	
	if(message_len%bufferSize)
		{
		while(send(fd,message+(message_len-message_len%bufferSize), bufferSize,0)<bufferSize){}
		}
	}

void dec_poll(int fd)
	{
	int count=0;
	for(;;)
		{
		count++;
		//domh poll gia na perimeni
		struct pollfd fdarray [1];
		int rc;
		fdarray [0]. fd = fd;
		fdarray [0]. events =  POLLIN ;
		rc = poll( fdarray , 1 , WTIME);
		if ( rc == 0) {perror("Poll timed - out\n");exit(1);}
		else if ( ( rc == 1) && (fdarray [0].revents ==  POLLIN ))
    		{
    		if ( fdarray [0]. fd == fd )
    			{
    			return;
    			//int n=recv(fd,buffer, bufferSize,0);
    			//if(n!=-1){return;}
   	 			
    			}
    		else{perror("fdarray [0]. fd != fd\n");}
    		}

    	if(count==MAXCOUNT){perror("MAX TRIES COUNT REACHED FOR RECEIVING A message");exit(1);}
    	}
	}
char* decode(int fd,int bufferSize)
	{
	dec_poll(fd);
	char buffer[bufferSize+1];
	int message_len;
	char msg[30]="";
	int msg_len=30;

	//receiving the message lenth
	for (int i = 0; i < msg_len/bufferSize; ++i)
		{
		while(recv(fd,buffer, bufferSize,MSG_WAITALL)<bufferSize){}
		memcpy(msg+(i*bufferSize),buffer,bufferSize);
		}

	if(msg_len%bufferSize)
		{
		while(recv(fd,buffer, bufferSize,MSG_WAITALL)<bufferSize){}
		memcpy(msg+(msg_len-msg_len%bufferSize),buffer,msg_len- (msg_len/bufferSize)*bufferSize);
		}

	message_len=atoi(msg);//printf("message_len %d\n",message_len );
	//now that we have the message length we create the message char array
	char *message=(char*) malloc(message_len*sizeof(char));
	//receiving the actual message
	for (int i = 0; i < message_len/bufferSize; ++i)
		{
		while(recv(fd,buffer, bufferSize,MSG_WAITALL)<bufferSize){}
		memcpy(message+(i*bufferSize),buffer,bufferSize);
		}
	
	if(message_len%bufferSize)
		{
		while(recv(fd,buffer, bufferSize,MSG_WAITALL)<bufferSize){}
		memcpy(message+(message_len-message_len%bufferSize),buffer,message_len- (message_len/bufferSize)*bufferSize);
		}
	message[message_len]='\0';
	return message;
	}
void encode_bloom(int fd,unsigned char* message,int bufferSize,int sizeOfBloom)
	{
	en_poll(fd);
	int message_len=sizeOfBloom;
	//sending the bloomfilter
	for (int i = 0; i < message_len/bufferSize; ++i)
		while(send(fd,message+(i*bufferSize), bufferSize,0)<bufferSize){}
	
	if(message_len%bufferSize)
		while(send(fd,message+(message_len-message_len%bufferSize), bufferSize,0)<bufferSize){}
	}
unsigned char* decode_bloom(int fd,int bufferSize,int sizeOfBloom)
	{
	unsigned char *message=(unsigned char*) calloc(sizeOfBloom,sizeof(unsigned char));
	unsigned char buffer[bufferSize];
	int message_len=sizeOfBloom;
	//receiving the bloomfilter
	for (int i = 0; i < message_len/bufferSize; ++i)
		{
		while(recv(fd,buffer, bufferSize,MSG_WAITALL)<bufferSize){}
		memcpy(message+(i*bufferSize),buffer,bufferSize);
		}
	
	if(message_len%bufferSize)
		{
		while(recv(fd,buffer, bufferSize,MSG_WAITALL)<bufferSize){}
		memcpy(message+(message_len-message_len%bufferSize),buffer,message_len- (message_len/bufferSize)*bufferSize);
		}

	return message;
	}
/*
int main()
	{
	int pid=fork();

	if(pid==-1)//to fork apetyxe teliwnw to programma
		{
		perror("fork failed\n");
		return 1;
		}
	else if (pid==0)//ine paidi eleftherwnw tus porous kai kalw thn execl me orismata ta named pipes
		{
		printf("son with pid: %d from parent with pid: %d \n",getpid(),getppid());
		int socket_fd=socket_init(8080,0);
		char* buffer =decode(socket_fd,3);
		printf("%s\n",buffer);free(buffer);

		for (int i = 0; i < 100; ++i)
			encode(socket_fd,"f off mate ",3);

		close(socket_fd);
		}
	else
		{
		int socket_fd=socket_init(8080,1);
		sleep(10);
		//socket_accept(socket_fd);
		char *hello = "Hello from parent";
		//send(socket_fd , hello , 20 , 0 );
		encode(socket_fd,hello,3);
		for (int i = 0; i < 100; ++i)
		{
		char* buffer =decode(socket_fd,3);
		printf("%s\n",buffer);free(buffer);
		}

		close(socket_fd);
		}
	}*/