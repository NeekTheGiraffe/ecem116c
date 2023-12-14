# ECE M116C

Computer assignments for [ECE M116C in Fall '23](https://ssysarch.github.io/ECE_M116C-CS_M151B/F23/index.html)
at UCLA (Computer Systems Architecture), with [Prof. Nader Sehatbakhsh](https://ssysarch.ee.ucla.edu/nader/).

- `project1`: A RISC-V interpreter supporting a limited instruction set.
- `project2`: A memory controller simulation with a 3-level cache hierarchy.

## Usage

I recommend using Unix to run these projects.

First clone the repository:
```
$ git clone https://github.com/NeekTheGiraffe/ecem116c.git
$ cd ecem116c
```

### Project 1

The list of supported instructions is:
- `ADD`
- `SUB`
- `XOR`
- `SRA`
- `ADDI`
- `LW`
- `SW`
- `BLT`
- `JALR`

More information about these instructions can be found in the
[RISC-V Specification](https://riscv.org/wp-content/uploads/2017/05/riscv-spec-v2.2.pdf),
and implementation details can be found in `project1/report.pdf`.

#### Running

```
ecem116c$ cd project1
project1$ make
project1$ ./cpusim [MACHINE CODE FILE]
```

The program will print out the values of `a0` (`x10`) and `a1` (`x11`) after
running all the instructions.

#### Generating machine code

You can convert any RISC-V assembly file (using the limited set of instructions)
to a form of "machine code" that the interpreter will understand using the
`generate-test.py` script:

```
project1$ python3 generate-test.py my-assembly.s my-machine-code.txt
```

Keep in mind this script depends on the `riscv-assembler` Python package.

### Project 2

This program simulates a memory hierarchy with the following features:
- Byte-addressable 4 KiB RAM
- A direct-mapped L1 cache with 1 block per line and 16 lines
- A fully-associative victim cache with 4 lines
- An 8-way set-associative L2 cache with 16 sets
- Each cache block is 4 bytes
- LRU replacement policy
- Exclusive, write-no-allocate, write-through caches

#### Running

```
ecem116c$ cd project2
project2$ make
project2$ ./memory_driver [TRACE FILE]
```

The program will print out the L1 miss rate, L2 miss rate, and average access
time of the memory hierarchy given the sequence of accesses.

#### Creating traces

Traces follow the following schema:
```
MemR, MemW, adr, data
```
- `MemR`: 1 for loads, 0 for stores
- `MemW`: 0 for loads, 1 for stores
- `adr`: The byte of memory to be accessed
- `data`: The data to be written (0 for loads)

## Running tests

In either project directory, run `make check`. This will run a Python script
checking each of the test cases in the `trace` directory.
