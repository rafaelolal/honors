PLATFORM = $(shell uname)

PROGRAMS = vis

DEFAULT: $(PROGRAMS)

#### EXPERIMENTS MODE
#### using -DNDEBUG turns asserts off. Use it only for timing.
CFLAGS = -m64 -Winline -DNDEBUG -O3 -Wall

####DEBUG MODE
# CFLAGS = -m64 -Winline -g -O3 -Wall

CC = g++ $(CFLAGS)

LIBS = -lm

SRCS = vismain.cpp vis.cpp visgrid.cpp bmp.cpp map.cpp grid.cpp pixel_buffer.cpp
HDRS = vis.h visgrid.h bmp.h map.h grid.h pixel_buffer.h stb_image_write.h

vis: $(SRCS) $(HDRS)
	$(CC) -o $@ $(SRCS) $(LIBS)

clean:
	rm -f $(PROGRAMS)
