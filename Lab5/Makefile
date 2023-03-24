CC=g++
FLAGS=-Wall

build:
	make main guest cleaner
.PHONY: build

main: main.h main.cpp
	$(CC) $(FLAGS) main.cpp -g -lpthread -lrt -o main

guest: guest.cpp main.h
	$(CC) $(FLAGS) guest.cpp -o guest

cleaner: cleaningStaff.cpp main.h
	$(CC) $(FLAGS) cleaningStaff.cpp -o cleaner

clean:
	rm main cleaner guest *.o
.PHONY: clean
