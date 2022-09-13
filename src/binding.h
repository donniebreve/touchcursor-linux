#ifndef binding_h
#define binding_h

/**
 * @brief The upper limit for enabling key events.
 *
 * There used to be KEY_MAX here, but that seems to be causing issues:
 * At one point there was an issue where the virtual device could not be created
 * if keys up to KEY_MAX (767) were included. 572 came from iterating down
 * from KEY_MAX until things started working again. Not sure what the underlying
 * cause is. For further reference, see
 * https://github.com/donniebreve/touchcursor-linux/pull/39#issuecomment-1000901050.
 */
#define MAX_KEYBIT 572

/**
 * The name of the input device.
 * */
extern char input_device_name[256];
/**
 * The event path for the input device.
 * */
extern char input_event_path[32];
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
extern char output_sys_path[32];
/**
 * The file descriptor for the output device.
 * */
extern int output_file_descriptor;

/**
 * Creates and binds a virtual output device using ioctl and uinput.
 * */
int bind_output();

/**
 * Releases the virtual output device.
 * */
int release_output();

#endif
