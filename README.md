# Thermal-Camera-Recorder
Recording thermal raw data with C and Python.

## Components
### main.c
It works as a controller with a simple command-line, and it starts all other functions as child threads.
#### start recording
You need to specify how long the recorder should execute.
```sh
$record 30
# record [time( unit: minute )]
```
#### end recording
When the output process is finished, you should close the dispatcher by close.
```sh
$close
```
#### exit the program
```sh
$exit
```
### camera.c
It controls the thermal video stream with the [libuvc](https://github.com/libuvc/libuvc). It's necessary for you to install and build the [libuvc](https://github.com/libuvc/libuvc) and other dependencies first.
### buffer.c
It provides a global mutex buffer for both video stream and child threads. It's applied with a mutex-lock to prevent it from a synchronized accessing.
### dispatcher.c
It launchs child threads to access the global buffer in a given frame rate. Each child thread will access the global buffer to get a frame of raw data, and store it in a 2-D array. When the timer is expired, the 2-D array will be sent to output.c.
### output.c
It runs a python interpreter environment and is responsible for sending the 2-D raw data array into python domain. It's necessary for you to install and build the [Python/C API](https://docs.python.org/3/c-api/index.html) and other dependencies first.
### gui.c
It starts a window to display the video stream of the thermal camera. The frame is reshaped into 2-D (the raw data is 1-D) and resized into a quadrupled version. Each frame is normalized due to its visibility, and you should be aware of the difference between raw data and the normalized picture. It's necessary for you to install and build [GTK](https://github.com/GNOME/gtk) and other dependencies first.
### write_mat.py
It outputs the received raw data in matlab 5 format. The matlab format data is processed with [SciPy](https://scipy.org/), and it's necessary for you to install and build the [SciPy](https://scipy.org/) and other dependencies first.

## Makefile
To compile, link and execute the code.
```sh
$make
$sudo make run
# opening an uvc device requires root privilege.
```
