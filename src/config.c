#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

char* keyboardDevice;

/**
 * Trims a string.
 * credit to chux: https://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way#122721
 */
char* trimString(char *s)
{
    while (isspace((unsigned char) *s)) s++;
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
 * Checks for header or commented lines.
 */
int isCommentHeaderOrEmpty(char* line)
{
    return line[0] == '[' || line [0] == '#' || line[0] == '\0';
}

/**
 * Reads the configuration file.
 */
void readConfiguration()
{
    char* homePath;
    if (!(homePath = getenv("HOME")))
    {
        homePath = getpwuid(getuid())->pw_dir;
    }

    char configFilePath[256];
    configFilePath[0] = '\0';
    strcat(configFilePath, homePath);
    strcat(configFilePath, "/.config/touchcursor/touchcursor.config");
    FILE* configFile = fopen(configFilePath, "r");
    if (!configFile)
    {
        fprintf(stderr, "error: could not open the configuration file at: ~/.config/touchcursor/touchcursor.config\n");
        return;
    }

    char* line = NULL;
    size_t length = 0;
    ssize_t result;
    while ((result = getline(&line, &length, configFile)) != -1)
    {
        if (isCommentHeaderOrEmpty(line)) continue;
        // Split the string
        char* token = strsep(&line, "=");
        if (strcmp(token, "keyboard") == 0)
        {
            token = strsep(&line, "=");
            if (token != NULL && strcmp(token, "") != 0)
            {
                keyboardDevice = trimString(token);
            }
        }
    }

    fclose(configFile);
    if (line) free(line);
}