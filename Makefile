# minimalist makefile
.SUFFIXES:
#
.SUFFIXES: .cpp .o .c .h
ifeq ($(DEBUG),1)
CFLAGS = -fPIC  -std=c99 -ggdb -march=native -Wall -Wextra -Wshadow -fsanitize=undefined  -fno-omit-frame-pointer -fsanitize=address
else
CFLAGS = -fPIC -std=c99 -O3  -march=native -Wall -Wextra -Wshadow
endif # debug
all: benchmark $(OBJECTS)
HEADERS=include/clhash.h  include/tabulated.h include/util.h

benchmark: ./benchmarks/benchmark.c $(HEADERS)
	$(CC) $(CFLAGS) -o benchmark ./benchmarks/benchmark.c -Iinclude
clean:
	rm -f  *.o benchmark
