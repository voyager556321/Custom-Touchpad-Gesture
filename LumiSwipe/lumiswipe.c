#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/uinput.h>
#include <linux/input.h>
#include <string.h>

#define THRESHOLD 50  // Поріг для жесту

// Глобальна змінна для координати Y
int prev_y = -1; 

// Налаштування uinput пристрою
int setup_uinput_device() {
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("Failed to open /dev/uinput");
        return -1;
    }

    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_KEYBIT, KEY_BRIGHTNESSDOWN);
    ioctl(fd, UI_SET_KEYBIT, KEY_BRIGHTNESSUP);

    struct uinput_setup usetup;
    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234;
    usetup.id.product = 0x5678;
    strcpy(usetup.name, "Virtual Brightness Controller");

    if (ioctl(fd, UI_DEV_SETUP, &usetup) < 0 || ioctl(fd, UI_DEV_CREATE) < 0) {
        perror("Failed to setup uinput device");
        close(fd);
        return -1;
    }

    return fd;
}

// Відправка події натискання клавіші
void send_key_event(int fd, int keycode) {
    struct input_event ev = { .type = EV_KEY, .code = keycode, .value = 1 };
    write(fd, &ev, sizeof(ev));

    ev.value = 0;
    write(fd, &ev, sizeof(ev));

    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(fd, &ev, sizeof(ev));
}

// Обробка координат Y
void handle_abs_event(struct input_event *ev, int uinput_fd) {
    if (ev->code == ABS_MT_POSITION_Y) {
        int current_y = ev->value;
        
        if (prev_y != -1) {
            int diff = current_y - prev_y;

            if (diff > THRESHOLD) {
                send_key_event(uinput_fd, KEY_BRIGHTNESSDOWN);
                printf("Brightness Down\n");
            } else if (diff < -THRESHOLD) {
                send_key_event(uinput_fd, KEY_BRIGHTNESSUP);
                printf("Brightness Up\n");
            }
        }
        prev_y = current_y;
    }
}

// Очищення ресурсів
void cleanup(int fd, int uinput_fd) {
    close(fd);
    ioctl(uinput_fd, UI_DEV_DESTROY);
    close(uinput_fd);
}

int main() {
    int uinput_fd = setup_uinput_device();
    if (uinput_fd < 0) return 1;

    const char *device = "/dev/input/event9";  // Заміни на правильний шлях
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
            handle_abs_event(&ev, uinput_fd);
        }
    }

    cleanup(fd, uinput_fd);
    return 0;
}

