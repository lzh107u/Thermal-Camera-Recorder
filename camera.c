#include <libuvc/libuvc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

#include "buffer.h"
#include "camera.h"

int CAMERA_EXIT = 0;

void close_camera()
{
	CAMERA_EXIT = 1;
}

void show_frame( uint16_t* frame )
{
	/* show the content of a frame */
	for ( int i = 0; i < 120; i++ ) // 120 rows
	{
		printf("[ row %d ] -------------------- \n", i+1 );
		for ( int j = 0; j < 10; j++ )
		{
			printf("[+] ");
			for ( int k = 0; k < 16; k++ )
			{
				printf("%d ", frame[ j*16 + k ] );
			}
			printf("\n");
		} // end of columns
		printf("------------------------------- \n" );
	} // end of rows
	return ;
}

uint16_t* extract_Y16_from_struct( uvc_frame_t* in )
{
	// printf("[+]uvc_Y16toBGR, frame type: %d\n", in->frame_format );
	// printf("[+]uvc_Y16toBGR, UVC_FRAME_FORMAT_Y16: %d\n", UVC_FRAME_FORMAT_Y16 );
	// initially set: 13
	// => 12: UVC_FRAME_FORMAT_BY8
	// => 11: UVC_FRAME_FORMAT_GRAY8
	// => 6 : UVC_FRAME_FORMAT_NV12
	// => 13: UVC_FRAME_FORMAT_Y16
	
	if ( in->frame_format != UVC_FRAME_FORMAT_Y16 ) // frame format not match
	{
		return NULL;
	}
	
	uint16_t* out = ( uint16_t* )malloc( sizeof( uint16_t ) * in->width * in->height ); // locate a space to contain the raw data
	uint16_t* src = in->data; // source data start. the data pointer is default to ( void* ) type.

	int count = 0;
	
	/* copy the Y16 raw data */
	//printf("size %d %d",in->width, in->height);
	while( count < ( in->width * in->height ) )
	{
		out[ count ] = src[ count ];
		count += 1;
		if ( count > 20000 )
		{
			printf("-----safety activate -----\n");
			break;
		}
	}
	return out;
}

void cb( uvc_frame_t *frame, void *ptr )
{
	uint16_t* ret;
	// printf("[+]call_back function, frame_structure inspection: frame sequence: %d\n", frame->sequence );
	
	ret = extract_Y16_from_struct( frame );
	
	if ( ret == NULL )
	{
		printf("ret is still NULL !!\n");
	}
	else
	{
		/* do something with extracted Y16 raw data ... */

		/* store it to a shared buffer */
		access_frame( ret );
	}
}

int camera_on()
{
	uvc_context_t *ctx;
	uvc_device_t *dev;
	uvc_device_handle_t *devh;
	uvc_stream_ctrl_t ctrl;
	uvc_error_t res;

	/* Initialize a UVC service context. Libuvc will set up its own libusb context.
	 * Replace NULL with a libusb_context pointer to run libuvc from an existing libusb context. */
	
	res = uvc_init( &ctx, NULL );

	if ( res < 0 )
	{
		uvc_perror( res, "uvc_init" );
		return res;
	} // end of uvc initialization
	puts( "-----UVC initialized-----" );

	/* Locates the first attached UVC device, stores in dev */
	res = uvc_find_device( ctx, &dev, 0, 0, NULL );

	if ( res < 0 )
	{
		uvc_perror( res, "uvc_find_device" ); // no device found
	} // end of device found failure
	else
	{
		puts("-----Device found-----");
		res = uvc_open( dev, &devh ); // try to open the device: requires exclusive access
		
		if ( res < 0 )
		{
			uvc_perror( res, "uvc_open" ); // unable to open device 
		} // end of device opened failure
		else
		{
			puts("-----Device opened-----");

			/* Print out a message containing all the information 
			 * that libuvc knows about the device */
			uvc_print_diag( devh, stderr );

			/* Try to negotiate a 160*120 9 fps Y16 stream profile */
			res = uvc_get_stream_ctrl_format_size( devh, &ctrl, UVC_FRAME_FORMAT_Y16, 160, 120, 9 );
			/* ctrl: result stored here
			 * frame format: Y16 ( enum value: 13,  you can check it on libuvc.h ) 
			 * width: 160
			 * height: 120
			 * frame rate ( fps ): 9
			 * Warning: Do not try to change the frame rate here. 
			 * The function will return a failure( -51 ) as a result of invalid mode. */

			uvc_print_stream_ctrl( &ctrl, stderr ); // print out the result

			if ( res < 0 )
			{
				printf("---------- error message start ----------\n");
				uvc_perror( res, "get_mode" ); // device doesn't provide a matching stream
				printf("---------- error message end ----------\n");
			} // end of stream profile negotiation failure
			else
			{
				puts("-----stream negotiation success-----");
				/* Start the video stream. */
				/* The library will call user function cb: cb( frame, ( void* ) user_ptr ) */
				void* user_ptr = NULL;
				res = uvc_start_streaming( devh, &ctrl, cb, user_ptr, 0 );

				if ( res < 0 )
				{
					uvc_perror( res, "start_streaming" ); // unable to start stream
				} // end of streaming failure
				else
				{
					puts( "-----Streaming...-----" );
					uvc_set_ae_mode( devh, 1 ); // turn on auto exposure
					
					/* the attempt to get a frame without memory waste.
					 * uvc_frame_t* frame = NULL; // a frame structure
					 * uvc_stream_handle_t* strmh = NULL; // a stream handle structure
					 * uvc_error_t error_ret; // a error type

					 * error_ret = uvc_stream_open_ctrl( devh, &strmh, &ctrl ); 
					 * 1. to open a new UVC stream.
					 * 2. to get a uvc_stream_handle_t*. 
					 * error_ret = uvc_stream_get_frame( strmh, &frame, 10*1000 );
					 * get a frame from a UVC stream and wait for only 10 sec( 10000 msec ). */
					
					/* keep streaming until the EXIT becomes 1. */	
					while ( CAMERA_EXIT == 0 )
						;

					uvc_stop_streaming( devh );
					puts( "-----Done Streaming-----" );

				} // end of streaming success
			} // end of stream profile negotiation failure
			uvc_close( devh );
			puts( "-----Device closed-----" );
		} // end of device opened success
		uvc_unref_device( dev );
	} // end of device found success

	/* Close the UVC context.
	 * This closes and cleans up any existing device handles
	 * and it closes the libusb context if one was not provided. */
	uvc_exit( ctx );
	puts( "-----UVC exited-----" );
	return 0;
}

void* camera_run_as_child()
{
	/* The other function should call this one instead of calling 
	 * camera_on() directly when it's called as a child thread. */
	int ret = camera_on();
	pthread_exit( NULL );
}
