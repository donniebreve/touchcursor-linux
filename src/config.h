#ifndef config_header
#define config_header

/**
 * The event path for the device.
 */
extern char eventPath[18];

/**
 * Map for keys and their conversion.
 */
extern int keymap[256];

/**
 * Reads the configuration file.
 */
void readConfiguration();

#endif