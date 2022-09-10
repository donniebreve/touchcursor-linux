#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "config.h"
#include "keys.h"

char configuration_file_path[256];
char event_path[18];

int hyperKey;
int keymap[256];

int remap[256];

/**
 * Trims trailing comments from a string.
 *
 * @param s The string to be trimmed.
 * @return char* The string without any trailing comments.
 * */
char* trim_comment(char* s)
{
    if (s != NULL)
    {
        char *p  = strchr(s, '#');
        if (p != NULL)
        {
            // p points to the start of the comment.
            *p = '\0';
        }
    }
    return s;
}

/**
 * Trims a string.
 *
 * @param s The string to be trimmed.
 * @remarks
 * Credit to chux: https://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way#122721
 * */
static char* trim_string(char* s)
{
    while (isspace((unsigned char)*s)) s++;
    if (*s)
    {
        char *p = s;
        while (*p) p++;
        while (isspace((unsigned char) *(--p)));
        p[1] = '\0';
    }
    return s;
}

/**
 * Checks if a string starts with a specific substring.
 *
 * @param s The string to be inspected.
 * @param ss The substring to search for.
 */
static int starts_with(const char* s, const char* ss)
{
    return strncmp(s, ss, strlen(ss)) == 0;
}

/**
 * Checks for commented or empty lines.
 *
 * @param line The configuration file line.
 */
static int is_comment_or_empty(char* line)
{
    return line[0] == '#' || line[0] == '\0';
}


/**
 * Checks for the device number if it is configured.
 * Also removes the trailing number configuration from the input.
 */
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
 * Searches /proc/bus/input/devices for the device event.
 */
static int find_device_event(char* device_config_value)
{
    event_path[0] = '\0';
    char* device_name = device_config_value;
    int device_number = get_device_number(device_name);
    fprintf(stdout, "info: configured device_name: %s\n", device_name);
    fprintf(stdout, "info: configured device_number: %i\n", device_number);
    FILE* devices_file = fopen("/proc/bus/input/devices", "r");
    if (!devices_file)
    {
        fprintf(stderr, "error: could not open /proc/bus/input/devices\n");
        return EXIT_FAILURE;
    }
    char* line = NULL;
    int matched_name = 0;
    int matched_count = 0;
    int found_event = 0;
    size_t length = 0;
    ssize_t result;
    while (!found_event && (result = getline(&line, &length, devices_file)) != -1)
    {
        if (length < 3) continue;
        if (isspace(line[0])) continue;
        if (!matched_name)
        {
            if (!starts_with(line, "N: ")) continue;
            char* trimmed_line = trim_string(line + 3);
            if (strcmp(trimmed_line, device_name) == 0)
            {
                if (device_number == ++matched_count)
                {
                    matched_name = 1;
                }
                continue;
            }
        }
        if (matched_name)
        {
            if (!starts_with(line, "H: Handlers")) continue;
            char* tokens = line;
            char* token = strsep(&tokens, "=");
            while (tokens != NULL)
            {
                token = strsep(&tokens, " ");
                if (starts_with(token, "event"))
                {
                    strcat(event_path, "/dev/input/");
                    strcat(event_path, token);
                    fprintf(stdout, "info: found the keyboard event: %s\n", event_path);
                    found_event = 1;
                    break;
                }
            }
        }
    }
    if (!found_event)
    {
        fprintf(stderr, "error: could not find the event path for device: %s\n", device_config_value);
    }
    fclose(devices_file);
    if (line) free(line);
    return EXIT_SUCCESS;
}

static enum sections
{
    configuration_none,
    configuration_device,
    configuration_remap,
    configuration_hyper,
    configuration_bindings
} section;

/**
 * Reads the configuration file.
 * */
int read_configuration()
{
    // Find the configuration file
    configuration_file_path[0] = '\0';
    FILE* configuration_file;
    char* home_path = getenv("HOME");
    if (!home_path)
    {
        fprintf(stderr, "error: home path environment variable not specified\n");
    }
    if (home_path)
    {
        strcat(configuration_file_path, home_path);
        strcat(configuration_file_path, "/.config/touchcursor/touchcursor.conf");
        fprintf(stdout, "info: looking for the configuration file at: %s\n", configuration_file_path);
        configuration_file = fopen(configuration_file_path, "r");
    }
    if (!configuration_file)
    {
        strcpy(configuration_file_path, "/etc/touchcursor/touchcursor.conf");
        fprintf(stdout, "info: looking for the configuration file at: %s\n", configuration_file_path);
        configuration_file = fopen(configuration_file_path, "r");
    }
    if (!configuration_file)
    {
        fprintf(stderr, "error: could not open the configuration file\n");
        return EXIT_FAILURE;
    }
    fprintf(stdout, "info: found the configuration file: %s\n", configuration_file_path);
    // Parse the configuration file
    char* buffer = NULL;
    size_t length = 0;
    ssize_t result = -1;
    while ((result = getline(&buffer, &length, configuration_file)) != -1)
    {
        char* line = trim_comment(buffer);
        line = trim_string(line);
        // Comment or empty line
        if (is_comment_or_empty(line))
        {
            continue;
        }
        // Check for section
        if (strncmp(line, "[Device]", strlen(line)) == 0)
        {
            section = configuration_device;
            continue;
        }
        if (strncmp(line, "[Remap]", strlen(line)) == 0)
        {
            section = configuration_remap;
            continue;
        }
        if (strncmp(line, "[Hyper]", strlen(line)) == 0)
        {
            section = configuration_hyper;
            continue;
        }
        if (strncmp(line, "[Bindings]", strlen(line)) == 0)
        {
            section = configuration_bindings;
            continue;
        }
        // Read configurations
        switch (section)
        {
            case configuration_device:
                {
                    if (event_path[0] == '\0')
                    {
                        find_device_event(line);
                    }
                    continue;
                }
            case configuration_remap:
                {
                    char* tokens = line;
                    char* token = strsep(&tokens, "=");
                    int fromCode = convertKeyStringToCode(token);
                    token = strsep(&tokens, "=");
                    int toCode = convertKeyStringToCode(token);
                    remap[fromCode] = toCode;
                    break;
                }
            case configuration_hyper:
                {
                    char* tokens = line;
                    char* token = strsep(&tokens, "=");
                    token = strsep(&tokens, "=");
                    int code = convertKeyStringToCode(token);
                    hyperKey = code;
                    break;
                }
            case configuration_bindings:
                {
                    char* tokens = line;
                    char* token = strsep(&tokens, "=");
                    int fromCode = convertKeyStringToCode(token);
                    token = strsep(&tokens, "=");
                    int toCode = convertKeyStringToCode(token);
                    keymap[fromCode] = toCode;
                    break;
                }
            case configuration_none:
            default:
                {
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
//     fprintf(stdout, "suggestion: use any of the following in the configuration file for this application:\n");
//     struct dirent* directory = NULL;
//     while ((directory = readdir(directoryStream)))
//     {
//         if (strstr(directory->d_name, "kbd"))
//         {
//             printf ("keyboard=/dev/input/by-id/%s\n", directory->d_name);
//         }
//     }
// }
