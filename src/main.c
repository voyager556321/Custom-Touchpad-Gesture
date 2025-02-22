/*
 * Custom-Touchpad-Gesture
 * Copyright (C) 2025 Vova Laskarzhevskyi
 *
 * Licensed under GNU GPLv3 or a commercial license.
 * Contact vovls5433@gmail.com for commercial licensing options.
 */

#include "custom-touchpad-gesture.h"

int main() {
    int uinput_fd = setup_uinput_device();
    if (uinput_fd < 0) return 1;

    const char *device = "/dev/input/event10"; // set correct eventX for your Touchpad
    int fd = open(device, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open input device");
        cleanup(fd, uinput_fd);
        return 1;
    }

    struct input_event ev;
    printf("Reading events... (Press CTRL+C to exit)\n");

    while (read(fd, &ev, sizeof(ev)) > 0) {
        if (ev.type == EV_ABS) {
            handle_abs_event(&ev);
        }
        process_gesture(uinput_fd);
    }

    cleanup(fd, uinput_fd);
    return 0;
}
