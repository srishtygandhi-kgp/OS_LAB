## Available commands

- `make` - Builds all the required executables

- `make kill` - kill all the running producer and consumer processes

- `make clean` - remove all temporarily created files

<br/>

### To run consumer process (from shell)

Run this: `./consumer <consumerID> <optimize>`

- `<consumerID>` - Specify a ID for the consumer according to which it will choose nodes for it.

- `<optimize>` - Pass 1 to optimize the run and 0 for a trivial run

<br/>

### Debugging

A file `debug_producer.txt` is created which contains the new nodes and edges added by the producer process.