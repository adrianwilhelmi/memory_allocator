#!/bin/bash

if [ -z "$P" ]; then
	echo "please enter the installation path:"
	read installation_path
else
	installation_path=$P
fi

header_path="$installation_path/include"
lib_path="$installation_path/lib"

mkdir -p "$header_path" "$lib_path"

echo "copying header file"
sudo cp include/allocator.h $header_path

CC=${CC:-gcc}
echo "compiling and creating lib..."
$CC -Iinclude -c -o allocator_stats.o src/allocator_stats.c
$CC -Iinclude -c -o mem_block.o src/mem_block.c
$CC -Iinclude -c -o allocator.o src/allocator.c

ar rcs liballocator.a allocator.o allocator_stats.o mem_block.o

sudo cp liballocator.a "$lib_path"

rm allocator.o allocator_stats.o mem_block.o
rm liballocator.a

echo "lib installed successfully to $installation_path"
