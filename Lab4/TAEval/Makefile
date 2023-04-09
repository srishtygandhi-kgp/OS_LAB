CC=g++
FLAGS=-Wall

build:
	make sns
.PHONY: build

main: sns.cpp macros.h input.csv
	$(CC) $(FLAGS) -g sns.cpp -o sns

test: sns
	valgrind --tool=memcheck --leak-check=yes ./sns

clean:
	rm sns *.log
.PHONY: clean
