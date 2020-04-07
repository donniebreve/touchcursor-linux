#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <linux/input.h>

#include "config.h"
#include "binding.h"
#include "emit.h"
#include "touchcursor.h"

/**
* Main method.
*/
int main(int argc, char* argv[])
{
    readConfiguration();
    if (eventPath[0] == '\0')
    {
        fprintf(stderr, "error: please specify the keyboard device name in the configuration file\n");
        return EXIT_FAILURE;
    }

    // Bind the input device
    bindInput(eventPath);
    if (input == -1)
    {
        fprintf(stderr, "error: could not capture the keyboard device\n");
        return EXIT_FAILURE;
    }

    // Bind the output device
    bindOutput();
    if (output == -1)
    {
        fprintf(stderr, "error: could not create the virtual keyboard device\n");
        return EXIT_FAILURE;
    }

    fprintf(stdout, "info: running\n");

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
