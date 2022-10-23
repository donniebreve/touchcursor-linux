#ifndef config_h
#define config_h

/**
 * The configuration file path.
 * */
extern char configuration_file_path[256];

/**
 * The hyper key.
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
