#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <libuvc/libuvc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include "buffer.h"

void close_camera();
void show_frame( uint16_t* frame );
void cb( uvc_frame_t* frame, void* ptr );
uint16_t* extract_Y16_from_struct( uvc_frame_t* in );
int camera_on();
void* camera_run_as_child();

#endif // __CAMERA_H__
