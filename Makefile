CXX := g++
CXXFLAGS := -std=c++11 -Wall
LDFLAGS := -lncurses
.PHONY: clean

binary := tetris

$(binary): main.o
	$(CXX) $(LDFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o $(binary)
