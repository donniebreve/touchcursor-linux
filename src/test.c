// build
// gcc -Wall src/queue.c src/keys.c src/strings.c src/binding.c src/config.c src/mapper.c src/test.c -o out/test
// run
// ./out/test

#include <linux/input.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "keys.h"
#include "strings.h"

static int tests_run = 0;
static int tests_failed = 0;

// String for the full key event output
static char output[256];

// String for the emit function output
static char emitString[8];

/*
 * Override of the emit function(s).
 */
void emit(int type, int code, int value)
{
    sprintf(emitString, "%i:%i ", code, value);
    strcat(output, emitString);
}

// Now include the mapper
#include "mapper.h"

struct test_keys
{
    int code;
    char* name;
};
static struct test_keys test_keys[] = {
    {KEY_LEFTSHIFT, "leftshift"}, // not mapped in layer

    {KEY_O, "other"}, // not mapped in layer

    {KEY_J, "m1"}, {KEY_LEFT, "layer_m1"}, // mapped
    {KEY_K, "m2"}, {KEY_DOWN, "layer_m2"}, // mapped
    {KEY_L, "m3"}, {KEY_RIGHT, "layer_m3"}, // mapped

    {KEY_S, "seq"}, {KEY_A, "lseq"}, {KEY_1, "seq1"}, {KEY_2, "seq2"}, {KEY_3, "seq3"}, {KEY_4, "seq4"}, // short and long sequences

    {KEY_R, "or1"}, {KEY_O, "or1_to"}, // other remap
    {KEY_E, "mr2"}, {KEY_K, "mr2_to"}, {KEY_5, "layer_mr2"}, // mapped remap

    {KEY_SPACE, "hyper"},

    {0, NULL}
};

/*
 * Get code for named key.
 */
static int lookup_key_code(char* name)
{
    for (int i = 0; test_keys[i].name != NULL; i++)
    {
        if (strcmp(name, test_keys[i].name) == 0) return test_keys[i].code;
    }
    return 0;
}

/*
 * Simulates typing keys.
 */
static int type(char* keys, char* expect)
{
    tests_run++;
    for (int i = 0; i < 256; i++) output[i] = 0;

    char* _keys = malloc(strlen(keys) + 1);
    strcpy(_keys, keys);
    char* tokens = _keys;
    char* token;
    while ((token = strsep(&tokens, ",")) != NULL)
    {
        token = trim_string(token);
        char* key = token;
        strsep(&token, " ");
        char* action = token;
        if (key == NULL || action == NULL)
        {
            printf("  FAIL [%s]\n    missing key or action '%s %s'\n", keys, key, action);
            free(_keys);
            return 1;
        }

        int code = lookup_key_code(key);
        if (code == 0)
        {
            printf("  FAIL [%s]\n    invalid key '%s'\n", keys, key);
            free(_keys);
            return 1;
        }

        if (strcmp(action, "down") == 0)
        {
            processKey(EV_KEY, code, 1);
        }
        else if (strcmp(action, "up") == 0)
        {
            processKey(EV_KEY, code, 0);
        }
        else if (strcmp(action, "tap") == 0)
        {
            processKey(EV_KEY, code, 1);
            processKey(EV_KEY, code, 0);
        }
        else
        {
            printf("  FAIL [%s]\n    invalid key action '%s'\n", keys, action);
            free(_keys);
            return 1;
        }
    }
    free(_keys);

    char expected_output[256];
    char* eo = expected_output;
    char* _expect = malloc(strlen(expect) + 1);
    strcpy(_expect, expect);
    tokens = _expect;
    if (*tokens == '\0')
    {
        expected_output[0] = '\0';
    }
    else while ((token = strsep(&tokens, ",")) != NULL)
    {
        token = trim_string(token);
        char* key = token;
        strsep(&token, " ");
        char* action = token;
        if (key == NULL || action == NULL)
        {
            printf("  FAIL [%s]\n    missing key or action '%s %s'\n", keys, key, action);
            free(_expect);
            return 1;
        }

        int code = lookup_key_code(key);
        if (code == 0)
        {
            printf("  FAIL [%s]\n    invalid expect key '%s'\n", keys, key);
            free(_expect);
            return 1;
        }

        if (strcmp(action, "down") == 0)
        {
            sprintf(eo, "%d:1 ", code);
        }
        else if (strcmp(action, "up") == 0)
        {
            sprintf(eo, "%d:0 ", code);
        }
        else if (strcmp(action, "tap") == 0)
        {
            sprintf(eo, "%d:1 %d:0 ", code, code);
        }
        else
        {
            printf("  FAIL [%s]\n    invalid expect action '%s'\n", keys, action);
            free(_expect);
            return 1;
        }

        eo += strlen(eo);
    }
    free(_expect);

    if (strcmp(output, expected_output) != 0)
    {
        printf("  FAIL [%s]\n    expected: [%s]\n    expected: '%s'\n      output: '%s'\n", keys, expect, expected_output, output);
        return 1;
    }
    printf("  pass [%s]\n      output: '%s'\n", keys, output);
    return 0;
}
#define TYPE(keys, ignore, expect) tests_failed += type(keys, expect)

