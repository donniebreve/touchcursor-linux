#ifndef strings_h
#define strings_h

/**
 * Trims trailing comments from a string.
 *
 * @param s The string to be trimmed.
 * @return char* The string without any trailing comments.
 * */
char* trim_comment(char* s);

/**
 * Trims a string.
 *
 * @param s The string to be trimmed.
 * @remarks
 * Credit to chux: https://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way#122721
 * */
char* trim_string(char* s);

/**
 * Checks if a string starts with a specific substring.
 *
 * @param s The string to be inspected.
 * @param ss The substring to search for.
 */
int starts_with(const char* s, const char* ss);

/**
 * Checks for commented or empty lines.
 *
 * @param line The configuration file line.
 */
int is_comment_or_empty(char* line);

#endif
