## short hash

How fast can you hash a 64-bit or 32-bit machine word?

### Requirements

- x86-64 processor supporting the SSSE3 and CLMUL instruction sets
  (most since 2011)

- Linux like system

## Usage

- clang appears remarkably faster (``export CC=clang; export CXX=clang++``)

```bash
make
./benchmark.exe
```


