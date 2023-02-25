#define _GNU_SOURCE
#include <ctype.h>
#include <string.h>

#include "strings.h"

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
        char* p = strchr(s, '#');
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
char* trim_string(char* s)
{
    while (isspace((unsigned char)*s)) s++;
    if (*s)
    {
        char* p = s;
        while (*p) p++;
        while (isspace((unsigned char)*(--p)));
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
int starts_with(const char* s, const char* ss)
{
    return strncmp(s, ss, strlen(ss)) == 0;
}

/**
 * Checks for commented or empty lines.
 *
 * @param line The configuration file line.
 */
int is_comment_or_empty(char* line)
{
    return line[0] == '#' || line[0] == '\0';
}
