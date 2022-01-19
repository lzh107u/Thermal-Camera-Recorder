#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "Python.h"

int list_build( PyObject* pyTotalList, const int number_frame, uint16_t** data )
{
	uint16_t* frame = NULL;
	PyObject* single_frame = NULL;
	int ret;
	printf("output, list_build: called.\n");
	for ( int i = 0; i < number_frame; i++ )
	{
		frame = data[ i ]; // process per frame
		single_frame = PyList_New( 0 ); // allocate a PyList for a frame
		
		if ( single_frame == NULL ) // allocated unsuccessfully
		{
			printf("output, output_data: the PyList of frame %d is allocated unsuccessfully.\n", i );
		}
		else
		{
			for ( int j = 0; j < 160*120; j++ ) // copy content
			{
				ret = PyList_Append( single_frame, Py_BuildValue( "i", frame[ j ] ) );
				if ( ret != 0 )
				{
					printf("output, list_build: frame %d, appening unsuccessful.\n", i );
					break;
				}
			}
			ret = PyList_Append( pyTotalList, single_frame ); // put a frame into total list
		}
	}
	printf("output, list_build: finished.\n");
	return 0;
}

int output_data( const int number_frame, uint16_t** data )
{
	char* filename = "write_mat";
	char* methodname = "write_data";

	Py_Initialize();
	int ret;
	ret = PyRun_SimpleString( "import sys; sys.path.insert( 0, '.' )" );
	if ( ret != 0 )
	{
		printf("output, output_data: PyRun_SimpleString() works unsuccessfully.\n");
		return 0;
	}
	
	// load the module
	PyObject* pyFileName = PyUnicode_FromString( filename );
	// transform a c_string( ansi ) to a utf-8 string ended with a NULL.
	PyObject* pyMod = PyImport_Import( pyFileName );
	// import a module
	
	// load the function
	PyObject* pyFunc = PyObject_GetAttrString( pyMod, methodname );
	// get an attribute named [ methodname.c_str() ] from the pyMod
	
	// test the function is callable.
	if ( pyFunc && PyCallable_Check( pyFunc ) )
	{
		ret = PyCallable_Check( pyFunc );

		// PyObject* pyParams = PyTuple_New( 2 );
		PyObject* pyTotalList = PyList_New( 0 );
		if ( pyTotalList == NULL )
		{
			printf("output, output_data: pyTotalList allocated failure.\n");
			return 0;
		}
		else
		{
			list_build( pyTotalList, number_frame, data );
		}
		PyObject* pyParams = PyTuple_New( 1 ); // a tuple for python function input argument
		ret = PyTuple_SetItem( pyParams, 0, pyTotalList );
		PyObject* pyValue = PyObject_CallObject( pyFunc, pyParams );
	}

	Py_DECREF( pyMod );
	Py_DECREF( pyFileName );
	Py_Finalize();
	printf("output, output_data: finish working.\n");
	return 0;
}
