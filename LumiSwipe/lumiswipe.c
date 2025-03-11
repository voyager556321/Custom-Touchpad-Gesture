#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define ZONE_MIN_X 0       // Define the left edge zone
#define ZONE_MAX_X 150     // Define the right edge zone
#define SCREEN_BRIGHTNESS_PATH "/sys/class/backlight/intel_backlight/brightness"

// Define brightness range
#define BRIGHTNESS_MIN 193
#define BRIGHTNESS_MAX 19393

// Define touchpad boundaries
#define TOUCHPAD_MIN_Y 0    // Upper position
#define TOUCHPAD_MAX_Y 670  // Lower position

#define MIN_MOVEMENT_THRESHOLD 5    // Minimum movement to count as intentional
#define IGNORE_MULTITOUCH 1         // Set to 1 to ignore multi-touch events
#define ACTIVATION_TIME_MS 2000      // Require 1s sustained touch before activation
#define ZONE_TIMEOUT_MS 2000         // Zone becomes inactive after 2 seconds
#define REACTIVATION_HOLD_MS 2000    // Hold for 1s to reactivate the zone

// Global variables
int touch_x = -1, touch_y = -1;
int prev_touch_x = -1, prev_touch_y = -1;
int touch_active = 0;  // Flag to check if finger is moving
int touch_started = 0; // Flag to check if touch began
int zone_active = 1;   // Whether the brightness adjustment zone is active

struct timespec touch_start_time, last_touch_time, zone_deactivation_time;

// Function to get current time in milliseconds
long current_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

// Set screen brightness
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

    // Reverse mapping: Lower Y = lower brightness, Higher Y = higher brightness
    return BRIGHTNESS_MIN + (BRIGHTNESS_MAX - BRIGHTNESS_MIN) * 
           (y - TOUCHPAD_MIN_Y) / (TOUCHPAD_MAX_Y - TOUCHPAD_MIN_Y);
}

// Handle touch input events
void handle_abs_event(struct input_event *ev) {
    if (ev->code == ABS_MT_SLOT && IGNORE_MULTITOUCH) {
        // Ignore multi-touch slots, process only the first finger
        return;
    }

    if (ev->code == ABS_MT_POSITION_X) {
        touch_x = ev->value;
    } else if (ev->code == ABS_MT_POSITION_Y) {
        touch_y = ev->value;
        
        long now = current_time_ms();

        // Check if touch has lasted long enough for activation
        if (!touch_started) {
            touch_started = 1;
            clock_gettime(CLOCK_MONOTONIC, &touch_start_time);
            return;
        }

        long touch_duration = now - (touch_start_time.tv_sec * 1000 + touch_start_time.tv_nsec / 1000000);
        if (touch_duration < ACTIVATION_TIME_MS) {
            return;  // Ignore quick accidental touches
        }

        // Handle zone reactivation if it's inactive
        if (!zone_active) {
            long hold_time = now - (zone_deactivation_time.tv_sec * 1000 + zone_deactivation_time.tv_nsec / 1000000);
            if (hold_time >= REACTIVATION_HOLD_MS) {
                zone_active = 1;
                printf("Zone reactivated!\n");
            }
            return;
        }

        // First touch, ignore it as accidental tap
        if (prev_touch_x == -1 || prev_touch_y == -1) {
            prev_touch_x = touch_x;
            prev_touch_y = touch_y;
            return;
        }

        // Check if finger moved significantly
        int dx = abs(touch_x - prev_touch_x);
        int dy = abs(touch_y - prev_touch_y);

        if (dx > MIN_MOVEMENT_THRESHOLD || dy > MIN_MOVEMENT_THRESHOLD) {
            touch_active = 1;  // Movement detected
        }

        if (touch_active && zone_active && touch_x >= ZONE_MIN_X && touch_x <= ZONE_MAX_X) {
            int brightness = map_touch_to_brightness(touch_y);
            set_brightness(brightness);
            printf("Brightness: %d\n", brightness);
        }

        // Update last touch time
        clock_gettime(CLOCK_MONOTONIC, &last_touch_time);

        // Update previous coordinates
        prev_touch_x = touch_x;
        prev_touch_y = touch_y;
    }
}

// Handle finger lift event
void handle_key_event(struct input_event *ev) {
    if (ev->code == BTN_TOUCH && ev->value == 0) {
        long now = current_time_ms();
        long time_since_last_touch = now - (last_touch_time.tv_sec * 1000 + last_touch_time.tv_nsec / 1000000);

        // If the zone was active and no touch happened in the last 2s, deactivate it
        if (zone_active && time_since_last_touch >= ZONE_TIMEOUT_MS) {
            zone_active = 0;
            clock_gettime(CLOCK_MONOTONIC, &zone_deactivation_time);
            printf("Zone deactivated due to inactivity.\n");
        }

        // Reset touch state when finger is lifted
        touch_x = touch_y = -1;
        prev_touch_x = prev_touch_y = -1;
        touch_active = 0;
        touch_started = 0;
    }
}

int main(void) {
        FILE *fp = popen("libinput list-devices | awk '/Touchpad/{getline; print $2}'", "r");
        if (!fp) {
                perror("popen failed");
                return 1;
        }
        char event[32];
        if (!fgets(event, sizeof(event), fp)) {
                perror("Failed to get event device");
                fclose(fp);
                return 1;
        }

        event[strcspn(event, "\n")] = 0;

        const char *device = event;
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
                } else if (ev.type == EV_KEY) {
                        handle_key_event(&ev);
                }
        }

        close(fd);
        fclose(fp);
        return 0;
}
