PLATFORM = $(shell uname)

PROGRAMS = vis

DEFAULT: $(PROGRAMS)

#### EXPERIMENTS MODE 
#### using -DNDEBUG turns asserts off. Use it only for timing. 
##CFLAGS = -m64 -Winline -DNDEBUG -O3 -Wall 

#DEBUG MODE 
CFLAGS = -m64 -Winline -g -O3 -Wall

CC = gcc $(CFLAGS)

LIBS = -lm

vis:  vismain.o viewshed.o map.o  grid.o pixel_buffer.o
	$(CC) -o $@ vismain.o viewshed.o map.o  grid.o pixel_buffer.o $(LIBS)

vismain.o: vismain.c viewshed.h map.h  grid.h  pixel_buffer.h stb_image_write.h
	$(CC) $(CFLAGS) -o $@ -c vismain.c 

viewshed.o: viewshed.c viewshed.h grid.h 
	$(CC) $(CFLAGS) -o $@ -c viewshed.c

map.o: map.c map.h grid.h pixel_buffer.h stb_image_write.h
	$(CC) $(CFLAGS) -o $@ -c map.c

grid.o: grid.c grid.h
	$(CC) $(CFLAGS) -o $@ -c grid.c

pixel_buffer.o: pixel_buffer.c pixel_buffer.h stb_image_write.h
	$(CC) $(CFLAGS) -o $@ -c pixel_buffer.c

clean: 
	rm -f *.o $(PROGRAMS)
