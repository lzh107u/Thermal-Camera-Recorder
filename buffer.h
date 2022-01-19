#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>


uint16_t* access_frame( uint16_t* ptr ); // access to the shared frame with mutex

#endif // __BUFFER_H__