/*
 * Get code for named key or output error message.
 */
static int lookup_key_code_with_error(char* name)
{
    int code = lookup_key_code(name);
    if (code == 0)
    {
        printf("ERROR invalid key '%s'\n", name);
    }
    return code;
}
#define KEY(name) lookup_key_code_with_error(name)

/*
 * Tests for normal (slow) typing.
 * These tests should rarely have overlapping key events.
 */
static void testNormalTyping()
{
    printf("Normal typing tests...\n");

    TYPE("hyper tap", EXPECT, "hyper tap");

    TYPE("hyper tap, hyper tap", EXPECT, "hyper tap, hyper tap");

    TYPE("other down, hyper tap, other up", EXPECT, "other down, hyper tap, other up");

    TYPE("hyper down, other tap, hyper up", EXPECT, "hyper down, other tap, hyper up");

    TYPE("m1 down, hyper tap, m1 up", EXPECT, "m1 down, hyper tap, m1 up");

    TYPE("hyper down, m1 tap, hyper up", EXPECT, "layer_m1 tap");

    TYPE("m1 down, hyper tap, m2 tap, m1 up", EXPECT, "m1 down, hyper tap, m2 tap, m1 up");

    TYPE("hyper down, seq tap, hyper up", EXPECT, "seq1 down, seq2 down, seq1 up, seq2 up");

    TYPE("hyper down, lseq tap, hyper up",
        EXPECT, "seq1 down, seq2 down, seq3 down, seq4 down, seq1 up, seq2 up, seq3 up, seq4 up");

    TYPE("or1 tap", EXPECT, "other tap");

    TYPE("hyper down, or1 tap, hyper up", EXPECT, "hyper down, other tap, hyper up");

    TYPE("mr2 tap", EXPECT, "m2 tap");

    // Key is not remapped in hyper mode
    TYPE("hyper down, mr2 tap, hyper up", EXPECT, "layer_mr2 tap");
}

/*
 * Tests for fast typing.
 * These tests should have many overlapping key events.
 */
static void testFastTyping()
{
    printf("Fast typing tests...\n");

    // The mapped key should not be converted
    TYPE("hyper down, m1 down, hyper up, m1 up", EXPECT, "hyper down, m1 down, hyper up, m1 up");

    // The mapped key should not be converted
    // This is not out of order, remember space down does not emit anything
    TYPE("m1 down, hyper down, m1 up, hyper up", EXPECT, "m1 tap, hyper tap");

    // The mapped keys should be sent converted
    // Extra up events are sent, but that does not matter
    TYPE("hyper down, m1 down, m2 down, hyper up, m1 up, m2 up", EXPECT, "layer_m1 down, layer_m2 down, layer_m1 up, layer_m2 up, m1 up, m2 up");

    // The mapped keys should be sent converted
    // Extra up events are sent, but that does not matter
    TYPE("hyper down, m1 down, m2 down, m3 down, hyper up, m1 up, m2 up, m3 up",
        EXPECT, "layer_m1 down, layer_m2 down, layer_m3 down, layer_m1 up, layer_m2 up, layer_m3 up, m1 up, m2 up, m3 up");
}

