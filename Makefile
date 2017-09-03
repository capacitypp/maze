OPENCVFLAGS=$(shell pkg-config --cflags opencv)
OPENCVLIBS=$(shell pkg-config --libs opencv)
CXX=ccache g++ -Wall -Wno-int-in-bool-context -O2 -DEIGEN_NO_DEBUG

.cpp.o:
	$(CXX) $(OPENCVFLAGS) -fPIC -c $<

all: maze

maze: maze.o
	$(CXX) -o $@ $^ $(OPENCVLIBS)

clean:
	rm -f *.o
	rm -f maze

