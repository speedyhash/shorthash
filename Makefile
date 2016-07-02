# minimalist makefile
.SUFFIXES:
#
.SUFFIXES: .cpp .o .c .h

SHARED_FLAGS = -fPIC -std=c99 -mssse3 -mpclmul -Wall -Wextra -Wshadow

ifeq ($(DEBUG),1)
CFLAGS = $(SHARED_FLAGS) -ggdb -fsanitize=undefined -fno-omit-frame-pointer -fsanitize=address
else
CFLAGS = $(SHARED_FLAGS) -O3
endif # debug

all: benchmark $(OBJECTS)

HEADERS=include/clhash.h include/tabulated.h include/util.h

benchmark: ./benchmarks/benchmark.c $(HEADERS)
	$(CC) $(CFLAGS) -o benchmark ./benchmarks/benchmark.c -Iinclude

clean:
	rm -f  *.o benchmark
