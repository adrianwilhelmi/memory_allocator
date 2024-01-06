CFLAGS=-Wall -Wextra -pedantic -ggdb
LFLAGS=-Iinclude

all: allocator

allocator:
	$(CC) $(FLAGS) $(LFLAGS) -o allocator src/allocator.c src/main.c
