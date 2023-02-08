CC=g++
FLAGS=-Wall

wish: main.o utils.o curse.o
	$(CC) $(FLAGS) main.o utils.o curse.o -lncurses -o wish

main.o: main.cpp
	$(CC) $(FLAGS) -c main.cpp -o main.o

utils.o: utils.cpp utils.h
	$(CC) $(FLAGS) -c utils.cpp -o utils.o

curse.o: curse.cpp curse.h
	$(CC) $(FLAGS) -c curse.cpp -o curse.o

clean:
	rm *.o wish