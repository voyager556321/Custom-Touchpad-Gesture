# Custom-Touchpad-Gesture
This project demonstrates a simple application to recognize multi-touch gestures and simulate key presses (such as "Ctrl + Page Down" or "Ctrl + Page Up") based on those gestures. It uses Linux input events (ABS_MT events) to track the movement of touch points and the uinput subsystem to send key events.

## Description

This software is intended for users running **GNOME** on **Wayland**. It allows you to recognize multi-touch gestures on the touchpad and perform corresponding actions, such as key presses (e.g. `Ctrl + PageDown` to scroll down) using the specified gestures. This software supports settings for various gestures, which allows you to improve interaction with the device for users who actively work with Wayland-based graphical interfaces.

### Who is this software for?

- **GNOME Developers:** If you are developing or configuring the GNOME environment on Wayland, this solution will allow you to integrate multi-touch gestures into your workflow.
- **GNOME Users on Wayland:** If you are using GNOME on Wayland, this software will help you to facilitate navigation using gestures, replacing the usual key presses.
- **Integrators and Tech Enthusiasts:** If you want to customize and integrate new touchpad functionality into GNOME, this software allows you to configure various settings for user convenience.

## License

This project is available under **dual licensing**:

- **GNU GPLv3** (for open source projects)
- **Commercial License** (for commercial use)

💡 If you would like to use this project in a commercial product, please [contact](mailto:vovls5433@gmail).



## 1. Project Overview
This program reads touch events from an input device (like a touchscreen) and processes gestures to simulate specific keypresses. It tracks touch slots (fingers) and moves based on the gestures. When a specific gesture (e.g., swipe up or down) is detected, the program simulates pressing certain keys, such as "Ctrl + PgDown" for a forward movement or "Ctrl + PgUp" for backward movement.

## 2. Prerequisites
Before running the program, ensure the following:
- A Linux system with `uinput` support enabled.
- A touchscreen or touchpad that generates `ABS_MT` events.
- The `input.h` header file and `uinput.h` should be available.
Make sure that the required permissions are set for accessing `/dev/uinput` and `/dev/input/eventX` files (where `X` is the event number of the input device).

## 3. How the Code Works
The program operates in the following key steps:

1. **Setting up the uinput device:** The function `setup_uinput_device()` configures a virtual input device using the uinput subsystem. This device is used to simulate key events (such as pressing "Ctrl + Page Down" or "Ctrl + Page Up").

    - **IOCTL calls** set up event types (`EV_KEY `for key events).

    - **uinput_setup struct** sets the properties of the virtual device.

2. **Reading touch input events:** The program opens the input device (e.g., `/dev/input/event10`) to read `ABS_MT` events. These events contain information about touch position and touch slot.

   - The `handle_abs_event()` function processes the events, updating the touch slot coordinates (`ABS_MT_POSITION_X`, `ABS_MT_POSITION_Y`) based on the active slot.

3. **Processing gestures:**  The function process_gesture() monitors the position of the third touch slot (`SLOT_TO_TRACK = 2`). The program calculates the movement from the previous base point.

   - If a large enough movement is detected (greater than the `THRESHOLD`), the program simulates a key press using the `simulate_keypress()` function. 

   - For forward gestures, "Ctrl + Page Down" is simulated.

   - For forward gestures, "Ctrl + Page Down" is simulated.

   

4. **Simulating key presses:** The function `simulate_keypress()` is responsible for sending the actual key press and release events for the virtual keyboard:

   - `EV_KEY` is used to send key down and key up events.
   - `EV_SYN` is used to indicate the end of the key event sequence.

5. **Cleaning up resources:** The `cleanup()` function closes the input device and destroys the uinput device.

## 4. Using the Program
To build and run the program:

1. **Compile the code:** Run `make` to compile the code:

   ``` make ``` 

2. **Run the program:** Run the compiled program with `sudo` (necessary for accessing `/dev/uinput` and input devices):

   ```sudo ./custom-touchpad-gesture``` 
   
   The program will start reading touch events from the input device and simulate key presses based on the detected gestures.
   
## 5. Program Configuration
- **SLOT_TO_TRACK:** Defines the slot to track for gesture recognition. In this case, slot 2 (the third touch point) is tracked.

- **THRESHOLD:** The threshold value that determines when a movement is considered significant enough to trigger a key event.

- **MAX_SLOTS:** The maximum number of slots (fingers) supported for touch tracking.

## 6. Troubleshooting

  - **Permission Issues:** If you encounter permission issues accessing `/dev/uinput` or 	`/dev/input/eventX`, make sure your user has the correct permissions. You may need to add your user to the `input` group or adjust device permissions manually.
  - **No Input Device Found:** Ensure the input device path (`/dev/input/eventX`) is correct. You can find the appropriate event device using the `ls /dev/input/` command.
  - **No Gesture Detected:** Check if the touch input device supports multitouch and that the input events are being generated. Use `evtest` or `libinput debug-events` to verify the events.
