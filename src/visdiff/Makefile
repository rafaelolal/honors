PROGRAMS = visdiff

DEFAULT: $(PROGRAMS)

CFLAGS = -m64 -Winline -DNDEBUG -O3 -Wall

####DEBUG MODE
# CFLAGS = -m64 -Winline -g -O3 -Wall

CC = g++ $(CFLAGS)

LIBS = -lm

SRCS = visdiff.cpp diff.cpp hillshade.cpp bmp.cpp grid.cpp
HDRS = diff.h hillshade.h bmp.h grid.h

visdiff: $(SRCS) $(HDRS)
	$(CC) -o $@ $(SRCS) $(LIBS)

clean:
	rm -f $(PROGRAMS)