/*
 * Tests for fast typing.
 * These tests should have many overlapping key events.
 */
static void testSpecialTyping()
{
    printf("Special typing tests...\n");

    // The key should be output, hyper mode not retained
    TYPE("hyper down, leftshift tap, hyper up", EXPECT, "leftshift tap, hyper tap");
}

/*
 * Main method.
 */
int main()
{
    // default config
    hyperKey = KEY("hyper");
    keymap[KEY("m1")].sequence[0] = KEY("layer_m1");
    keymap[KEY("m2")].sequence[0] = KEY("layer_m2");
    keymap[KEY("m3")].sequence[0] = KEY("layer_m3");

    keymap[KEY("seq")].sequence[0] = KEY("seq1");
    keymap[KEY("seq")].sequence[1] = KEY("seq2");
    keymap[KEY("lseq")].sequence[0] = KEY("seq1");
    keymap[KEY("lseq")].sequence[1] = KEY("seq2");
    keymap[KEY("lseq")].sequence[2] = KEY("seq3");
    keymap[KEY("lseq")].sequence[3] = KEY("seq4");

    remap[KEY("or1")] = KEY("other");
    remap[KEY("mr2")] = KEY("m2");
    keymap[KEY("mr2")].sequence[0] = KEY("layer_mr2");

    testNormalTyping();
    testFastTyping();
    testSpecialTyping();

    printf("\nTests run: %d\n", tests_run);
    if (tests_failed > 0)
    {
        printf("*** %d tests FAILED ***\n", tests_failed);
    }
    else
    {
        printf("All tests passed!\n");
    }

    return tests_failed;
}

// Sample tests from touchcursor source

// normal (slow) typing
// CHECK((SP, up,  SP,up, 0));
// CHECK((SP, dn,  SP,up, 0));
// CHECK((SP, up,  SP,up, SP,dn, SP,up, 0));
// CHECK((x, dn,   SP,up, SP,dn, SP,up, x,dn, 0));
// CHECK((x, up,   SP,up, SP,dn, SP,up, x,dn, x,up, 0));
// CHECK((j, dn,   SP,up, SP,dn, SP,up, x,dn, x,up, j,dn, 0));
// CHECK((j, up,   SP,up, SP,dn, SP,up, x,dn, x,up, j,dn, j,up, 0));

// overlapped slightly
// resetOutput();
// CHECK((SP, dn,  0));
// CHECK((x, dn,   SP,dn, x,dn, 0));
// CHECK((SP, up,  SP,dn, x,dn, SP,up, 0));
// CHECK((x, up,   SP,dn, x,dn, SP,up, x,up, 0));

//... plus repeating spaces
// CHECK((SP, dn,  0));
// CHECK((SP, dn,  0));
// CHECK((j, dn,   0));
// CHECK((SP, dn,  0));
// CHECK((SP, up,  SP,dn, j,dn, SP,up, 0));
// CHECK((j, up,   SP,dn, j,dn, SP,up, j,up, 0));

// key ups in waitMappedDown
// CHECK((SP, dn,  0));
// CHECK((x, up,   x,up, 0));
// CHECK((j, up,   x,up, j,up, 0));
// CHECK((SP, up,  x,up, j,up, SP,dn, SP,up, 0));

// other keys in waitMappedUp
// CHECK((SP, dn,  0));
// CHECK((j, dn,   0));
// CHECK((x, up,   SP,dn, j,dn, x,up, 0));
// CHECK((x, dn,   SP,dn, j,dn, x,up, x,dn, 0));
// CHECK((j, dn,   SP,dn, j,dn, x,up, x,dn, j,dn, 0));
// CHECK((SP, up,  SP,dn, j,dn, x,up, x,dn, j,dn, SP,up, 0));

// CHECK((SP, dn,  0));
// CHECK((j, dn,   0));
// CHECK((x, dn,   SP,dn, j,dn, x,dn, 0));
// CHECK((SP, up,  SP,dn, j,dn, x,dn, SP,up, 0));

// activate mapping
// CHECK((SP, dn,  0));
// CHECK((j, dn,   0));
// CHECK((j, up,   LE,edn, LE,up, 0));
// CHECK((SP, up,  LE,edn, LE,up, 0));

