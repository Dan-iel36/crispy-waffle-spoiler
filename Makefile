.PHONY : all

all:  install compile build


compile:
	@gcc -c main.c -o main.o


build: compile
	@gcc main.o -o spoiler

install: build
	@chmod +x spoiler


clean:
	@rm -rf ./"*.o"
	@rm -rf spoiler