CC=gcc
RM=rm
my_tar: main.c
	$(CC) -o my_tar -Wall -Wextra main.c my_functions.c my_functions.h

clean:
	$(RM) -f my_tar