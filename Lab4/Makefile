CC=g++
FLAGS=-Wall

build:
	make main
.PHONY: build

main: main.cpp macros.h musae_git_edges.csv
	$(CC) $(FLAGS) main.cpp -o main

clean:
	rm main 
.PHONY: clean
