PROGS=test1 test2 test3 test4 test5 text-dictionary

CXXFLAGS=-std=c++11 -Wall -Wextra -pedantic

JSON11=json11.o

all: $(PROGS) $(JSON11)

clean:
	rm -f $(PROGS) *.o

json11.o: json11.cpp
	g++ $(CXXFLAGS) -c -o $@ $^

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

test5: test5.o textsrc.o json11.o
	g++ $(LDFLAGS) -o $@ $^
test5.o: test5.cpp
	g++ $(CXXFLAGS) -c -o $@ $^

text-dictionary: text-dictionary.o textsrc.o json11.o
	g++ $(LDFLAGS) -o $@ $^
text-dictionary.o: text-dictionary.cpp
	g++ $(CXXFLAGS) -c -o $@ $^

textsrc.o: textsrc.cpp
	g++ $(CXXFLAGS) -c -o $@ $^

