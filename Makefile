CC = gcc
CFLAGS = -Wall
EXEC = tfs_create tfs_partition tfs_analyse tfs_format tfs_mkdir
HEADERS = $(wildcard *.h)
OBJECTS = read_block.o write_block.o start_disk.o sync_disk.o tfs_manipulation.o tfs_open.o tfs_close.o util.o
MAIN_OBJECTS = $(EXEC:=.o)

all: $(EXEC)

tfs_create: tfs_create.o $(OBJECTS)
	$(CC) -o $@ $^

tfs_partition: tfs_partition.o $(OBJECTS)
	$(CC) -o $@ $^

tfs_analyse: tfs_analyse.o $(OBJECTS)
	$(CC) -o $@ $^

tfs_format: tfs_format.o $(OBJECTS)
	$(CC) -o $@ $^

tfs_mkdir: tfs_mkdir.o $(OBJECTS)
	$(CC) -o $@ $^

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJECTS) $(EXEC) $(MAIN_OBJECTS)