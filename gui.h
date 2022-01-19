#ifndef __GUI_H__
#define __GUI_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> // for uint16_t
#include <pthread.h>
#include <unistd.h>
#include <gtk/gtk.h> // for GUI

#include "buffer.h"

typedef struct rgb{
	uint8_t r;
	uint8_t g;
	uint8_t b;
} rgb;

void pixbuf_free( guchar* pixels, gpointer data );
void destroy( void );
void* open_camera();
gboolean draw_call( gpointer user_data );
void GUI();
void gui_run_as_child();

#endif // __GUI_H__
