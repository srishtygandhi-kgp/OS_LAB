CC=g++
FLAGS=-Wall

build:
	make main
.PHONY: build

main: main.h main.o guest.o cleaner.o
	$(CC) $(FLAGS) guest.o cleaner.o main.o -lpthread -lrt -o main

main.o: main.cpp main.h
	$(CC) $(FLAGS) -c main.cpp -o main.o

guest.o: guest.cpp main.h
	$(CC) $(FLAGS) -c guest.cpp -o guest.o

cleaner.o: cleaningStaff.cpp main.h
	$(CC) $(FLAGS) -c cleaningStaff.cpp -o cleaner.o

clean:
	rm main *.o
.PHONY: clean
