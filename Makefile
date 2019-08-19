PROGS=text1

CXXFLAGS=-std=c++11 -Wall -Wextra -pedantic

all: $(PROGS)

clean:
	rm -f $(PROGS) *.o

text1: text1.o
	g++ $(LDFLAGS) -o $@ $<
text1.o: test1.cpp
	g++ $(CXXFLAGS) -c -o $@ $<

