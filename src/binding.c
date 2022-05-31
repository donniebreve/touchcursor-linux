#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/uinput.h>

#include "binding.h"

// The input device
int input = -1;

// The output device
int output = -1;

/**
 * Binds to the input device using ioctl.
 * */
int bindInput(char* eventPath)
{
    // Open the keyboard device
    fprintf(stdout, "info: attempting to capture: '%s'\n", eventPath);
    input = open(eventPath, O_RDONLY);
    if (input < 0)
    {
        fprintf(stderr, "error: failed to open the input device: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    // Retrieve the device name
    char keyboardName[256] = "Unknown";
    if (ioctl(input, EVIOCGNAME(sizeof(keyboardName) - 1), keyboardName) < 0)
    {
        fprintf(stderr, "error: failed to get the device name (EVIOCGNAME: %s)\n", strerror(errno));
        return EXIT_FAILURE;
    }
    // Check that the device is not our virtual device
    if (strcasestr(keyboardName, "Virtual TouchCursor Keyboard") != NULL)
    {
        fprintf(stdout, "error: you cannot capture the virtual device: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    // Allow last key press to go through
    // Grabbing the keys too quickly prevents the last key up event from being sent
    // https://bugs.freedesktop.org/show_bug.cgi?id=101796
    usleep(200 * 1000);
    // Grab keys from the input device
    if (ioctl(input, EVIOCGRAB, 1) < 0)
    {
        fprintf(stdout, "error: failed to capture the device (EVIOCGRAB: %s)\n", strerror(errno));
        return EXIT_FAILURE;
    }
    fprintf(stdout, "info: successfully captured device %s\n", keyboardName);
    return EXIT_SUCCESS;
}

/**
 * Creates and binds a virtual output device using ioctl and uinput.
 * */
int bindOutput()
{
    // Define the virtual keyboard
    struct uinput_setup virtualKeyboard;
    memset(&virtualKeyboard, 0, sizeof(virtualKeyboard));
    strcpy(virtualKeyboard.name, "Virtual TouchCursor Keyboard");
    virtualKeyboard.id.bustype = BUS_USB;
    virtualKeyboard.id.vendor  = 0x01;
    virtualKeyboard.id.product = 0x01;
    virtualKeyboard.id.version = 1;
    // Open uinput
    output = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (output < 0)
    {
        fprintf(stdout, "error: failed to open /dev/uinput: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    // Enable key press/release events
    if (ioctl(output, UI_SET_EVBIT, EV_KEY) < 0)
    {
        fprintf(stdout, "error: failed to set EV_KEY on output (UI_SET_KEYBIT, EV_KEY: %s)\n", strerror(errno));
        return EXIT_FAILURE;
    }
    // Enable the set of KEY events
    for (int i = 0; i <= MAX_KEYS_TO_ENABLE_KEY_EVENTS_HANDLING_FOR; i++)
    {
        int result = ioctl(output, UI_SET_KEYBIT, i);
        if (result < 0)
        {
            fprintf(stdout, "error: failed to set key bit (UI_SET_KEYBIT, %i: %s)\n", i, strerror(errno));
            return EXIT_FAILURE;
        }
    }
    // Set up the device
    if (ioctl(output, UI_DEV_SETUP, &virtualKeyboard) < 0)
    {
       fprintf(stdout, "error: failed to set up the virtual device (UI_DEV_SETUP: %s)\n", strerror(errno)); 
       return EXIT_FAILURE;
    }
    // Create the device via an IOCTL call 
    if (ioctl(output, UI_DEV_CREATE) < 0)
    {
        fprintf(stdout, "error: failed to create the virtual device (UI_DEV_CREATE: %s)\n", strerror(errno));
        return EXIT_FAILURE;
    }
    fprintf(stdout, "info: successfully created virtual output device\n");
    return EXIT_SUCCESS;
}
