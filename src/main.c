#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <dirent.h>

#include <pwd.h>
#include <grp.h>

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
    if (eventPath[0] == 0)
    {
        fprintf(stderr, "error: please specify the keyboard device name in the configuration file\n");
        return EXIT_FAILURE;
    }

    printf("keyboard event: '%s'\n", eventPath);

    // Bind the input device
    bindInput(eventPath);

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
