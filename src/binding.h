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

extern int input;

/**
 * Binds to the input device using ioctl.
 * */
int bind_input(char* fileDescriptor);

/**
 * Releases the input device.
 * */
int release_input(char* fileDescriptor);

extern int output;

/**
 * Creates and binds a virtual output device using ioctl and uinput.
 * */
int bind_output();

/**
 * Releases the virtual output device.
 * */
int release_output();

#endif
