// build
// gcc -Wall binding.c emit.c queue.c touchcursor.c main.c  -o ../output/touchcursor

// run
// your useraccount must be in the input group (or equivalent). check ls -l /dev/input/.
// sudo usermod -a -G input user
// ./touchcursor /dev/input/event#

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
#include "emit.h"
#include "touchcursor.h"

/**
* Main method.
*/
int main(int argc, char* argv[])
{
    // Check the argument count
    if(argc < 2)
    {
        fprintf(stderr, "error: please specify the input device found in /dev/input/by-id/\n");
        return EXIT_FAILURE;
    }

    // Bind the input device
    bindInput(argv[1]);

    // Bind the output device
    bindOutput();

    // Read events
    struct input_event inputEvent;
    ssize_t result;
    while (1)
    {
        result = read(input, &inputEvent, sizeof(inputEvent));
        if (result == (ssize_t) - 1)
        {
            if (errno == EINTR) continue;
        }
        if (result == (ssize_t)0)
        {
            return ENOENT;
        }
        if (result != sizeof(inputEvent))
        {
            return EIO;
        }
        // We only want to manipulate key presses
        if (inputEvent.type == EV_KEY
            && !isModifier(inputEvent.code)
            && (inputEvent.value == 0 || inputEvent.value == 1 || inputEvent.value == 2))
        {
            processKey(inputEvent.type, inputEvent.code, inputEvent.value);
        }
        else
        {
            emit(inputEvent.type, inputEvent.code, inputEvent.value);
        }
    }
}