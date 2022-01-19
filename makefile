# vars
CC = gcc
LIBUVCDIR = /usr/include
LIBPYDIR = /usr/include/python3.6
CFLAGS = -luvc -lpthread -lpython3.6m `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0` -I$(LIBPYDIR)
OBJECTS = camera.o buffer.o gui.o output.o dispatcher.o main.o
CFILE = camera.c buffer.c gui.c output.c dispatcher.c main.c
TARGET = main.out
all: $(TARGET)

camera.o: camera.c
	$(CC) camera.c -o camera.o -c -luvc -lpthread

buffer.o: buffer.c
	$(CC) buffer.c -o buffer.o -c -lpthread

output.o: output.c
	$(CC) output.c -o output.o -c -lpython3.6m -I$(LIBPYDIR)

dispatcher.o: dispatcher.c
	$(CC) dispatcher.c -o dispatcher.o -c -lpthread -lpython3.6m -I$(LIBPYDIR)

gui.o: gui.c
	$(CC) -o gui.o gui.c -c `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0` -lpthread

main.o: main.c 
	$(CC) -o main.o main.c -c $(CFLAGS)

main.out: $(OBJECTS)
	$(CC) -o main.out $(OBJECTS) $(CFLAGS) 

clean:
	rm -f $(OBJECTS) $(TARGET)

run:
	./$(TARGET)