// autorepeat into mapping, and out
// CHECK((SP, dn,  0));
// CHECK((j, dn,   0));
// CHECK((j, dn,   LE,edn, LE,edn, 0));
// CHECK((j, dn,   LE,edn, LE,edn, LE,edn, 0));
// CHECK((j, up,   LE,edn, LE,edn, LE,edn, LE,up, 0));
// CHECK((SP, dn,  LE,edn, LE,edn, LE,edn, LE,up, 0));
// CHECK((j, dn,   LE,edn, LE,edn, LE,edn, LE,up, LE,edn, 0));
// CHECK((SP, up,  LE,edn, LE,edn, LE,edn, LE,up, LE,edn, LE,up, 0));
// CHECK((j, dn,   LE,edn, LE,edn, LE,edn, LE,up, LE,edn, LE,up, j,dn, 0));
// CHECK((j, up,   LE,edn, LE,edn, LE,edn, LE,up, LE,edn, LE,up, j,dn, j,up, 0));

// other keys during mapping
// CHECK((SP, dn,  0));
// CHECK((j, dn,   0));
// CHECK((j, up,   LE,edn, LE,up, 0));
// CHECK((x, dn,   LE,edn, LE,up, x,dn, 0));
// CHECK((x, up,   LE,edn, LE,up, x,dn, x,up, 0));
// CHECK((j, dn,   LE,edn, LE,up, x,dn, x,up, LE,edn, 0));
// CHECK((SP, up,  LE,edn, LE,up, x,dn, x,up, LE,edn, LE,up, 0));

// check space-emmitted states
// CHECK((SP, dn,  0));
// CHECK((x, dn,   SP,dn, x,dn, 0));
// CHECK((SP, dn,  SP,dn, x,dn, 0));
// CHECK((x, dn,   SP,dn, x,dn, x,dn, 0));
// CHECK((x, up,   SP,dn, x,dn, x,dn, x,up, 0));
// CHECK((j, up,   SP,dn, x,dn, x,dn, x,up, j,up, 0));
// CHECK((j, dn,   SP,dn, x,dn, x,dn, x,up, j,up, 0));
// CHECK((j, up,   SP,dn, x,dn, x,dn, x,up, j,up, LE,edn, LE,up, 0));
// CHECK((SP, up,  SP,dn, x,dn, x,dn, x,up, j,up, LE,edn, LE,up, 0)); //XXX should this emit a space (needs mappingSpaceEmitted state)

// wmuse
// CHECK((SP, dn,  0));
// CHECK((x, dn,   SP,dn, x,dn, 0));
// CHECK((j, dn,   SP,dn, x,dn, 0));
// CHECK((SP, dn,  SP,dn, x,dn, 0));
// CHECK((SP, up,  SP,dn, x,dn, j,dn, SP,up, 0));

// CHECK((SP, dn,  0));
// CHECK((x, dn,   SP,dn, x,dn, 0));
// CHECK((j, dn,   SP,dn, x,dn, 0));
// CHECK((j, dn,   SP,dn, x,dn, LE,edn, LE,edn, 0));
// CHECK((SP, up,  SP,dn, x,dn, LE,edn, LE,edn, LE,up, 0)); //XXX should this emit a space (needs mappingSpaceEmitted state)

// CHECK((SP, dn,  0));
// CHECK((x, dn,   SP,dn, x,dn, 0));
// CHECK((j, dn,   SP,dn, x,dn, 0));
// CHECK((x, up,   SP,dn, x,dn, x,up, 0));
// CHECK((j, up,   SP,dn, x,dn, x,up, LE,edn, LE,up, 0));
// CHECK((SP, up,  SP,dn, x,dn, x,up, LE,edn, LE,up, 0)); //XXX should this emit a space (needs mappingSpaceEmitted state)

