# minimalist makefile
.SUFFIXES:
#
.SUFFIXES: .cpp .o .c .h

SHARED_FLAGS = -march=native -Wall -Wextra -Wshadow -Wno-unused-function \
    -Wno-missing-braces

ifeq ($(DEBUG),1)
FLAGS = $(SHARED_FLAGS) -ggdb -fsanitize=undefined -fno-omit-frame-pointer \
    -fsanitize=address -O0 -fno-inline-functions
else
FLAGS = $(SHARED_FLAGS) -O3 -fomit-frame-pointer
endif # debug

CFLAGS = $(FLAGS) -std=c99
CXXFLAGS =  $(FLAGS) -std=c++11

all: benchmark.exe param_htbenchmark.exe htbenchmark.exe lptimed.exe	\
    bucketbenchmark.exe linearprobebenchmark.exe cw-trick-test.exe	\
    collision-test.exe worst.exe fig2a.exe $(OBJECTS)

HEADERS = include/clhash.h include/tabulated.h include/util.h		\
    include/multiply-shift.h include/cw-trick.h benchmarks/hashpack.h	\
    benchmarks/hashmap.h benchmarks/timers.hpp benchmarks/buckets.hpp	\
    include/linear.h include/identity.h benchmarks/simple-hashmap.h	\
    benchmarks/sep-chaining.h benchmarks/rehashset.h

siphash24.o: ./include/siphash24.c
	$(CC) $(CFLAGS) -o $@ $< -c

benchmark.exe: ./benchmarks/benchmark.cpp siphash24.o $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $< -Iinclude siphash24.o

param_htbenchmark.exe: ./benchmarks/param_htbenchmark.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $< -Iinclude

htbenchmark.exe: ./benchmarks/htbenchmark.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $< -Iinclude

lptimed.exe: ./benchmarks/lptimed.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $< -Iinclude

bucketbenchmark.exe: ./benchmarks/bucketbenchmark.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $< -Iinclude

linearprobebenchmark.exe: ./benchmarks/linearprobebenchmark.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $< -Iinclude

cw-trick-test.exe: ./test/cw-trick-test.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $< -Iinclude

collision-test.exe: ./test/collision-test.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $< -Iinclude

worst.exe: ./benchmarks/worst.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $< -Iinclude

fig2a.exe: ./benchmarks/fig2a.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $< -Iinclude

short-width.exe: ./test/short-width.cc
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f *.o *.exe
