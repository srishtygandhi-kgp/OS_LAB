CC=g++
FLAGS=-Wall

build:
	make sns
.PHONY: build

main: sns.cpp macros.h musae_git_edges.csv
	$(CC) $(FLAGS) sns.cpp -o sns

clean:
	rm sns 
.PHONY: clean
