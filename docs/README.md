# Memory Allocator
## Overview

Simple memory allocator that allocates memory using the `sbrk` system call and a simple first-fit algorithm (I will probably expand this to use mmap and other algorithms too).

## Functionalities

* Provides `void alloc(size_t bytes)` and `void free(void* addr)` functions, which work similarly to the classic `malloc()` and `free()`.
* Allows checking information about allocated memory at any time while the program is running.
* Collects the following statistics:
  + Total allocated memory in bytes.
  + Number of `alloc()` calls.
  + Number of `sbrk()` calls.
  + Mean bytes allocated.
  + Peak memory usage.
  + Number of broken memory chunks (chunks with their metadata overwritten).
  + Number of unfreed blocks (file, line).
  
  These statistics are printed on the terminal upon exit.
* Operates in a multithreaded environment.
* Has been heavily tested with analyzers.

## Requirements
* Linux distribution
* Any C compiler
* make
* binutils

Executing `scripts/install_env.sh` script will detect your Linux distribution and install required software. You can run `make install_env` if you have at least make installed on your computer.

## Installation, compilation and usage

To build and install the library to a desired directory with desired compier enter ``` make install P=/path/to/allocator CC=<c_compiler>```. Running `make install` by itself will execute the installation script, which will prompt you to enter the desired path where the allocator library should be installed and will use gcc as default compiler. Program using the allocator should be compiled with `-I/path/to/allocator/include -L/path/to/allocator/lib -lallocator` flags. You also need to add `#include"allocator.h"` line to your C program using the allocator.

### Example
- I downloaded this repository to /home/$user/Documents/work/allocator/
- Ran `make isntall_env`
- Ran `make install P=/home/$USER/Documents/work/allocator/example CC=gcc` (it doesn't have to be installed in the same directory as the repository)
- Cretaed `example.c` in `example` directory and wrote a simple program using the allocator
- Compiled it with `gcc example/example.c -Iexample/include -Lexample/lib -o example_out`
