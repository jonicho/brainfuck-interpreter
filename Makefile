CC = gcc
CFLAGS = -Wall -Wextra -pedantic

brainfuck-interpreter: main.c
	$(CC) $(CFLAGS) -o brainfuck-interpreter main.c

.PHONY: clean
clean:
	rm -f brainfuck-interpreter
