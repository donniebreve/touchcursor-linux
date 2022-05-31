#ifndef binding_h
#define binding_h

/**
 * @brief The highest input event code to enable key events handling for.
 *
 * There used to be KEY_MAX here, but that seems to be causing issues:
 * At one point there was an issue where the virtual device could not be created
 * if keys up to KEY_MAX (767) were included. 572 came from iterating down
 * from KEY_MAX until things started working again. Not sure what the underlying
 * cause is. For further reference, see
 * https://github.com/donniebreve/touchcursor-linux/pull/39#issuecomment-1000901050.
 */
#define MAX_KEYS_TO_ENABLE_KEY_EVENTS_HANDLING_FOR 572

extern int input;

/**
 * Binds to the input device using ioctl.
 * */
int bindInput(char* fileDescriptor);

extern int output;

/**
 * Creates and binds a virtual output device using ioctl and uinput.
 * */
int bindOutput();

#endif
