#ifndef __DISPATCHER_H__
#define __DISPATCHER_H__

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "buffer.h"

typedef struct task_struct
{
	pthread_t* ptr_task; // the pointer of child thread
	struct task_struct* ptr_next; // the pointer of next task_struct
} task_struct;

void* child();
task_struct* task_list_access( pthread_t* task );
void* thread_receiver();
void* timer( void* time );
void close_dispatcher();
int dispatcher( int frame_rate, int time );

#endif // __DISPATCHER_H__
