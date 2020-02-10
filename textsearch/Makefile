PROGS=test1 test2 test3 test4

CXXFLAGS=-std=c++11 -Wall -Wextra -pedantic

all: $(PROGS)

clean:
	rm -f $(PROGS) *.o

test1: test1.o textsrc.o
	g++ $(LDFLAGS) -o $@ $^
test1.o: test1.cpp
	g++ $(CXXFLAGS) -c -o $@ $^

test2: test2.o textsrc.o
	g++ $(LDFLAGS) -o $@ $^
test2.o: test2.cpp
	g++ $(CXXFLAGS) -c -o $@ $^

test3: test3.o textsrc.o
	g++ $(LDFLAGS) -o $@ $^
test3.o: test3.cpp
	g++ $(CXXFLAGS) -c -o $@ $^

test4: test4.o textsrc.o
	g++ $(LDFLAGS) -o $@ $^
test4.o: test4.cpp
	g++ $(CXXFLAGS) -c -o $@ $^

textsrc.o: textsrc.cpp
	g++ $(CXXFLAGS) -c -o $@ $^

