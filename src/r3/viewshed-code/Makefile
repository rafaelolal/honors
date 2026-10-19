CC = g++
CFLAGS = -g3 -Wall -Wstrict-prototypes
LDFLAGS = -lm

OBJS = grid.o utils.o vis.o vismain.o

vis: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o vis

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

test: vis
	./vis data/flat.asc out.flat.asc 5 5 1
	./vis data/wall.asc out.wall.asc 5 0 1

clean:
	$(RM) *.o vis out.*.asc