// run configure tests
// idle
// CHECK((F5, dn,  F5,dn, 0));
// CHECK((SP, dn,  F5,dn, 0));
// wmd
// CHECK((F5, dn,  F5,dn, '*',dn, 0));
// CHECK((F5, up,  F5,dn, '*',dn, F5,up, 0));
// CHECK((j, dn,   F5,dn, '*',dn, F5,up, 0));
// wmu
// CHECK((F5, dn,  F5,dn, '*',dn, F5,up, '*',dn, 0));
// CHECK((j, up,   F5,dn, '*',dn, F5,up, '*',dn, LE,edn, LE,up, 0));
// mapping
// CHECK((F5, dn,  F5,dn, '*',dn, F5,up, '*',dn, LE,edn, LE,up, '*',dn, 0));
// CHECK((SP, up,  F5,dn, '*',dn, F5,up, '*',dn, LE,edn, LE,up, '*',dn, 0));

// CHECK((SP, dn,  0));
// wmd
// CHECK((x, dn,   SP,dn, x,dn, 0));
// wmd-se
// CHECK((F5, dn,  SP,dn, x,dn, '*',dn, 0));
// CHECK((j, dn,   SP,dn, x,dn, '*',dn, 0));
// wmu-se
// CHECK((F5, dn,  SP,dn, x,dn, '*',dn, '*',dn, 0));
// CHECK((SP, up,  SP,dn, x,dn, '*',dn, '*',dn, j,dn, SP,up, 0));

// Overlapping mapped keys
// CHECK((SP, dn,  0));
// CHECK((m, dn,   0));
// CHECK((j, dn,   DEL,edn, LE,edn, 0));
// CHECK((j, up,   DEL,edn, LE,edn, LE,up, 0));
// CHECK((m, up,   DEL,edn, LE,edn, LE,up, DEL,up, 0));
// CHECK((SP, up,  DEL,edn, LE,edn, LE,up, DEL,up, 0));

// Overlapping mapped keys -- space up first.
// should release held mapped keys.  (Fixes sticky Shift bug.)
// CHECK((SP, dn,  0));
// CHECK((m, dn,   0));
// CHECK((j, dn,   DEL,edn, LE,edn, 0));
// release order is in vk code order
// CHECK((SP, up,  DEL,edn, LE,edn, LE,up, DEL,up, 0));

// mapped modifier keys
// options.keyMapping['C'] = ctrlFlag | 'C'; // ctrl+c
// CHECK((SP, dn,  0));
// CHECK((c, dn,   0));
// CHECK((c, up,   ctrl,dn, c,dn, ctrl,up, c,up, 0));
// CHECK((c, dn,   ctrl,dn, c,dn, ctrl,up, c,up, ctrl,dn, c,dn, ctrl,up, 0));
// CHECK((c, up,   ctrl,dn, c,dn, ctrl,up, c,up, ctrl,dn, c,dn, ctrl,up, c,up, 0));
// CHECK((SP, up,  ctrl,dn, c,dn, ctrl,up, c,up, ctrl,dn, c,dn, ctrl,up, c,up, 0));
// with modifier already down:
// CHECK((SP, dn,  0));
// CHECK((ctrl,dn, ctrl,dn, 0));
// CHECK((c, dn,   ctrl,dn, 0));
// CHECK((c, up,   ctrl,dn, c,dn, c,up, 0));
// CHECK((ctrl,up, ctrl,dn, c,dn, c,up, ctrl,up, 0));
// CHECK((SP,up,   ctrl,dn, c,dn, c,up, ctrl,up, 0));

// training mode
// options.trainingMode = true;
// options.beepForMistakes = false;
// CHECK((x, dn,   x,dn, 0));
// CHECK((x, up,   x,dn, x,up, 0));
// CHECK((LE, edn, x,dn, x,up, 0));
// CHECK((LE, up,  x,dn, x,up, 0));
// with modifier mapping
// CHECK((c, dn,    c,dn, 0));
// CHECK((c, up,    c,dn, c,up, 0));
// CHECK((ctrl, dn, c,dn, c,up, ctrl,dn, 0));
// CHECK((c, dn,    c,dn, c,up, ctrl,dn, 0));
// CHECK((c, up,    c,dn, c,up, ctrl,dn, 0));
// CHECK((ctrl, up, c,dn, c,up, ctrl,dn, ctrl,up, 0));

// SM.printUnusedTransitions();
