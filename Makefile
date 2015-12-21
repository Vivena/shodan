CC = gcc
CFLAGS = -Wall
EXEC = tfs_create tfs_partition
HEADERS = $(wildcard *.h)
OBJECTS = read_block.o write_block.o start_disk.o util.o
MAIN_OBJECTS = tfs_create.o tfs_partition.o

all: $(EXEC)

tfs_create: tfs_create.o $(OBJECTS)
	$(CC) -o $@ $^
tfs_partition: tfs_partition.o $(OBJECTS)
	$(CC) -o $@ $^

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJECTS) $(EXEC) $(MAIN_OBJECTS)