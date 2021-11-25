#ifndef keys_header
#define keys_header

/**
 * Checks if the event is a key down.
 */
int isDown(int value);

/**
 * Checks if the key is a modifier key.
 */
int isModifier(int code);

/**
 * Converts a key string "KEY_I" to its corresponding code.
 */
int convertKeyStringToCode(char* keyString);

#endif