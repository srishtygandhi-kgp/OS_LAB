CC=gcc
FLAGS=-Wall

build:
	make main producer consumer
.PHONY: build

main: main.c macros.h facebook_combined.txt
	$(CC) $(FLAGS) main.c -o main

producer: producer.c macros.h facebook_combined.txt
	$(CC) $(FLAGS) producer.c -o producer

consumer: consumer.o queue.o
	$(CC) $(FLAGS) consumer.o queue.o -lm -o consumer

consumer.o: consumer.c macros.h facebook_combined.txt
	$(CC) $(FLAGS) -c consumer.c -o consumer.o

queue.o: queue.c queue.h macros.h
	$(CC) $(FLAGS) -c queue.c -o queue.o

clean:
	rm main producer consumer consumer*.txt consumer.o queue.o debug_producer.txt
.PHONY: clean

kill:
	killall -9 consumer producer
.PHONY: kill