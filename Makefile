.PHONY:clean all

SOURCE = $(wildcard *c)
TARGET = $(patsubst %.c, %, $(SOURCE))

CC=gcc
CFLAGS= -Wall -g

all: $(TARGET)

%:%.c
	$(CC) $(CFLAGS) $^ -o $@


clean:
	rm -f $(TARGET)
