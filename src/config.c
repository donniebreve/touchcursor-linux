#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "config.h"
#include "keys.h"

char configFilePath[256];
char eventPath[18];
int hyperKey;
int keymap[256];

/**
 * Trims a string.
 * credit to chux: https://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way#122721
 */
char* trimString(char* s)
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
 * Checks if the string starts with a specific value.
 */
int startsWith(const char* str, const char* value)
{
    return strncmp(str, value, strlen(value)) == 0;
}

/**
 * Checks for commented or empty lines.
 */
int isCommentOrEmpty(char* line)
{
    return line [0] == '#' || line[0] == '\0';
}

/**
 * Searches /proc/bus/input/devices for the device event.
 */
void findDeviceEvent(char* deviceConfigValue)
{
    eventPath[0] = '\0';

    char* deviceName = deviceConfigValue;
    int deviceNumber = 1;
    if (strstr(deviceConfigValue, ":"))
    {
        char* tokens = deviceConfigValue;
        char* token = strsep(&tokens, ":");
        deviceName = token;
        token = strsep(&tokens, ":");
        deviceNumber = atoi(token);
    }

    char* devicesFilePath = "/proc/bus/input/devices";
    FILE* devicesFile = fopen(devicesFilePath, "r");
    if (!devicesFile)
    {
        fprintf(stdout, "error: could not open /proc/bus/input/devices\n");
        return;
    }

    char* line = NULL;
    int matchedName = 0;
    int matchedCount = 0;
    int foundEvent = 0;
    size_t length = 0;
    ssize_t result;
    while (!foundEvent && (result = getline(&line, &length, devicesFile)) != -1)
    {
        if (length < 3) continue;
        if (isspace(line[0])) continue;
        if (!matchedName)
        {
            if (!startsWith(line, "N: ")) continue;
            char* trimmedLine = trimString(line + 3);
            if (strcmp(trimmedLine, deviceName) == 0)
            {
                if (deviceNumber == ++matchedCount)
                {
                    matchedName = 1;
                }
                continue;
            }
        }
        if (matchedName)
        {
            if (!startsWith(line, "H: Handlers")) continue;
            char* tokens = line;
            char* token = strsep(&tokens, "=");
            while (tokens != NULL)
            {
                token = strsep(&tokens, " ");
                if (startsWith(token, "event"))
                {
                    strcat(eventPath, "/dev/input/");
                    strcat(eventPath, token);
                    foundEvent = 1;
                    break;
                }
            }
        }
    }

    if (!foundEvent)
    {
        fprintf(stdout, "error: could not find device: %s\n", deviceConfigValue);
    }

    fclose(devicesFile);
    if (line) free(line);
}

static enum sections
{
    none,
    device,
    hyper,
    bindings
} section;

/**
 * Reads the configuration file.
 */
void readConfiguration()
{
    configFilePath[0] = '\0';
    FILE* configFile;

    char* homePath = getenv("HOME");
    if (!homePath)
    {
        fprintf(stdout, "error: home path environment variable not specified\n");
    }
    if (homePath)
    {
        strcat(configFilePath, homePath);
        strcat(configFilePath, "/.config/touchcursor/touchcursor.conf");
        fprintf(stdout, "info: looking for the configuration file at: %s\n", configFilePath);
        configFile = fopen(configFilePath, "r");
    }
    if (!configFile)
    {
        strcpy(configFilePath, "/etc/touchcursor/touchcursor.conf");
        fprintf(stdout, "info: looking for the configuration file at: %s\n", configFilePath);
        configFile = fopen(configFilePath, "r");
    }
    if (!configFile)
    {
        fprintf(stdout, "error: could not open the configuration file\n");
        return;
    }
    fprintf(stdout, "info: found the configuration file\n");

    char* buffer = NULL;
    size_t length = 0;
    ssize_t result = -1;
    while ((result = getline(&buffer, &length, configFile)) != -1)
    {
        char* line = trimString(buffer);

        // Comment or empty line
        if (isCommentOrEmpty(line)) continue;

        // Check for section
        if (strncmp(line, "[Device]", strlen(line)) == 0)
        {
            section = device;
            continue;
        }
        if (strncmp(line, "[Hyper]", strlen(line)) == 0)
        {
            section = hyper;
            continue;
        }
        if (strncmp(line, "[Bindings]", strlen(line)) == 0)
        {
            section = bindings;
            continue;
        }

        // Read configurations
        switch (section)
        {
            case device:
            {
                if (eventPath[0] == '\0')
                {
                    findDeviceEvent(line);
                }
                continue;
            }

            case hyper:
            {
                char* tokens = line;
                char* token = strsep(&tokens, "=");
                token = strsep(&tokens, "=");
                int code = convertKeyStringToCode(token);
                hyperKey = code;
                break;
            }

            case bindings:
            {
                char* tokens = line;
                char* token = strsep(&tokens, "=");
                int fromCode = convertKeyStringToCode(token);
                token = strsep(&tokens, "=");
                int toCode = convertKeyStringToCode(token);
                keymap[fromCode] = toCode;
                break;
            }

            case none:
            default:
                continue;
        }
    }

    fclose(configFile);
    if (buffer) free(buffer);
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
