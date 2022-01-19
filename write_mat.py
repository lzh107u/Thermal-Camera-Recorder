import numpy as np
import scipy.io as sio
import cv2

# sio.savemat( matdir, { 'x': x, 'y':y } )

def write_data( data ):
    print('write_mat, write_data: start working')
    matdir = 'test.mat' # data directory
    matfile = sio.loadmat( matdir ) # load file
    
    output_content = dict() # the data format of an output content is dictionary
    frame_index = None # string type index of every frame
    index = 0
    print('write_mat, write_data: length:', len( data ) )
    print('write_mat, write_data: ready to iter.')
    # extract each frame from data

    img_display = np.zeros( [ 120, 160, 3 ], dtype = 'uint8' )
    # size: [ num_row, num_col, num_channel ]

    for frame_list in data:
        
        if frame_list is None:
            print('it is a None')
        else :
            # print( type( frame_list ), len( frame_list ) )
            
            frame_array = np.array( frame_list ) # turn raw-data from list to array
            
            if ( len( frame_list ) != 19200 ):
                print('write_mat, write_data: the length of current data is not matched with 160*120, discarded.')
            else:
                index = index + 1 # only count frames with correct size
                frame_index = 'frame' + str( index )
                frame_array = np.reshape( frame_array, ( 120, 160 ), order = 'C' ) # reshape the 1-dim raw-data

                frame_array = cv2.flip( frame_array, -1 )
                frame_array = cv2.flip( frame_array, 1 )

                output_content[ frame_index ] = frame_array
                # size: [ num_row, num_col ]
                """
                frame_copy = np.array( frame_array, dtype = 'uint8' )
                img_display[ : , : , 0 ] = frame_copy[ : , : ]
                cv2.imwrite( 'sample.jpg', img_display )
                """
    # print( output_content )
    sio.savemat( matdir, output_content )
    
