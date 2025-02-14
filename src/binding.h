#ifndef binding_h
#define binding_h

#include <linux/input-event-codes.h>

/**
 * The name of the input device.
 * */
extern char input_device_name[256];
/**
 * The event path for the input device.
 * */
extern char input_event_path[256];
/**
 * The file descriptor for the input device.
 * */
extern int input_file_descriptor;

/**
 * Searches /proc/bus/input/devices for the device event.
 *
 * @param name The device name.
 * @param number The device instance number.
 */
int find_device_event_path(char* name, int number);

/**
 * Binds to the input device using ioctl.
 * */
int bind_input();

/**
 * Releases the input device.
 * */
int release_input();

/**
 * The name of the output device.
 * */
extern char output_device_name[32];
/**
 * The sys path for the output device.
 * */
extern char output_sys_path[256];
/**
 * The output device key state.
 * */
extern int output_device_keystate[KEY_MAX];
/**
 * The file descriptor for the output device.
 * */
extern int output_file_descriptor;

/**
 * Creates and binds a virtual output device using ioctl and uinput.
 * */
int bind_output();

/**
 * Releases any held keys on the output device.
 * */
void release_output_keys();

/**
 * Releases the virtual output device.
 * */
int release_output();

#endif
