#define _GNU_SOURCE
#include <errno.h>
#include <limits.h>
#include <linux/input.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/select.h>
#include <unistd.h>

#include "binding.h"
#include "buffers.h"
#include "config.h"
#include "emit.h"
#include "mapper.h"

volatile sig_atomic_t should_reload = 0;
volatile sig_atomic_t should_exit = 0;
static int inotify_descriptor;
static int watch_descriptor;
static pthread_t main_thread_identifier;
static pthread_t watch_thread_identifier;

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
        error("error: unable to allocate signal stack: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    signal_stack.ss_size = SIGSTKSZ;
    signal_stack.ss_flags = 0;
    if (sigaltstack(&signal_stack, (stack_t*)0) < 0)
    {
        error("error: sigaltstack: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    struct sigaction signal_action;
    signal_action.sa_flags = SA_ONSTACK;
    sigemptyset(&signal_action.sa_mask);
    signal_action.sa_handler = on_signal;
    sigaction(SIGHUP, &signal_action, NULL);
    sigaction(SIGINT, &signal_action, NULL);
    sigaction(SIGTERM, &signal_action, NULL);
    return EXIT_SUCCESS;
}

/**
 * Reads inotify watch events.
 * */
static void* read_watch_events(void *)
{
    struct inotify_event event;
    while (1)
    {
        if (should_exit)
        {
            break;
        }
        ssize_t result = read(inotify_descriptor, &event, sizeof(event));
        if (result == (ssize_t)-1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                error("error: unable to read inotify event: %s\n", strerror(errno));
                error("error: file events will no longer be processed\n");
                break;
            }
        }
        if (result == (ssize_t)0)
        {
            error("error: received eof while reading inotify events\n");
            error("error: file events will no longer be processed\n");
            break;
        }
        if (result != sizeof(event))
        {
            warn("warning: partial inotify event received\n");
            continue;
        }
        if (event.mask & IN_MODIFY)
        {
            pthread_kill(main_thread_identifier, SIGHUP);
        }
        if (event.mask & IN_DELETE_SELF)
        {
            inotify_rm_watch(inotify_descriptor, watch_descriptor);
            watch_descriptor = inotify_add_watch(inotify_descriptor, configuration_file_path, IN_MODIFY | IN_DELETE_SELF);
            if (watch_descriptor < 0)
            {
                error("error: failed to create the configuration file watch: %s\n", strerror(errno));
                break;
            }
            pthread_kill(main_thread_identifier, SIGHUP);
        }
    }
    pthread_exit(NULL);
    return EXIT_SUCCESS;
}

/**
 * Starts watching for changes in the configuration file.
 * */
static int watch_configuration_file()
{
    inotify_descriptor = inotify_init();
    if (inotify_descriptor < 0)
    {
        error("error: failed to initialize inotify: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    watch_descriptor = inotify_add_watch(inotify_descriptor, configuration_file_path, IN_MODIFY | IN_DELETE_SELF);
    if (watch_descriptor < 0)
    {
        error("error: failed to create the configuration file watch: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    pthread_create(&watch_thread_identifier, NULL, read_watch_events, NULL);
    return EXIT_SUCCESS;
}

/**
 * Releases the configuration file watch.
 * */
static int release_configuration_file_watch()
{
    if (watch_thread_identifier > 0)
    {
        pthread_kill(watch_thread_identifier, SIGTERM);
        pthread_join(watch_thread_identifier, NULL);
    }
    if (watch_descriptor > 0)
    {
        log("info: releasing configuration file watch\n");
        inotify_rm_watch(inotify_descriptor, watch_descriptor);
    }
    if (inotify_descriptor > 0)
    {
        close(inotify_descriptor);
    }
    return EXIT_SUCCESS;
}

/**
 * Checks if any input devices are currently active.
 * */
static int has_active_devices()
{
    for (int i = 0; i < input_device_count; i++)
    {
        if (input_devices[i].active)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * Releases the input and output devices.
 * */
static void clean_up()
{
    release_configuration_file_watch();
    release_input();
    release_output();
}

/**
 * Main method.
 *
 * @remarks
 * read: Read NBYTES into BUF from FD. Return the number read, -1 for errors or 0 for EOF.
 * EOF doesn't make sense here. Partial events will be ignored.
 * https://docs.kernel.org/input/uinput.html
 * https://stackoverflow.com/questions/20943322/accessing-keys-from-linux-input-device
 * */
int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        error("error: invalid arguments\n");
        return EXIT_FAILURE;
    }

    main_thread_identifier = pthread_self();
    if (attach_signal_handlers() != EXIT_SUCCESS)
    {
        error("error: failed to attach signal handlers\n");
        return EXIT_FAILURE;
    }
    if (find_configuration_file() != EXIT_SUCCESS)
    {
        error("error: could not find the configuration file\n");
        return EXIT_FAILURE;
    }
    if (read_configuration() != EXIT_SUCCESS)
    {
        error("error: failed to read the configuration\n");
        return EXIT_FAILURE;
    }
    if (watch_configuration_file() != EXIT_SUCCESS)
    {
        error("error: failed to watch the configuration file\n");
        return EXIT_FAILURE;
    }
    if (bind_input() != EXIT_SUCCESS)
    {
        error("error: could not capture the input device\n");
    }
    if (bind_output() != EXIT_SUCCESS)
    {
        error("error: could not create the virtual output device\n");
        return EXIT_FAILURE;
    }
    log("info: running\n");
    // Read events from multiple devices
    struct input_event event;
    ssize_t result;
    fd_set read_fds;
    int max_fd;
    struct timeval timeout;
    
    while (1)
    {
        if (should_reload)
        {
            log("info: reloading\n");
            release_output_keys();
            release_input();
            if (read_configuration() != EXIT_SUCCESS)
            {
                error("error: failed to read the configuration\n");
                clean_up();
                return EXIT_FAILURE;
            }
            if (bind_input() != EXIT_SUCCESS)
            {
                error("error: could not capture any keyboard devices\n");
            }
            should_reload = 0;
        }
        if (should_exit)
        {
            log("info: exiting\n");
            clean_up();
            return EXIT_SUCCESS;
        }
        if (!has_active_devices())
        {
            log("info: no active input devices. You may update the configuration file to have the application attempt discovering input devices again.\n");
            sleep(UINT_MAX); // this can be interrupted
            continue;
        }

        // Prepare file descriptor set for select()
        FD_ZERO(&read_fds);
        max_fd = -1;
        
        for (int i = 0; i < input_device_count; i++)
        {
            if (input_devices[i].active && input_devices[i].file_descriptor > 0)
            {
                FD_SET(input_devices[i].file_descriptor, &read_fds);
                if (input_devices[i].file_descriptor > max_fd)
                {
                    max_fd = input_devices[i].file_descriptor;
                }
            }
        }
        
        if (max_fd == -1)
        {
            // No active devices
            continue;
        }
        
        // Set timeout for select (1 second)
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        // Wait for activity on any of the file descriptors
        int select_result = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
        
        if (select_result == -1)
        {
            if (errno == EINTR)
            {
                continue; // Interrupted by signal, continue
            }
            else
            {
                error("error: select failed: %s\n", strerror(errno));
                clean_up();
                return EXIT_FAILURE;
            }
        }
        else if (select_result == 0)
        {
            // Timeout, continue the loop to check for reload/exit signals
            continue;
        }
        
        // Check which device has data available
        for (int i = 0; i < input_device_count; i++)
        {
            if (input_devices[i].active && 
                input_devices[i].file_descriptor > 0 && 
                FD_ISSET(input_devices[i].file_descriptor, &read_fds))
            {
                result = read(input_devices[i].file_descriptor, &event, sizeof(event));
                if (result == (ssize_t)-1)
                {
                    if (errno == EINTR)
                    {
                        continue;
                    }
                    else
                    {
                        error("error: unable to read input event from device %s: %s\n", 
                              input_devices[i].name, strerror(errno));
                        // Mark device as inactive but continue with other devices
                        input_devices[i].active = 0;
                        close(input_devices[i].file_descriptor);
                        input_devices[i].file_descriptor = -1;
                        continue;
                    }
                }
                if (result == (ssize_t)0)
                {
                    error("error: received EOF while reading input events from device %s\n", 
                          input_devices[i].name);
                    // Mark device as inactive but continue with other devices
                    input_devices[i].active = 0;
                    close(input_devices[i].file_descriptor);
                    input_devices[i].file_descriptor = -1;
                    continue;
                }
                if (result != sizeof(event))
                {
                    warn("warning: partial input event received from device %s\n", 
                         input_devices[i].name);
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
    }
}
