## short hash

How fast can you hash a machine word?

The C++ standard includes functions that return a ``size_t`` value which is
a 64-bit value on 64-bit systems. We follow this convention and 
compute the hash value of 64-bit words.

### Requirements

- Recent Intel processor (e.g., Skylake)
- Linux like system

## Usage

```bash
make
./benchmark
```

## Todo 

- Test with hash table 




