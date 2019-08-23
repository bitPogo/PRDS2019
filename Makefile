CC=gcc
CFLAGS=--std=c99 -Wall -Werror -Wpedantic -DNDEBUG -g
OBJ=loesung.c
NAME=loesung

default:
	$(CC) -o $(NAME) $(OBJ) $(CFLAGS)
clean:
	rm -f $(NAME)
