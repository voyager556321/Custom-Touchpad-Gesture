#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/uinput.h>
#include <linux/input.h>
#include <stdlib.h>
#include <string.h>

#define SLOT_TO_TRACK 2 // Відстеження третього пальця (slot 2)
#define THRESHOLD 150   // Поріг для виявлення руху

// Функція ініціалізації uinput
int setup_uinput_device() {
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("Не вдалося відкрити /dev/uinput");
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

    if (ioctl(fd, UI_DEV_SETUP, &usetup) < 0) {
        perror("UI_DEV_SETUP error");
        close(fd);
        return -1;
    }
    if (ioctl(fd, UI_DEV_CREATE) < 0) {
        perror("UI_DEV_CREATE error");
        close(fd);
        return -1;
    }

    return fd;
}

void send_key_event(int fd, int keycode, int value) {
    struct input_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_KEY;
    ev.code = keycode;
    ev.value = value;
    write(fd, &ev, sizeof(ev));

    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(fd, &ev, sizeof(ev));
}

void simulate_keypress(int fd, int modifier_keycode, int keycode) {
    send_key_event(fd, modifier_keycode, 1);
    send_key_event(fd, keycode, 1);
    send_key_event(fd, keycode, 0);
    send_key_event(fd, modifier_keycode, 0);
}

int main() {
    int uinput_fd = setup_uinput_device();
    if (uinput_fd < 0) {
        return 1;
    }

    const char *device = "/dev/input/event10";
    int fd = open(device, O_RDONLY);
    if (fd < 0) {
        perror("Не вдалося відкрити пристрій");
        return 1;
    }

    struct input_event ev;
    int slots[3][2] = {{-1, -1}, {-1, -1}, {-1, -1}};
    int active_slot = 0;
    int base_point[2] = {-1, -1};
    int current_point[2] = {-1, -1};

    printf("Читаю події... (Натисніть CTRL+C для виходу)\n");

    while (read(fd, &ev, sizeof(ev)) > 0) {
        if (ev.type == EV_ABS) {
            if (ev.code == ABS_MT_SLOT) {
                active_slot = ev.value;
            } else if (ev.code == ABS_MT_POSITION_X && active_slot < 3) {
                slots[active_slot][0] = ev.value;
            } else if (ev.code == ABS_MT_POSITION_Y && active_slot < 3) {
                slots[active_slot][1] = ev.value;
            }
        }

        if (active_slot == SLOT_TO_TRACK && slots[SLOT_TO_TRACK][0] != -1 && slots[SLOT_TO_TRACK][1] != -1) {
            if (base_point[0] == -1 && base_point[1] == -1) {
                base_point[0] = slots[SLOT_TO_TRACK][0];
                base_point[1] = slots[SLOT_TO_TRACK][1];
                printf("Базова точка: X = %d, Y = %d\n", base_point[0], base_point[1]);
            }

            current_point[0] = slots[SLOT_TO_TRACK][0];
            current_point[1] = slots[SLOT_TO_TRACK][1];

            int delta_x = current_point[0] - base_point[0];
            int delta_y = current_point[1] - base_point[1];

            if (delta_x > THRESHOLD && delta_y > THRESHOLD) {
                printf("Рух вперед -> Ctrl + PgDn\n");
                simulate_keypress(uinput_fd, KEY_LEFTCTRL, KEY_PAGEDOWN);
                base_point[0] = current_point[0];
                base_point[1] = current_point[1];
            } else if (delta_x < -THRESHOLD && delta_y < -THRESHOLD) {
                printf("Рух назад -> Ctrl + PgUp\n");
                simulate_keypress(uinput_fd, KEY_LEFTCTRL, KEY_PAGEUP);
                base_point[0] = current_point[0];
                base_point[1] = current_point[1];
            }
        }
    }

    close(fd);
    ioctl(uinput_fd, UI_DEV_DESTROY);
    close(uinput_fd);
    return 0;
}

