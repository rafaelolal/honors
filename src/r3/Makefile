PLATFORM = $(shell uname)

PROGRAMS = vis

DEFAULT: $(PROGRAMS)

#### EXPERIMENTS MODE 
#### using -DNDEBUG turns asserts off. Use it only for timing. 
##CFLAGS = -m64 -Winline -DNDEBUG -O3 -Wall 

#DEBUG MODE 
CFLAGS = -m64 -Winline -g -O3 -Wall

CC = g++ $(CFLAGS)

LIBS = -lm

vis:  vismain.o vis.o map.o  grid.o pixel_buffer.o
	$(CC) -o $@ vismain.o vis.o map.o  grid.o pixel_buffer.o $(LIBS)

vismain.o: vismain.cpp vis.h map.h  grid.h  pixel_buffer.h stb_image_write.h
	$(CC) $(CFLAGS) -o $@ -c vismain.cpp 

vis.o: vis.cpp vis.h grid.h 
	$(CC) $(CFLAGS) -o $@ -c vis.cpp

map.o: map.cpp map.h grid.h pixel_buffer.h stb_image_write.h
	$(CC) $(CFLAGS) -o $@ -c map.cpp

grid.o: grid.cpp grid.h
	$(CC) $(CFLAGS) -o $@ -c grid.cpp

pixel_buffer.o: pixel_buffer.cpp pixel_buffer.h stb_image_write.h
	$(CC) $(CFLAGS) -o $@ -c pixel_buffer.cpp

clean: 
	rm -f *.o $(PROGRAMS)
