CC = gcc
CFLAGS = -Wall
EXEC = tfs_create
HEADERS = $(wildcard *.h)
OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) -o $@ $^

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJECTS) $(EXEC)