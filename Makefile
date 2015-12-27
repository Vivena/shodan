CC = gcc
CFLAGS = -Wall
EXEC = tfs_create tfs_partition tfs_analyse
HEADERS = $(wildcard *.h)
OBJECTS = read_block.o write_block.o start_disk.o sync_disk.o tfs_manipulation.o util.o
MAIN_OBJECTS = $(EXEC:=.o)

all: $(EXEC)

tfs_create: tfs_create.o $(OBJECTS)
	$(CC) -o $@ $^

tfs_partition: tfs_partition.o $(OBJECTS)
	$(CC) -o $@ $^

tfs_analyse: tfs_analyse.o $(OBJECTS)
	$(CC) -o $@ $^

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJECTS) $(EXEC) $(MAIN_OBJECTS)