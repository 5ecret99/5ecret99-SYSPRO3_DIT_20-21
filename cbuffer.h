#ifndef _CBUFFER__
#define _CBUFFER__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

pthread_mutex_t cb_lock;//mutex lock gia to kikliko buffer

typedef struct centry centry;
typedef struct centry
	{
	char* data;//to data diladi to onoma tu directory arxeiou
	centry* next;//to epomeno centry
	int last;//an ine to teleuteo centry 1 aliws 0
	}centry;

typedef struct cbuffer
	{
	int total_entries;//sinolikos arithmos entries p mphkan ston kikliko buffer
	int curr_entries;//arithmos entries p iparxun twra ston cbuffer
	int max_entries;//megistos arithmos entries p mporei na kratisei
	centry* header;//prwto entry (fifo)
	}cbuffer;

void cbuffer_init(cbuffer* cbuff,int max_entries);//arxikopoiei to kikliko buffer
int cbuffer_empty(cbuffer* cbuff);//epistrefi 1 an to cbuffer ine adio aliws 0
int cbuffer_insert(cbuffer* cbuff,char* data);//isagei neo entry sto cbuffer,epistrefi 1 an apotixei tin isagwgi aliws 0
centry*  cbuffer_pop(cbuffer* cbuff);//vgazi  to analogo entry apto cbuffer(fifo first in first out)
int cbuffer_full(cbuffer* cbuff);//epistrefi 1 an to cbuffer ine gemato aliws 0

#endif