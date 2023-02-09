CC=g++
FLAGS=-Wall

wish: main.o utils.o
	$(CC) $(FLAGS) main.o utils.o -lreadline -o wish

main.o: main.cpp
	$(CC) $(FLAGS) -c main.cpp -o main.o

utils.o: utils.cpp utils.h
	$(CC) $(FLAGS) -c utils.cpp -o utils.o

clean:
	rm *.o wish