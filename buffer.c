#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h> // for uint16_t
#include "buffer.h"

pthread_mutex_t mutex_frame = PTHREAD_MUTEX_INITIALIZER; // a mutex for accessing the shared frame.
uint16_t* shared_frame = NULL;

uint16_t* access_frame( uint16_t* ptr )
{
	pthread_mutex_lock( &mutex_frame ); // lock on
	uint16_t* ret = NULL;
	if ( ptr == NULL ) // if input pointer is NULL, it means that it's an output task.
	{
		ret = shared_frame;
	}
	else // if input pointer is not NULL, it means that it's an input task.
	{
		// printf("buffer, access_frame: a frame is been stored.\n");
		if ( shared_frame != NULL ) // if shared_frame stores a frame, then it should free it first.
		{
			// printf("buffer, access_frame: clean up previous frame.\n");
			free( shared_frame );
		}
		shared_frame = ptr;
	}
	pthread_mutex_unlock( &mutex_frame ); // unlock
	return ret;
}
