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
#include <time.h>

#include "queue.h"

initq();

// The output device
int output;

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
int convertInput(int key)
{
    switch (key)
    {
        case KEY_I: return KEY_UP;
        case KEY_J: return KEY_LEFT;
        case KEY_K: return KEY_DOWN;
        case KEY_L: return KEY_RIGHT;
        case KEY_U: return KEY_HOME;
        case KEY_O: return KEY_END;
        case KEY_P: return KEY_BACKSPACE;
        case KEY_H: return KEY_PAGEUP;
        case KEY_N: return KEY_PAGEDOWN;
        case KEY_M: return KEY_DELETE;
        default: return key;
    }
}

/**
 * Checks if the key is mapped.
 */
int isMapped(int key)
{
    switch (key)
    {
        case KEY_I:
        case KEY_J:
        case KEY_K:
        case KEY_L:
        case KEY_U:
        case KEY_O:
        case KEY_P:
        case KEY_H:
        case KEY_N:
        case KEY_M:
            return 1;
        default:
            return 0;
    }
}

/**
 * Checks if the key is a modifier key.
 */
int isModifier(int key)
{
    switch (key)
    {
        case KEY_ESC:
        case KEY_BACKSPACE:
        case KEY_TAB:
        case KEY_ENTER:
        case KEY_LEFTCTRL:
        case KEY_GRAVE:
        case KEY_LEFTSHIFT:
        case KEY_RIGHTSHIFT:
        case KEY_LEFTALT:
        case KEY_CAPSLOCK:
        case KEY_F1:
        case KEY_F2:
        case KEY_F3:
        case KEY_F4:
        case KEY_F5:
        case KEY_F6:
        case KEY_F7:
        case KEY_F8:
        case KEY_F9:
        case KEY_F10:
        case KEY_NUMLOCK:
        case KEY_SCROLLLOCK:
            return 1;
        default:
            return 0;
    }
}

enum states
{
    idle,
    delay,
    waiting,
    mapping
};
enum states state;

int hyper = 0; // if the hyper key is pressed or not
int hyperEmitted = 0; // if the hyper key down has been emitted
int mappedKeyPressed = 0; // if a mapped key was pressed
int captured = 0; // if a mapped key has been captured
int captureEmitted = 0; // if the captured mapped key has been emitted
struct input_event capturedEvent; // the mapped key down captured event

/*
 * Process the key event. Returns true if the key should be discarded.
 */
int processKey(struct input_event inputEvent)
{
    switch (state)
    {
        case idle:
            if (inputEvent.code == 57)
            {
                if (inputEvent.value == 2)
                {
                    return 1;
                }
                if (inputEvent.value == 1)
                {
                    hyper = 1;
                    return 1;
                }
                if (inputEvent.value == 0)
                {
                    emit(output, EV_KEY, KEY_SPACE, 1);
                }
            }
            if (hyper == 1)
            {
                if (inputEvent.value == 1 || inputEvent.value == 2)
                {
                    if (isMapped(inputEvent.code))
                    {
                        enqueue(inputEvent.code);
                        state = delay;
                        return 1;
                    }
                }
            }
            break;
        case delay:
            if (inputEvent.value == 1 || inputEvent.value == 2)
            {
                if (isMapped(inputEvent.code))
                {
                    enqueue(inputEvent.code);
                    return 1;
                }
            }
            break;
    }




    if (inputEvent.code == 57)
    {
        if (inputEvent.value == 2)
        {
            return 1;
        }
        if (inputEvent.value == 1)
        {
            hyper = 1;
            hyperEmitted = 0;
            mappedKeyPressed = 0;
            captured = 0;
            captureEmitted = 0;
            return 1;
        }
        if (inputEvent.value == 0)
        {
            if (!mappedKeyPressed && !hyperEmitted)
            {
                emit(output, EV_KEY, KEY_SPACE, 1);
                emit(output, EV_SYN, 0, 0);
            }
            if (captured && !captureEmitted)
            {
                emit(output, EV_KEY, KEY_SPACE, 1);
                emit(output, EV_KEY, capturedEvent.code, capturedEvent.value);
                emit(output, EV_SYN, 0, 0);
            }
            hyper = 0;
        }
    }
    if (hyper)
    {
        if (inputEvent.value == 1 || inputEvent.value == 2)
        {
            if (isMapped(inputEvent.code))
            {
                mappedKeyPressed = 1;
                if (!captured)
                {
                    printf("captured 1\n");
                    capturedEvent = inputEvent;
                    captured = 1;
                    return 1;
                }
                else
                {
                    if (!captureEmitted)
                    {
                        emit(output, EV_KEY, convertInput(capturedEvent.code), capturedEvent.value);
                        emit(output, EV_SYN, 0, 0);
                        captureEmitted = 1;
                    }
                    emit(output, EV_KEY, convertInput(inputEvent.code), inputEvent.value);
                    return 1;
                }
            }
            else
            {
                if (!hyperEmitted)
                {
                    emit(output, EV_KEY, KEY_SPACE, 1);
                    emit(output, EV_SYN, 0, 0);
                    hyperEmitted = 1;
                }
            }
        }
        if (inputEvent.value == 0)
        {
            if (isMapped(inputEvent.code))
            {
                if (captured && !captureEmitted)
                {
                    emit(output, EV_KEY, convertInput(capturedEvent.code), capturedEvent.value);
                    emit(output, EV_SYN, 0, 0);
                    captureEmitted = 1;
                }
                else
                {
                    printf("captured 2\n");
                    capturedEvent = inputEvent;
                    captured = 1;
                    return 1;
                }
                mappedKeyPressed = 1;
                emit(output, EV_KEY, convertInput(inputEvent.code), inputEvent.value);
                return 1;
            }
            else
            {
                if (!hyperEmitted)
                {
                    emit(output, EV_KEY, KEY_SPACE, 1);
                    emit(output, EV_SYN, 0, 0);
                    hyperEmitted = 1;
                }
                if (captured && !captureEmitted)
                {
                    emit(output, EV_KEY, convertInput(capturedEvent.code), capturedEvent.value);
                    emit(output, EV_SYN, 0, 0);
                    captureEmitted = 1;
                }
            }
        }
    }
    return 0;
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
    output = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
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
    
    struct input_event inputEvent;
    ssize_t result;
    while (1)
    {
        result = read(input, &inputEvent, sizeof(inputEvent));
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

        int discard = 0;
        // We only want to manipulate key presses
        if (inputEvent.type == EV_KEY
            && (inputEvent.value == 0 || inputEvent.value == 1 || inputEvent.value == 2)
            && !isModifier(inputEvent.code))
        {
            discard = processKey(inputEvent);
        }
        // Emit the key
        if (!discard)
        {
            emit(output, inputEvent.type, inputEvent.code, inputEvent.value);
            emit(output, EV_SYN, 0, 0);
        }
    }
}

//fprintf(stdout, "input - type: %i, code: %i, value: %i\n", inputEvent.type, inputEvent.code, inputEvent.value);