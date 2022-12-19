CC := gcc
CFLAGS := -Wall
CXX := g++
CXXFLAGS := -std=c++11 -Wall
LDFLAGS := -lncurses
.PHONY: all c cpp clean

bin := tetris
cbin := $(bin)_c
cppbin := $(bin)_cpp

all: cpp c

cpp: $(cppbin)
$(cppbin): $(cppbin).o
	$(CXX) $(LDFLAGS) $^ -o $@
$(cppbin).o: $(bin).cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

c: $(cbin)
$(cbin): $(cbin).o
	$(CC) $(LDFLAGS) $^ -o $@
$(cbin).o: $(bin).c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(cbin) $(cppbin)
