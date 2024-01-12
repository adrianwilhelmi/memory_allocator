#!/bin/bash

echo "please enter the installation path"
read installation_path

header_path="$installation_path/include"
lib_path="$installation_path/lib"

mkdir -p "$header_path" "$lib_path"

echo "copying header file"
sudo cp include/allocator.h $header_path

echo "compiling and creating lib..."
gcc -Iinclude -c -o allocator.o src/allocator.c
ar rcs liballocator.a allocator.o
sudo cp liballocator.a "$lib_path"
rm allocator.o
rm liballocator.a

echo "lib installed successfully to $installation_path"
