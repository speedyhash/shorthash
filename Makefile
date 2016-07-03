# minimalist makefile
.SUFFIXES:
#
.SUFFIXES: .cpp .o .c .h

SHARED_FLAGS = -fPIC -march=native -Wall -Wextra -Wshadow

ifeq ($(DEBUG),1)
FLAGS = $(SHARED_FLAGS) -ggdb -fsanitize=undefined -fno-omit-frame-pointer -fsanitize=address
else
FLAGS = $(SHARED_FLAGS) -O3
endif # debug

CFLAGS = $(FLAGS) -std=c99
CXXFLAGS = $(FLAGS) -std=c++11

all: benchmark $(OBJECTS)

HEADERS=include/clhash.h include/tabulated.h include/util.h

benchmark: ./benchmarks/benchmark.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o benchmark ./benchmarks/benchmark.cpp -Iinclude

clean:
	rm -f  *.o benchmark
