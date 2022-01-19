#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "gui.h" // for GUI
#include "camera.h" // for camera controlling
#include "buffer.h" // for accessing global frame buffer
#include "dispatcher.h" // for task dispatching

void wait_for_stream()
{
	/* wait until the global buffer has the first frame from camera. */
	
	uint16_t* frame = NULL;

	while( frame == NULL )
	{
		frame = access_frame( NULL );
	}
}

pthread_t camera_open()
{
	/* open the camera */
	printf("main, camera_open: called.\n");
	pthread_t task_camera;
	pthread_create( &task_camera, NULL, camera_run_as_child, NULL ); // camera start processing.
	wait_for_stream(); // wait for camera open.
	return task_camera;
}

int camera_close( pthread_t task_camera )
{
	/* close the camera */
	
	close_camera(); // change CAMERA_EXIT from 0 to 1 .
	pthread_join( task_camera, NULL );
	return 0;
}

int GUI_OUT = 0;
void* run_an_gui()
{
	/* open gui here */
	GUI();
	GUI_OUT = 1;
	pthread_exit( NULL );
}

pthread_t start_gui()
{
	/* create a thread to run gui */
	printf("main, start_gui: called.\n");
	pthread_t task_gui;
	pthread_create( &task_gui, NULL, run_an_gui, NULL );
	return task_gui;
}

int wait_for_gui_close( pthread_t task_gui )
{
	/* wait until the gui close. */
	while( GUI_OUT == 0 )
		;
	pthread_join( task_gui, NULL );
	return 0;
}

void* run_dispatcher()
{
	int frame_rate = 9; // frame per second
	int interval = 3;   // timer setting
	dispatcher( frame_rate, interval );
	pthread_exit( NULL );
}

void end_dispatcher()
{
	close_dispatcher();
}

pthread_t start_dispatcher()
{
	pthread_t task_dispatcher;
	pthread_create( &task_dispatcher, NULL, run_dispatcher, NULL );
	return task_dispatcher;
}

void command_UI()
{
	char* buffer = ( char* )malloc( sizeof( char ) * 50 );
	pthread_t task_dispatcher;
	int thread_count = 0;
	int dispatcher_flag = 0;
	printf("\n\n");
	printf("************************************************************\n");
	printf("thermal recorder UI starts.\n");
	printf("exit: leave the UI and close window and camera.\n");
	printf("startdis: start dispatcher.\n");
	printf("closedis: close dispatcher.\n");
	printf("************************************************************\n");

	while ( fgets( buffer, 49, stdin ) != NULL )
	{
		if ( strcmp( "exit\n", buffer ) == 0 )
		{
			printf("[+] thermal recorder UI exits\n");
			break;
		}
		else if ( strcmp( "startdis\n", buffer ) == 0 )
		{
			if ( dispatcher_flag == 0 )
			{
				printf("[+] dispatcher starts.\n");
				task_dispatcher = start_dispatcher();
				dispatcher_flag = 1;
			}
			else
			{
				printf("[+] dispatcher has already started.\n");
			}
		}
		else if ( strcmp( "closedis\n", buffer ) == 0 )
		{
			if ( dispatcher_flag == 1 )
			{
				printf("[+] dispatcher closes.\n");
				close_dispatcher();
				pthread_join( task_dispatcher, NULL );
				dispatcher_flag = 0;
			}
			else
			{
				printf("[+] dispatcher hasn't started.\n");
			}
		}
	}
	free( buffer );
}

int main( int argc, char** argv )
{
	/* open camera */
	pthread_t task_camera = camera_open();
	/* open window */
	pthread_t task_gui = start_gui();
	/* other function ... */
	int ret;
	/* start a UI */
	command_UI();
	/* close window */
	ret = wait_for_gui_close( task_gui );

	/* close camera */
	ret = camera_close( task_camera );
	return 0;
}
