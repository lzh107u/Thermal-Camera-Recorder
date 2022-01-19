#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <libuvc/libuvc.h>
#include <stdint.h>
#include <string.h>
#include <gtk/gtk.h>

#include "buffer.h"
#include "gui.h"

void pixbuf_free( guchar* pixels, gpointer data )
{
	/* a handler function for closing pixbuf */
	g_free( pixels );
	( void )data;
	// fprintf( stderr, "pixbuf data freed\n" );
}

void destroy( void )
{
	gtk_main_quit();
}

rgb* image_restoration( uint16_t* frame, int ratio )
{
	/* arguments set */
	size_t width = 160;
	size_t height = 120;
	size_t Stride_rgb = width;
	int min, max, num;
	long double temp, t1, t2;
	min = 65535;
	max = 0;
	// printf("gui, image_restoration: the first pixel from raw-data: %hu\n", frame[ 0 ] );
	for ( size_t i = 0; i < width*height; i++ )
	{
		num = *( frame + i ); // move to current address.
		if ( num > max ) max = num;
		if ( num < min ) min = num;
		/* set bound */
	}
	uint8_t* img8 = ( uint8_t* )malloc( sizeof( uint8_t )*width*height );
	int black_count = 0;
	
	for ( size_t i = 0; i < width*height; i++ )
	{
		num = frame[ i ]; // move to current address
		t1 = num - min; // the value of the origin in the designated range
		t2 = max - min; // the range of normalization
		temp = t1 / t2;
		img8[ i ] = temp * 255; // store the normalized result into a new array.
	}
	
	/* test 12/31:
	 * => delete the part of normalization
	 * => result: only the luminance of picture changerd
	 * 	=> the problem is not here.
	 * */
	rgb* buffer_rgb = malloc( width * height * ratio*ratio * sizeof( rgb ) ); // a new picture structure with 3 channels.
	int offset, big_offset;
	uint8_t grey;
	/* processing per row. */
	
	for ( size_t x = 0; x < height; x++ )
	{
		for ( size_t y = 0; y < width; y++ )
		{
			grey = *( img8 + ( height - x - 1 )* width + y ); // set grey value.
			big_offset = ratio * ( width*x*ratio + y );
			for ( int z_x = 0; z_x < ratio; z_x++ )
			{
				for ( int z_y = 0; z_y < ratio; z_y++ )
				{
					offset = big_offset + ( z_x * width * ratio ) + z_y;
					buffer_rgb[ offset ].r = grey;
					buffer_rgb[ offset ].g = grey;
					buffer_rgb[ offset ].b = grey;
				}
			}
			// offset = width*x + y;
			// buffer_rgb[ offset ].r = grey;
			// buffer_rgb[ offset ].g = grey;
			// buffer_rgb[ offset ].b = grey;
			/* test 12/31:
			 * => change the value of channel g and channel b into 0
			 * => the channel r remains grey
			 * => result: the image turn into a red style, but the content is the same.
			 * 	=> the problem is not here.
			 * */
		}
	}
	return buffer_rgb;
}

/* gtk window */
GtkWidget *window; // an object of window
GtkWidget *image; // an object in window for storing image
gboolean draw_call( gpointer user_data )
{
	int ratio = 3;
	uint16_t* get_img;
	get_img = NULL;
	get_img = access_frame( NULL );
	size_t width = 160 * ratio;
	size_t height = 120 * ratio;
	size_t Stride_rgb = width;
	// printf("gui, draw_call: get image.\n");
	
	if ( get_img == NULL )
	{
		printf("gui, draw_call: no image is get.\n");
		return TRUE;
	}
	rgb* buffer_rgb = image_restoration( get_img, ratio );

	GdkPixbuf *pixbuf_rgb = gdk_pixbuf_new_from_data( ( guchar* )buffer_rgb, GDK_COLORSPACE_RGB, FALSE, 8, width, height, Stride_rgb*3, pixbuf_free, NULL );
	
	gtk_image_set_from_pixbuf( ( GtkImage* )image, pixbuf_rgb );
	g_object_unref( pixbuf_rgb );

	return TRUE;
}

void GUI()
{
	int ratio = 3;
	uint16_t* get_img; // for a single frame
	get_img = NULL;
	get_img = access_frame( NULL );

	size_t Width = 160 * ratio;
	size_t Height = 120 * ratio;
	size_t Stride_rgb = Width;
	rgb* buffer_rgb = image_restoration( get_img, ratio );
	
	gtk_init( 0, NULL );
	window = gtk_window_new( GTK_WINDOW_TOPLEVEL ); // a window
	gtk_window_set_title( GTK_WINDOW ( window ), "test" ); // set window title
	g_signal_connect( window, "destroy", G_CALLBACK( gtk_main_quit ), NULL ); // connect a handler to window for window destruction

	GdkPixbuf* pixbuf_rgb = gdk_pixbuf_new_from_data( ( guchar* )buffer_rgb, GDK_COLORSPACE_RGB, FALSE, 8, Width, Height, Stride_rgb*3, pixbuf_free, NULL );
	image = gtk_image_new_from_pixbuf( pixbuf_rgb );
	g_object_unref( pixbuf_rgb );

	gtk_container_add( GTK_CONTAINER( window ), image );
	gtk_widget_show_all( window );

	g_timeout_add( 111, draw_call, NULL );

	gtk_main();
}

void gui_run_as_child()
{
	/* The other function should call this one instead of calling 
	 * GUI() directly when it's called as a child thread. */
	GUI();
	pthread_exit( NULL );
}
