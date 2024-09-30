#define _GNU_SOURCE
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "binding.h"
#include "buffers.h"
#include "config.h"
#include "keys.h"
#include "strings.h"

char configuration_file_path[256];

int hyperKey;
struct key_output keymap[MAX_KEYMAP] = { 0 };
int remap[MAX_KEYMAP] = { 0 };

/**
 * Checks for the device number if it is configured.
 * Also removes the trailing number configuration from the input.
 * */
static int get_device_number(char* device_config_value)
{
    int device_number = 1;
    int length = strlen(device_config_value);
    for (int i = length - 1; i >= 0; i--)
    {
        if (device_config_value[i] == '\0') break;
        if (device_config_value[i] == '"') break;
        if (device_config_value[i] == ':')
        {
            device_number = atoi(device_config_value + i + 1);
            device_config_value[i] = '\0';
        }
    }
    return device_number;
}

/**
 * Checks if a file exists.
 * */
static int file_exists(const char* const path)
{
    int result = access(path, F_OK);
    if (result < 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/**
 * Finds the configuration file location.
 * */
int find_configuration_file()
{
    configuration_file_path[0] = '\0';
    char* home_path = getenv("HOME");
    if (!home_path)
    {
        error("error: home path environment variable not specified\n");
    }
    else
    {
        strcat(configuration_file_path, home_path);
        strcat(configuration_file_path, "/.config/touchcursor/touchcursor.conf");
    }
    if (!file_exists(configuration_file_path))
    {
        strcpy(configuration_file_path, "/etc/touchcursor/touchcursor.conf");
    }
    if (file_exists(configuration_file_path))
    {
        log("info: found the configuration file: %s\n", configuration_file_path);
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

enum sections
{
    configuration_none,
    configuration_device,
    configuration_remap,
    configuration_hyper,
    configuration_bindings,
    configuration_invalid
};

/**
 * Reads the configuration file.
 * */
int read_configuration()
{
    // Clear existing hyper key
    hyperKey = 0;

    // Zero the existing arrays
    memset(keymap, 0, sizeof(keymap));
    memset(remap, 0, sizeof(remap));

    // Open the configuration file
    FILE* configuration_file = fopen(configuration_file_path, "r");
    if (!configuration_file)
    {
        error("error: could not open the configuration file\n");
        return EXIT_FAILURE;
    }
    // Parse the configuration file
    char* buffer = NULL;
    size_t length = 0;
    ssize_t result = -1;
    int lineno = 0;
    enum sections section = configuration_none;
    while ((result = getline(&buffer, &length, configuration_file)) != -1)
    {
        lineno++;
        char* line = trim_comment(buffer);
        line = trim_string(line);
        // Comment or empty line
        if (is_comment_or_empty(line))
        {
            continue;
        }
        // Check for section
        if (line[0] == '[')
        {
            size_t line_length = strlen(line);
            if (strncmp(line, "[Device]", line_length) == 0)
            {
                section = configuration_device;
                continue;
            }
            if (strncmp(line, "[Remap]", line_length) == 0)
            {
                section = configuration_remap;
                continue;
            }
            if (strncmp(line, "[Hyper]", line_length) == 0)
            {
                section = configuration_hyper;
                continue;
            }
            if (strncmp(line, "[Bindings]", line_length) == 0)
            {
                section = configuration_bindings;
                continue;
            }
            error("error[%d]: invalid section: %s\n", lineno, line);
            section = configuration_invalid;
            continue;
        }
        // Read configurations
        switch (section)
        {
            case configuration_device:
            {
                char* name = line;
                int number = get_device_number(name);
                if (find_device_event_path(name, number) == EXIT_SUCCESS)
                {
                    section = configuration_none;
                }
                break;
            }
            case configuration_remap:
            {
                char* tokens = line;
                char* token = strsep(&tokens, "=");
                int fromCode = convertKeyStringToCode(token);
                if (fromCode == 0)
                {
                    error("error[%d]: invalid key: expected a single key: %s\n", lineno, token);
                    continue;
                }
                if (fromCode > MAX_KEYMAP_CODE)
                {
                    error("error[%d]: left key code must be less than %d: %s\n", lineno, MAX_KEYMAP, token);
                    continue;
                }
                token = strsep(&tokens, "=");
                int toCode = convertKeyStringToCode(token);
                if (toCode == 0)
                {
                    error("error[%d]: invalid key: expected a single key: %s\n", lineno, token);
                    continue;
                }
                remap[fromCode] = toCode;
                break;
            }
            case configuration_hyper:
            {
                char* tokens = line;
                char* token = strsep(&tokens, "=");
                token = strsep(&tokens, "=");
                int code = convertKeyStringToCode(token);
                if (code == 0)
                {
                    error("error[%d]: invalid key: expected a single key: %s\n", lineno, token);
                    continue;
                }
                if (hyperKey != 0)
                {
                    error("error[%d]: hyper key set multiple times\n", lineno);
                    continue;
                }
                hyperKey = code;
                break;
            }
            case configuration_bindings:
            {
                char* tokens = line;
                char* token = strsep(&tokens, "=");
                int fromCode = convertKeyStringToCode(token);
                if (fromCode == 0)
                {
                    error("error[%d]: invalid key: expected a single key: %s\n", lineno, token);
                    continue;
                }
                if (fromCode > MAX_KEYMAP_CODE)
                {
                    error("error[%d]: left key code must be less than %d: %s\n", lineno, MAX_KEYMAP, token);
                    continue;
                }
                int index = 0;
                while ((token = strsep(&tokens, ",")) != NULL)
                {
                    if (index >= MAX_SEQUENCE)
                    {
                        error("error[%d]: exceeded limit of %d keys in sequence: %s\n", lineno, MAX_SEQUENCE, token);
                        continue;
                    }
                    int toCode = convertKeyStringToCode(token);
                    if (toCode == 0)
                    {
                        error("error[%d]: invalid key: expected a single key or comma separated list of keys: %s\n", lineno, token);
                        continue;
                    }
                    keymap[fromCode].sequence[index++] = toCode;
                }
                break;
            }
            case configuration_invalid:
            {
                error("error[%d]: ignoring line in invalid section: %s\n", lineno, line);
                continue;
            }
            case configuration_none:
            {
                error("error[%d]: ignoring line not in a section: %s\n", lineno, line);
                continue;
            }
        }
    }
    fclose(configuration_file);
    if (buffer)
    {
        free(buffer);
    }
    return EXIT_SUCCESS;
}

/**
 * Helper method to print existing keyboard devices.
 * Does not work for bluetooth keyboards.
 */
// Need to revisit this
// void printKeyboardDevices()
// {
//     DIR* directoryStream = opendir("/dev/input/");
//     if (!directoryStream)
//     {
//         printf("error: could not open /dev/input/\n");
//         return; //EXIT_FAILURE;
//     }
//     log("suggestion: use any of the following in the configuration file for this application:\n");
//     struct dirent* directory = NULL;
//     while ((directory = readdir(directoryStream)))
//     {
//         if (strstr(directory->d_name, "kbd"))
//         {
//             printf ("keyboard=/dev/input/by-id/%s\n", directory->d_name);
//         }
//     }
// }
