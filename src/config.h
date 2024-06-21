#ifndef config_h
#define config_h

#define MAX_SEQUENCE 4

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
struct key_output
{
    int sequence[MAX_SEQUENCE];
};
extern struct key_output keymap[256];

/**
 * Map for permanently remapped keys.
 * */
extern int remap[256];

/**
 * Finds the configuration file location.
 * */
int find_configuration_file();

/**
 * Reads the configuration file.
 * */
int read_configuration();

#endif
