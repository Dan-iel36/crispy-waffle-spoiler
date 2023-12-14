.PHONY : all

all:  install compile build


compile:
	@gcc -c main.c -o main.o -pthread


build: compile
	@gcc main.o -o spoiler -pthread

install: build
	@chmod +x spoiler


clean:
	@rm -rf ./"*.o"
	@rm -rf spoiler