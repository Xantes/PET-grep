CC = gcc
CFLAGS = -Wall -Werror -Wextra
OS := $(shell uname -s)

ifeq ($(OS), Darwin)
	CFLAGS += -D OS_DARWIN_
endif


all: build

build: s21_grep

s21_grep:
	$(CC) $(CFLAGS) src/s21_grep.c -o s21_grep

rebuild: clean build

clean:
	rm -rf *.o
	rm -rf s21_grep
