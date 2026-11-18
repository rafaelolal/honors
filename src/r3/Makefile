PROGRAMS = vis

BASE  = -m64 -Wall -Winline
# fastest; asserts off
FAST  = $(BASE) -O3 -march=native -DNDEBUG
# fast + symbols + intact call stacks for perf
PERF  = $(FAST) -g -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer
# quick build, asserts on, debuggable
DEBUG = $(BASE) -O0 -g

LIBS = -lm

SRCS = vismain.cpp vis.cpp visgrid.cpp bmp.cpp map.cpp grid.cpp
HDRS = vis.h visgrid.h bmp.h map.h grid.h

# always rebuild so flags never mix
.PHONY: default perf debug clean

default:
	g++ $(FAST) -o vis $(SRCS) $(LIBS)

perf:
	g++ $(PERF) -o vis $(SRCS) $(LIBS)

debug:
	g++ $(DEBUG) -o vis $(SRCS) $(LIBS)

clean:
	rm -f $(PROGRAMS)
