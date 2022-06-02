outname := out

CXX := g++
CXXFLAGS := -Wall -Wextra -Wpedantic -Werror -std=c++17 -g

srcfiles := $(shell find . -maxdepth 1 -name "*.cpp")
objects  := $(patsubst %.cpp, %.o, $(srcfiles))

all: $(outname)

$(outname): $(objects)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(outname) $(objects) $(LDLIBS)

depend: .depend

.depend: $(srcfiles)
	rm -f ./.depend
	$(CXX) $(CXXFLAGS) -MM $^>>./.depend;

clean:
	rm -f $(objects)

clear: clean
	rm -f *~ .depend $(outname)

include .depend
