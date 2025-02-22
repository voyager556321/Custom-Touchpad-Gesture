/*
 * Custom-Touchpad-Gesture
 * Copyright (C) 2025 Vova Laskarzhevskyi
 *
 * Licensed under GNU GPLv3 or a commercial license.
 * Contact vovls5433@gmail.com for commercial licensing options.
 */

#pragma once

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/uinput.h>
#include <linux/input.h>
#include <stdlib.h>
#include <string.h>

#define SLOT_TO_TRACK 2 // Tracking the third finger (slot 2)
#define THRESHOLD 50   // Threshold for gesture recognition
#define MAX_SLOTS 3

extern int slots[MAX_SLOTS][2];
extern int base_point[2];
extern int active_slot;

int setup_uinput_device(void);
void send_key_event(int fd, int keycode, int value);
void simulate_keypress(int fd, int modifier_keycode, int keycode);
void handle_abs_event(struct input_event *ev);
void process_gesture(int uinput_fd);
void cleanup(int fd, int uinput_fd);



