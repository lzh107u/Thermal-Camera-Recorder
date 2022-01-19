#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>

#include "dispatcher.h"
#include "buffer.h"
#include "output.h"

task_struct* task_head = NULL; 
task_struct* task_tail = NULL;

int EXIT = 0;
uint16_t** total_list = NULL;
pthread_mutex_t mutex_task = PTHREAD_MUTEX_INITIALIZER; // a mutex for accessing task list.

void* child( void* index_void )
{	
	int index = *( int* )index_void;
	uint16_t* frame_copy;
	/* child thread tasks ... */
	uint16_t* frame = access_frame( NULL );
	/* do something with the frame ... */
	if ( frame == NULL )
	{
		printf("dispatcher, child: frame is still NULL !!\n");
		/* Warning: Do not free frame outside the buffer.c .
		 * Every frame will be free in buffer.c .
		 * A Deletion outside the buffer.c will cause a execution failure of double free, resulting in a core dumped. */
	}
	else
	{
		frame_copy = ( uint16_t* )malloc( sizeof( uint16_t ) * 160*120 );
		for( int i = 0; i < 160*120; i++ )
		{
			frame_copy[ i ] = frame[ i ];
		}
	}
	total_list[ index ] = frame_copy;
	pthread_exit( NULL );
}

task_struct* task_list_access( pthread_t* task )
{
	pthread_mutex_lock( &mutex_task ); // lock it on. Only one thread can access the list at a time.
	task_struct* ret;
	if ( task != NULL ) // job == 0: put a new task.
	{
		task_struct* new_task = ( task_struct* )malloc( sizeof( task_struct ) ); // malloc a new task structure.
		new_task->ptr_task = task; // link the task ptr.
		new_task->ptr_next = NULL; // the new task will be put at the tail, so there will be no other task behind it.
		if ( task_tail != NULL ) // if tail exists, it should link with the new task.
			task_tail->ptr_next = new_task; // put the new task at the tail here.
		task_tail = new_task; // the new task becomes the tail.

		if ( task_head == NULL ) // if there is no head task, the tail will also be the head.
		{
			task_head = task_tail;
		}
		ret = NULL; // nothing needs to be returned.
	}
	else if ( task == NULL ) // job == 1: return the head
	{
		if ( task_head == NULL )
		{
			printf("clock, task_list_access: task_head doesn't exist, returning NULL\n");
		}
		ret = task_head; // points to the current head as return value.
		task_head = task_head->ptr_next; // the head pointer moves to the next one.
		if ( task_head == NULL ) // the task_head is NULL now, which means there is no task in the list now.
		{
			task_tail = NULL; // also set task_tail as NULL.
		}
	}

	pthread_mutex_unlock( &mutex_task ); // unlock
	return ret;
}

void* thread_receiver()
{
	task_struct* task = NULL; // a pointer to catch returned task_struct.
	int frame_count = 0; // counts for the number of child workers
	uint16_t* changed_frame = NULL;
	/* the receiver will be terminated if all the following conditions are satisfied:
	 * EXIT == 1: all missions are about to be terminated.
	 * task_tail == NULL: there is no more task in the linked list. */
	while( EXIT == 0 || task_tail != NULL ) 
	{
		if ( task_head != NULL )
		{
			/* access to task-linked-list */
			task = task_list_access( NULL ); 
			/* release a child thread */
			pthread_join( *task->ptr_task, NULL );
			frame_count += 1;
			free( task ); // free the memory of a released task_struct.
			
		}
	}
}

void* timer( void* time ) // set EXIT to 1 when the time passes.
{
	int time_to_tick = *( int* )time; // set bound
	int count = 0;
	
	while( ( count < time_to_tick ) && ( EXIT == 0 ) ) // exit when the time is expired or EXIT is set to 1
	{
		sleep( 1 );
		count += 1;
	}
	EXIT = 1;
	printf("dispatcher, timer: interval: %d, timer passes out.\n", time_to_tick );
}

void close_dispatcher()
{
	/* stop dispatcher and its companies. */
	EXIT = 1;	
}

int dispatcher( int frame_rate, int time )
{
	if ( frame_rate > 9 )
	{
		printf("clock, clock: the frame_rate( %d ) can't bigger than 9fps.\n", frame_rate );
		return -1;
	}
	total_list = ( uint16_t** )malloc( sizeof( uint16_t* ) * ( time*frame_rate + 20 ) );
	pthread_t t; // a child thread var
	pthread_t t_receiver; // var for thread_receiver
	pthread_t t_timer; // var for ticking
	int interval = ( int )( 1000 * 1000 / frame_rate ); // time unit: usec ( 10^-6 sec )
	// int count = 0; // counts for the number of child workers
	pthread_create( &t_receiver, NULL, thread_receiver, NULL ); // thread_receiver starts working
	pthread_create( &t_timer, NULL, timer, &time ); // timer starts ticking
	int frame_count = 0;
	/* starts to dispatch child worker... */
	while ( EXIT == 0 )
	{
		pthread_create( &t, NULL, child, ( void* )&frame_count ); // starts a child worker
		usleep( interval );
		frame_count += 1;
		/* access to task-linked-list */
		task_list_access( &t );
		// count += 1;
		// printf("clock, clock: a thread is created, created count: %d\n", count );
	}

	pthread_join( t_timer, NULL ); // ticking ends ticking ...
	pthread_join( t_receiver, NULL ); // thread_receiver ends working ...
	printf("dispatcher, dispatcher: ready to write data.\n");
	output_data( frame_count, total_list );
	for ( int i = 0; i < frame_count; i++ ) // release the total_list
	{
		free( total_list[ i ] );
	}
	free( total_list );
	return 0;
}
