PROGRAMS = vis

# HUGEPAGES: big grids on 2 MB pages
BASE  = -m64 -Wall -Winline -DHUGEPAGES
# fastest; asserts off
FAST  = $(BASE) -O3 -march=native -DNDEBUG
# fast + symbols + intact call stacks for perf
PROFILING = $(FAST) -g -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer
# quick build, asserts on, debuggable
DEBUG = $(BASE) -O0 -g

LIBS = -lm

SRCS = vismain.cpp vis.cpp visgrid.cpp bmp.cpp map.cpp grid.cpp
HDRS = vis.h visgrid.h bmp.h map.h grid.h

# always rebuild so flags never mix
.PHONY: default profiling debug clean

# vis prints these flags with its results
default:
	g++ $(FAST) -DBUILD_FLAGS='"$(FAST)"' -o vis $(SRCS) $(LIBS)

profiling:
	g++ $(PROFILING) -DBUILD_FLAGS='"$(PROFILING)"' -o vis $(SRCS) $(LIBS)

debug:
	g++ $(DEBUG) -DBUILD_FLAGS='"$(DEBUG)"' -o vis $(SRCS) $(LIBS)

clean:
	rm -f $(PROGRAMS)
