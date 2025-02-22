TARGET = custom-touchpad-gesture
C_SOURCES = custom-touchpad-gesture.c main.c
H_HEADERS = custom-touchpad-gesture.h

CC = gcc
CFLAGS = -g -Wall

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(C_SOURCES) $(H_HEADERS)
	$(CC) $(CFLAGS) $(C_SOURCES) -o $(TARGET)

clean:
	-rm -Rf $(TARGET)

