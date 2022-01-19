#ifndef __OUTPUT_H__
#define __OUTPUT_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "Python.h"

int list_build( PyObject* pyTotalList, const int number_frame, uint16_t** data );
int output_data( const int number_frame, uint16_t** data );

#endif // __OUTPUT_H__
