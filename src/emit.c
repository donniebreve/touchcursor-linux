#include <linux/input.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "binding.h"
#include "emit.h"

/**
 * Emits a key event.
 * */
void emit(int type, int code, int value)
{
    if (type == EV_KEY)
    {
        // TODO: I don't like this here
        output_device_keystate[code] = value;
    }

    // printf("emit: code=%i value=%i\n", code, value);
    struct input_event e[2];
    memset(e, 0, sizeof(e));

    // Emit the virtual key code / value
    // time.tv_sec = 0
    // time.tv_usec = 0
    e[0].type = type;
    e[0].code = code;
    e[0].value = value;

    // Emit a syn event
    // time.tv_sec = 0
    // time.tv_usec = 0
    e[1].type = EV_SYN;
    e[1].code = SYN_REPORT;
    // value = 0

    write(output_file_descriptor, &e, sizeof(e));
}
