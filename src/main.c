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

static int should_reload = 0;
static int should_exit = 0;

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
    else if (signal == SIGSEGV)
    {
        // cry
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
    sigaction(SIGSEGV, &signal_action, NULL);
    sigaction(SIGHUP, &signal_action, NULL);
    sigaction(SIGINT, &signal_action, NULL);
    sigaction(SIGTERM, &signal_action, NULL);
    return 0;
}

/**
* Main method.
* */
int main(int argc, char* argv[])
{
    if (attach_signal_handlers() < 0)
    {
        return EXIT_FAILURE;
    }
    readConfiguration();
    if (eventPath[0] == '\0')
    {
        fprintf(stderr, "error: please specify the keyboard device name in the configuration file\n");
        return EXIT_FAILURE;
    }
    // Bind the input device
    if (bind_input(eventPath) != EXIT_SUCCESS)
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
    struct input_event inputEvent;
    ssize_t result;
    while (1)
    {
        if (should_reload)
        {
            fprintf(stdout, "info: reloading\n");
            release_input(eventPath);
            readConfiguration();
            if (bind_input(eventPath) != EXIT_SUCCESS)
            {
                fprintf(stderr, "error: could not capture the keyboard device\n");
                return EXIT_FAILURE;
            }
            should_reload = 0;
        }
        if (should_exit)
        {
            fprintf(stdout, "info: exiting\n");
            release_input(eventPath);
            release_output();
            return EXIT_SUCCESS;
        }
        result = read(input, &inputEvent, sizeof(inputEvent));
        if (result == (ssize_t) - 1 && errno == EINTR)
        {
            continue;
        }
        if (result == (ssize_t)0)
        {
            return EXIT_FAILURE;
        }
        if (result != sizeof(inputEvent))
        {
            return EXIT_FAILURE;
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
