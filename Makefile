CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = 	SystemResources
SOURCES = main.c display.c cpu.c memory.c disk.c network.c
OBJECTS = $(SOURCES:.c=.o)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.c monitor.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)

.PHONY: clean