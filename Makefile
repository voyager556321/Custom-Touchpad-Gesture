#
# Makefile for Custom-Touchpad-Gesture
# Copyright (C) 2025 Vova Laskarzhevskyi
#
# Licensed under GNU GPLv3 or a commercial license.
# Contact vovls5433@gmail.com for commercial licensing options.
#

TARGET = custom-touchpad-gesture
SRC_DIR = src
INCLUDE_DIR = include

C_SOURCES = $(SRC_DIR)/custom-touchpad-gesture.c $(SRC_DIR)/main.c
H_HEADERS = $(INCLUDE_DIR)/custom-touchpad-gesture.h

CC = gcc
CFLAGS = -g -Wall -I$(INCLUDE_DIR)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(C_SOURCES) $(H_HEADERS)
	$(CC) $(CFLAGS) $(C_SOURCES) -o $(TARGET)

clean:
	-rm -Rf $(TARGET)
