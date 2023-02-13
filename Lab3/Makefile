CC=gcc
FLAGS=-Wall

build:
	make main producer
.PHONY: build

main: main.c facebook_combined.txt
	$(CC) $(FLAGS) main.c -o main

producer: producer.c facebook_combined.txt
	$(CC) $(FLAGS) producer.c -o producer

clean:
	rm main producer
.PHONY: clean