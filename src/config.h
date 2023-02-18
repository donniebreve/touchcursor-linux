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
//const int MAX_CHORDS = 4;
#define MAX_CHORDS 4
struct mapped_keycodes {
	int codes[MAX_CHORDS];
};
extern struct mapped_keycodes keymap[256];

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
