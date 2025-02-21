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

// Global variables for coordinates
int slots[MAX_SLOTS][2] = {{-1, -1}, {-1, -1}, {-1, -1}};
int base_point[2] = {-1, -1};
int active_slot = 0;

// Initialize uinput device
int setup_uinput_device() {
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("Failed to open /dev/uinput");
        return -1;
    }

    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_KEYBIT, KEY_LEFTCTRL);
    ioctl(fd, UI_SET_KEYBIT, KEY_PAGEDOWN);
    ioctl(fd, UI_SET_KEYBIT, KEY_PAGEUP);

    struct uinput_setup usetup;
    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234;
    usetup.id.product = 0x5678;
    strcpy(usetup.name, "Virtual Keyboard");

    if (ioctl(fd, UI_DEV_SETUP, &usetup) < 0 || ioctl(fd, UI_DEV_CREATE) < 0) {
        perror("Failed to setup uinput device");
        close(fd);
        return -1;
    }

    return fd;
}

// Send key events
void send_key_event(int fd, int keycode, int value) {
    struct input_event ev = { .type = EV_KEY, .code = keycode, .value = value };
    write(fd, &ev, sizeof(ev));

    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(fd, &ev, sizeof(ev));
}

// Simulate key press
void simulate_keypress(int fd, int modifier_keycode, int keycode) {
    send_key_event(fd, modifier_keycode, 1);
    send_key_event(fd, keycode, 1);
    send_key_event(fd, keycode, 0);
    send_key_event(fd, modifier_keycode, 0);
}

// Handle ABS_MT events
void handle_abs_event(struct input_event *ev) {
    if (ev->code == ABS_MT_SLOT) {
        active_slot = ev->value;
    } else if (ev->code == ABS_MT_POSITION_X && active_slot < MAX_SLOTS) {
        slots[active_slot][0] = ev->value;
    } else if (ev->code == ABS_MT_POSITION_Y && active_slot < MAX_SLOTS) {
        slots[active_slot][1] = ev->value;
    }
}

// Process gestures and perform actions
void process_gesture(int uinput_fd) {
    if (active_slot != SLOT_TO_TRACK || slots[SLOT_TO_TRACK][0] == -1 || slots[SLOT_TO_TRACK][1] == -1) {
        return;
    }

    int current_x = slots[SLOT_TO_TRACK][0];
    int current_y = slots[SLOT_TO_TRACK][0];
    printf("Processing gesture: X = %d, Y = %d\n", current_x, current_y);

    if (base_point[0] == -1 && base_point[1] == -1) {
        base_point[0] = current_x;
        base_point[1] = current_y;
        printf("Base point: X = %d, Y = %d\n", base_point[0], base_point[1]);
        return;
    }

    int delta_x = current_x - base_point[0];
    int delta_y = current_y - base_point[1];

    printf("Delta X: %d, Delta Y: %d\n", delta_x, delta_y);

    if (delta_x > THRESHOLD && delta_y > THRESHOLD) {
        printf("Forward movement -> Ctrl + PgDn\n");
        simulate_keypress(uinput_fd, KEY_LEFTCTRL, KEY_PAGEDOWN);
        base_point[0] = current_x; 
        base_point[1] = current_y; 
    } else if (delta_x < -THRESHOLD && delta_y < -THRESHOLD) {
            static int first_move_detected = 0;
            if ( first_move_detected) {
                printf("Backward movement -> Ctrl + PgUp\n");
                simulate_keypress(uinput_fd, KEY_LEFTCTRL, KEY_PAGEUP);
                base_point[0] = current_x;
                base_point[1] = current_y;
            } else {
                    printf("First move detected, ignoring backward action\n");
                    first_move_detected = 1;
            }

    }
}

// Close files and clean up resources
void cleanup(int fd, int uinput_fd) {
    close(fd);
    ioctl(uinput_fd, UI_DEV_DESTROY);
    close(uinput_fd);
}

int main() {
    int uinput_fd = setup_uinput_device();
    if (uinput_fd < 0) return 1;

    const char *device = "/dev/input/event10";
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
