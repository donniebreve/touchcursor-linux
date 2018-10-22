/**
 * Touch Cursor for Linux
 * Replicates the touch cursor style movement under linux (works under wayland)
 *
 * Special thanks to Thomas Bocek for the starting point for this application.
 * Special thanks to Martin Stone for Touch Cursor project.
 *
 * Running
 * make sure the file has the sudo execution permission
 * pass the /dev/input/by-path/id path of your keyboard
 *
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <string.h>
#include <stdio.h>

/**
* Emits a key event
*/
int emit(int fileDescriptor, int type, int code, int value)
{
   struct input_event ie;
   ie.type = type;
   ie.code = code;
   ie.value = value;
   /* timestamp values below are ignored */
   ie.time.tv_sec = 0;
   ie.time.tv_usec = 0;
   return write(fileDescriptor, &ie, sizeof(ie));
}

/**
* Converts input key to touch cursor key
* To do: make this configurable
*/
static int convertInput(int key)
{
    switch (key)
    {
        case 23: return 103; // i - up
        case 36: return 105; // j - left
        case 37: return 108; // k - down
        case 38: return 106; // l - right
        case 22: return 102; // u - home
        case 24: return 107; // o - end
        case 25: return 14;  // p - backspace
        case 35: return 104; // h - page up
        case 49: return 109; // n - page down
        case 50: return 111; // m - del
        default: return key;
    }
}

/**
* Main method.
*/
int main(int argc, char* argv[])
{
    // Set the user id 
    if (setuid(0) < 0)
    {
        fprintf(stderr, "error: setuid(0) failed: %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }
    // Check the argument count
    if(argc < 2)
    {
        fprintf(stderr, "error: please specify the input device found in /dev/input/by-id/\n");
        return EXIT_FAILURE;
    }

    // Open the keyboard device
    int input = open(argv[1], O_RDONLY);
    if (input < 0)
    {
        fprintf(stderr, "error: cannot open the input device: %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }
    // Retrieve the device name
    char keyboardName[256] = "Unknown";
    if (ioctl(input, EVIOCGNAME(sizeof(keyboardName) - 1), keyboardName) < 0)
    {
        fprintf(stderr, "error: cannot get the device name: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    else
    {
        fprintf(stdout, "attached to: %s\n", keyboardName);
    }
    // Check that the device is not our virtual device
    if (strcasestr(keyboardName, "Virtual TouchCursor Keyboard") != NULL)
    {
        fprintf(stderr, "error: cannot attach to the virtual device: %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }
    // Allow last key press to go through
    // Grabbing the keys too quickly prevents the last key up event from being sent
    // https://bugs.freedesktop.org/show_bug.cgi?id=101796
    usleep(200 * 1000);
    // Grab keys from the input device
    if (ioctl(input, EVIOCGRAB, 1) < 0)
    {
        fprintf(stderr, "error: EVIOCGRAB: %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }

    // Define the virtual keyboard
    struct uinput_user_dev virtualKeyboard;
    memset(&virtualKeyboard, 0, sizeof(virtualKeyboard));
    snprintf(virtualKeyboard.name, UINPUT_MAX_NAME_SIZE, "Virtual TouchCursor Keyboard");
    virtualKeyboard.id.bustype = BUS_USB;
    virtualKeyboard.id.vendor  = 0x01;
    virtualKeyboard.id.product = 0x01;
    virtualKeyboard.id.version = 1;

    // Open the output
    int output = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (output < 0)
    {
        fprintf(stderr, "error: failed to open /dev/uinput: %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }
    // Enable key press/release event
    if (ioctl(output, UI_SET_EVBIT, EV_KEY) < 0)
    {
        fprintf(stderr, "error: cannot set EV_KEY on output: %s.\n", strerror(errno));
    }
    // Enable set of KEY events
    for (int i = 0; i < KEY_MAX; i++)
    {
        if (ioctl(output, UI_SET_KEYBIT, i) < 0)
        {
            fprintf(stderr, "error: cannot set key bit: %s.\n", strerror(errno));
            return EXIT_FAILURE;
        }
    }
    // Enable synchronization event
    if (ioctl(output, UI_SET_EVBIT, EV_SYN) < 0)
    {
        fprintf(stderr, "error: cannot set EV_SYN on output: %s\n", strerror(errno));
    }
    // Write the uinput_user_dev structure into uinput file descriptor
    if (write(output, &virtualKeyboard, sizeof(virtualKeyboard)) < 0)
    {
        fprintf(stderr, "error: cannot write uinput_user_dev struct into uinput file descriptor: %s\n", strerror(errno));
    }
    // create the device via an IOCTL call 
    if (ioctl(output, UI_DEV_CREATE) < 0)
    {
        fprintf(stderr, "error: ioctl: UI_DEV_CREATE: %s\n", strerror(errno));
    }
    
    int hyper = 0;
    int keyPressed = 0;
    struct input_event inputEvent;
    while (1)
    {
        ssize_t result = read(input, &inputEvent, sizeof(inputEvent));   
        if (result == (ssize_t) - 1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            break;
        }
        if (result == (ssize_t)0)
        {
            errno = ENOENT;
            break;                
        }
        if (result != sizeof(inputEvent))
        {
            errno = EIO;
            break;
        }

        // We only want key presses
        if (inputEvent.type != EV_KEY || (inputEvent.value != 0 && inputEvent.value != 1 && inputEvent.value != 2)) continue;

        //fprintf(stdout, "input - type: %i, code: %i, value: %i\n", inputEvent.type, inputEvent.code, inputEvent.value);

        // If the space bar is pressed down
        if (inputEvent.code == 57)
        {
            if (inputEvent.value == 1)
            {
                hyper = 1;
                keyPressed = 0;
                continue;
            }
            if (inputEvent.value == 2)
            {
                continue;
            }
            if (inputEvent.value == 0)
            {
                hyper = 0;
                if (keyPressed == 0)
                {
                    emit(output, inputEvent.type, inputEvent.code, 1);
                    emit(output, inputEvent.type, inputEvent.code, 0);
                    emit(output, EV_SYN, 0, 0);
                }
                continue;
            }
        }
        if (hyper == 1)
        {
            inputEvent.code = convertInput(inputEvent.code);
            if (inputEvent.value == 1 || inputEvent.value == 2)
            {
                keyPressed = 1;
            }
        }

        //fprintf(stdout, "output - type: %i, code: %i, value: %i\n", inputEvent.type, inputEvent.code, inputEvent.value);

        emit(output, inputEvent.type, inputEvent.code, inputEvent.value);
        emit(output, EV_SYN, 0, 0);
    }
}
