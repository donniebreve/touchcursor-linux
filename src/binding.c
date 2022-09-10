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
static char input_device_name[256] = "Unknown";
int input = -1;

// The output device
static char output_device_sys_path[256] = "\0";
int output = -1;

/**
 * Binds to the input device using ioctl.
 * */
int bind_input(char* event_path)
{
    // Open the keyboard device
    fprintf(stdout, "info: attempting to capture: %s\n", event_path);
    input = open(event_path, O_RDONLY);
    if (input < 0)
    {
        fprintf(stderr, "error: failed to open the input device: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    // Retrieve the device name
    if (ioctl(input, EVIOCGNAME(sizeof(input_device_name)), input_device_name) < 0)
    {
        fprintf(stderr, "error: failed to get the device name (EVIOCGNAME: %s)\n", strerror(errno));
        return EXIT_FAILURE;
    }
    // Check that the device is not our virtual device
    if (strcasestr(input_device_name, "Virtual TouchCursor Keyboard") != NULL)
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
    fprintf(stdout, "info: successfully captured device %s (%s)\n", input_device_name, event_path);
    return EXIT_SUCCESS;
}

/**
 * Releases the input device.
 * */
int release_input(char* event_path)
{
    fprintf(stdout, "info: releasing: %s (%s)\n", input_device_name, event_path);
    ioctl(input, EVIOCGRAB, 0);
    close(input);
    return EXIT_SUCCESS;
}

/**
 * Creates and binds a virtual output device using ioctl and uinput.
 * */
int bind_output()
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
    for (int i = 0; i <= MAX_KEYBIT; i++)
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
    // Get the device path
    char sysname[16];
    if (ioctl(output, UI_GET_SYSNAME(sizeof(sysname)), sysname) < 0)
    {
        fprintf(stderr, "error: failed to get the sysfs name (UI_GET_SYSNAME: %s)\n", strerror(errno));
        return EXIT_FAILURE;
    }
    strcat(output_device_sys_path, "/sys/devices/virtual/input/");
    strcat(output_device_sys_path, sysname);
    fprintf(stdout, "info: successfully created output Virtual TouchCursor Keyboard (%s)\n", output_device_sys_path);
    return EXIT_SUCCESS;
}

/**
 * Releases the virtual output device.
 * */
int release_output()
{
    fprintf(stdout, "info: releasing: Virtual TouchCursor Keyboard (%s)\n", output_device_sys_path);
    ioctl(output, UI_DEV_DESTROY);
    close(output);
    return EXIT_SUCCESS;
}
