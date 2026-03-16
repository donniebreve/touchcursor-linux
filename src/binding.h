#ifndef binding_h
#define binding_h

#include <linux/input-event-codes.h>

#define MAX_INPUT_DEVICES 32

/**
 * Structure representing an input device.
 * */
struct input_device
{
    char name[256];
    char event_path[256];
    int file_descriptor;
    int active;
};

/**
 * Array of input devices.
 * */
extern struct input_device input_devices[MAX_INPUT_DEVICES];
/**
 * Number of active input devices.
 * */
extern int input_device_count;

/**
 * Searches /proc/bus/input/devices for the device event.
 *
 * @param name The device name.
 * @param number The device instance number.
 * @param device_index The index where to store the found device.
 */
int find_device_event_path(char* name, int number, int device_index);

/**
 * Binds to a specific input device using ioctl.
 * 
 * @param device_index The index of the device to bind.
 * */
int bind_input_device(int device_index);

/**
 * Binds to all configured input devices using ioctl.
 * */
int bind_input();

/**
 * Releases a specific input device.
 * 
 * @param device_index The index of the device to release.
 * */
int release_input_device(int device_index);

/**
 * Releases all input devices.
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
