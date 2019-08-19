PROGS=test1

CXXFLAGS=-std=c++11 -Wall -Wextra -pedantic

all: $(PROGS)

clean:
	rm -f $(PROGS) *.o

test1: test1.o
	g++ $(LDFLAGS) -o $@ $<
test1.o: test1.cpp
	g++ $(CXXFLAGS) -c -o $@ $<

