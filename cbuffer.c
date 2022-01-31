#include "cbuffer.h"

void cbuffer_init(cbuffer* cbuff,int max_entries)
	{
	pthread_mutex_lock(&cb_lock);
	cbuff->total_entries=0;
	cbuff->curr_entries=0;
	cbuff->max_entries=max_entries;
	cbuff->header==NULL;
	pthread_mutex_unlock(&cb_lock);
	}
int cbuffer_empty(cbuffer* cbuff)
	{
	pthread_mutex_lock(&cb_lock);
	if(cbuff==NULL||cbuff->curr_entries==0){pthread_mutex_unlock(&cb_lock);return 1;}
	pthread_mutex_unlock(&cb_lock);
	return 0;
	}

int cbuffer_full(cbuffer* cbuff)
	{
	pthread_mutex_lock(&cb_lock);
	if(cbuff->max_entries==cbuff->curr_entries){pthread_mutex_unlock(&cb_lock);return 1;}
	pthread_mutex_unlock(&cb_lock);
	return 0;
	}
int cbuffer_insert(cbuffer* cbuff,char* data)
	{
	pthread_mutex_lock(&cb_lock);
	if(data==NULL||cbuff->curr_entries>=cbuff->max_entries){
		pthread_mutex_unlock(&cb_lock);
		return 1;}

	cbuff->total_entries=cbuff->total_entries+1;

	centry* temp=(centry*) malloc(sizeof(centry));
	temp->data=data;
	temp->last=1;//tha mpei sto telos

	if(cbuff->header==NULL)//to cbuffer den exei header ara auto to stoixeio p isagoume tha ine to header
		{
		temp->next=temp;
		cbuff->curr_entries=1;
		cbuff->header=temp;
		}
	else//kanoniki isagwgi
		{
		temp->next=cbuff->header;
		cbuff->curr_entries=cbuff->curr_entries+1;
		centry* curr=cbuff->header;

		while(curr->last!=1)//pigainei sto teleuteo entry tu cbuffer
			curr=curr->next;
		
		curr->last=0;
		curr->next=temp;
		}
	//printf("%s\n",data);
	pthread_mutex_unlock(&cb_lock);
	return 0;
	}
centry*  cbuffer_pop(cbuffer* cbuff)
	{
	pthread_mutex_lock(&cb_lock);
	if(cbuff->header==NULL){
		pthread_mutex_unlock(&cb_lock);
		return NULL;}

	centry* temp=cbuff->header;
	if(cbuff->header->last!=1)//den ine to teleuteo entry
		{
		cbuff->header=cbuff->header->next;
		cbuff->curr_entries=cbuff->curr_entries-1;
		}
	else//ine to teleteo ara to cbuffer den tha exei header(=NULL)
		{
		cbuff->header=NULL;
		cbuff->curr_entries=0;
		}

	//printf("%s %d\n",temp->data,cbuff->curr_entries);
	pthread_mutex_unlock(&cb_lock);
	return temp;
	}