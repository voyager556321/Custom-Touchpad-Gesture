/*
 * Custom-Touchpad-Gesture
 * Copyright (C) 2025 Vova Laskarzhevskyi
 *
 * Licensed under GNU GPLv3 or a commercial license.
 * Contact vovls5433@gmail.com for commercial licensing options.
 */

#include "custom-touchpad-gesture.h"

int main(void) {
    int uinput_fd = setup_uinput_device();
    if (uinput_fd < 0) {
        perror("Failed to setup uinput device");
        return 1;
    }

    FILE *fp = popen("libinput list-devices | awk '/Touchpad/{getline; print $2}'", "r");
    if (!fp) {
        perror("popen failed");
        cleanup(uinput_fd, -1);
        return 1;
    }

    char event[32];
    if (!fgets(event, sizeof(event), fp)) {
        perror("Failed to get event device");
        pclose(fp);  // Replace fclose with pclose here
        cleanup(uinput_fd, -1);
        return 1;
    }
    event[strcspn(event, "\n")] = 0;  // Remove newline character

    const char *device = event;
    int fd = open(device, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open input device");
        pclose(fp);  // Replace fclose with pclose here
        cleanup(uinput_fd, -1);
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
    pclose(fp);  // Replace fclose with pclose here
    return 0;
}
