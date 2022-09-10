#ifndef config_h
#define config_h

/**
 * The event path for the device.
 * */
extern char event_path[18];

/**
 * The hyper key;
 * */
extern int hyperKey;

/**
 * Map for keys and their conversion.
 * */
extern int keymap[256];

/**
 * Map for permanently remapped keys.
 * */
extern int remap[256];

/**
 * Reads the configuration file.
 * */
int read_configuration();

#endif
