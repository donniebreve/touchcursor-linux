#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <dirent.h>

char eventPath[18];

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
 * Checks for header or commented lines.
 */
int isCommentHeaderOrEmpty(char* line)
{
    return line[0] == '[' || line [0] == '#' || line[0] == '\0';
}

/**
 * Searches /proc/bus/input/devices for the device event.
 */
void findDeviceEvent(char* deviceName)
{
    eventPath[0] = '\0';

    char* devicesFilePath = "/proc/bus/input/devices";
    FILE* devicesFile = fopen(devicesFilePath, "r");
    if (!devicesFile)
    {
        fprintf(stderr, "error: could not open /proc/bus/input/devices\n");
        return;
    }

    char* line = NULL;
    int matchedName = 0;
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
                matchedName = 1;
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

    fclose(devicesFile);
    if (line) free(line);
}

/**
 * Reads the configuration file.
 */
void readConfiguration()
{
    char* configFilePath = "/etc/touchcursor/touchcursor.conf";
    FILE* configFile = fopen(configFilePath, "r");
    if (!configFile)
    {
        fprintf(stderr, "error: could not open the configuration file at: %s\n", configFilePath);
        return;
    }

    char* line = NULL;
    size_t length = 0;
    ssize_t result;
    while ((result = getline(&line, &length, configFile)) != -1)
    {
        if (isCommentHeaderOrEmpty(line)) continue;
        // Look for the device name
        if (startsWith(line, "Name="))
        {
            printf("found device name in config\n");
            findDeviceEvent(trimString(line));
        }
    }

    fclose(configFile);
    if (line) free(line);
}

/**
 * Helper method to print existing keyboard devices.
 * Does not work for bluetooth keyboards.
 */
void printKeyboardDevices()
{
    DIR* directoryStream = opendir("/dev/input/");
    if (!directoryStream)
    {
        printf("error: could not open /dev/input/\n");
        return; //EXIT_FAILURE;
    }
    fprintf(stderr, "suggestion: use any of the following in the configuration file for this application:\n");
    struct dirent* directory = NULL;
    while ((directory = readdir(directoryStream)))
    {
        if (strstr(directory->d_name, "kbd"))
        {
            printf ("keyboard=/dev/input/by-id/%s\n", directory->d_name);
        }
    }
}
