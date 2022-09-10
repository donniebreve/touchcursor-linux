#define _GNU_SOURCE
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <linux/input.h>

#include "config.h"
#include "binding.h"
#include "emit.h"
#include "mapper.h"

volatile sig_atomic_t should_reload = 0;
volatile sig_atomic_t should_exit = 0;

/**
 * Handles signal events.
 * */
static void on_signal(int signal)
{
    if (signal == SIGHUP)
    {
        should_reload = 1;
    }
    else if (signal == SIGINT || signal == SIGTERM)
    {
        should_exit = 1;
    }
}

/**
 * Attaches signal event handlers.
 * */
static int attach_signal_handlers()
{
    stack_t signal_stack;
    if ((signal_stack.ss_sp = malloc(SIGSTKSZ)) == NULL)
    {
        fprintf(stderr, "error: unable to allocate signal stack: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    signal_stack.ss_size = SIGSTKSZ;
    signal_stack.ss_flags = 0;
    if (sigaltstack(&signal_stack, (stack_t*)0) < 0)
    {
        fprintf(stderr, "error: sigaltstack: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    struct sigaction signal_action;
    signal_action.sa_flags = SA_ONSTACK;
    sigemptyset(&signal_action.sa_mask);
    signal_action.sa_handler = on_signal;
    sigaction(SIGHUP, &signal_action, NULL);
    sigaction(SIGINT, &signal_action, NULL);
    sigaction(SIGTERM, &signal_action, NULL);
    return 0;
}

static void clean_up()
{
    release_input(event_path);
    release_output();
}

/**
* Main method.
*
* @remarks
*   read: Read NBYTES into BUF from FD. Return the number read, -1 for errors or 0 for EOF.
*   EOF doesn't make sense here. Partial events will be ignored.
*   https://docs.kernel.org/input/uinput.html
*   https://stackoverflow.com/questions/20943322/accessing-keys-from-linux-input-device
* */
int main(int argc, char* argv[])
{
    if (attach_signal_handlers() < 0)
    {
        return EXIT_FAILURE;
    }
    if (read_configuration() != EXIT_SUCCESS)
    {
        fprintf(stderr, "error: failed to read the configuration\n");
        return EXIT_FAILURE;
    }
    // Bind the input device
    if (bind_input(event_path) != EXIT_SUCCESS)
    {
        fprintf(stderr, "error: could not capture the keyboard device\n");
        return EXIT_FAILURE;
    }
    // Bind the output device
    if (bind_output() != EXIT_SUCCESS)
    {
        fprintf(stderr, "error: could not create the virtual keyboard device\n");
        return EXIT_FAILURE;
    }
    fprintf(stdout, "info: running\n");
    // Read events
    struct input_event event;
    ssize_t result;
    while (1)
    {
        if (should_reload)
        {
            fprintf(stdout, "info: reloading\n");
            release_input(event_path);
            if (read_configuration() != EXIT_SUCCESS)
            {
                fprintf(stderr, "error: failed to read the configuration\n");
                return EXIT_FAILURE;
            }
            if (bind_input(event_path) != EXIT_SUCCESS)
            {
                fprintf(stderr, "error: could not capture the keyboard device\n");
                return EXIT_FAILURE;
            }
            should_reload = 0;
        }
        if (should_exit)
        {
            fprintf(stdout, "info: exiting\n");
            clean_up();
            return EXIT_SUCCESS;
        }
        result = read(input, &event, sizeof(event));
        if (result == (ssize_t)-1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                fprintf(stderr, "error: unable to read event\n");
                fprintf(stdout, "info: exiting\n");
                clean_up();
                return EXIT_FAILURE;
            }
        }
        if (result == (ssize_t)0)
        {
            fprintf(stderr, "error: received EOF\n");
            fprintf(stdout, "info: exiting\n");
            clean_up();
            return EXIT_FAILURE;
        }
        if (result != sizeof(event))
        {
            fprintf(stdout, "warning: partial event received\n");
            continue;
        }
        // We only want to manipulate key presses
        if (event.type == EV_KEY
            && (event.value == 0 || event.value == 1 || event.value == 2))
        {
            processKey(event.type, event.code, event.value);
        }
        else
        {
            emit(event.type, event.code, event.value);
        }
    }
}
