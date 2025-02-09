#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/uinput.h>
#include <linux/input.h>
#include <string.h>
#include <stdlib.h>

#define ZONE_MIN_X 0       // Define the left edge zone
#define ZONE_MAX_X 150     // Define the right edge zone
#define SCREEN_BRIGHTNESS_PATH "/sys/class/backlight/intel_backlight/brightness"
#define MAX_BRIGHTNESS_PATH "/sys/class/backlight/intel_backlight/max_brightness"

// Define brightness range
#define BRIGHTNESS_MIN 193
#define BRIGHTNESS_MAX 19393

// Define touchpad boundaries (adjust based on your device)
#define TOUCHPAD_MIN_Y 0    // Upper position
#define TOUCHPAD_MAX_Y 670 // Lower position (find actual max for your device)

// Global variables
int touch_x = -1;
int touch_y = -1;

// Set the screen brightness value
void set_brightness(int brightness) {
    FILE *fp = fopen(SCREEN_BRIGHTNESS_PATH, "w");
    if (fp) {
        fprintf(fp, "%d", brightness);
        fclose(fp);
    } else {
        perror("Failed to set brightness");
    }
}

// Map touch Y-coordinate to brightness range
int map_touch_to_brightness(int y) {
    if (y < TOUCHPAD_MIN_Y) y = TOUCHPAD_MIN_Y;
    if (y > TOUCHPAD_MAX_Y) y = TOUCHPAD_MAX_Y;

    // Reverse mapping: lower Y means lower brightness, higher Y means higher brightness
    return BRIGHTNESS_MIN + (BRIGHTNESS_MAX - BRIGHTNESS_MIN) * (y - TOUCHPAD_MIN_Y) / (TOUCHPAD_MAX_Y - TOUCHPAD_MIN_Y);
}

void handle_abs_event(struct input_event *ev) {
    if (ev->code == ABS_MT_POSITION_X) {
        touch_x = ev->value;
    } else if (ev->code == ABS_MT_POSITION_Y) {
        touch_y = ev->value;

        // Ensure gesture is within the work zone
        if (touch_x >= ZONE_MIN_X && touch_x <= ZONE_MAX_X) {
            int brightness = map_touch_to_brightness(touch_y);
            set_brightness(brightness);
            printf("Brightness: %d\n", brightness);
        }
    }
}

int main() {
    const char *device = "/dev/input/event9";  // Change based on your device
    int fd = open(device, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open input device");
        return 1;
    }

    struct input_event ev;
    printf("Reading events... (Swipe vertically to adjust brightness)\n");

    while (read(fd, &ev, sizeof(ev)) > 0) {
        if (ev.type == EV_ABS) {
            handle_abs_event(&ev);
        }
    }

    close(fd);
    return 0;
}

