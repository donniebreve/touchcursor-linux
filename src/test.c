// build
// gcc -Wall queue.c touchcursor.c test.c  -o ../output/test

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <linux/input.h>

// minunit http://www.jera.com/techinfo/jtns/jtn002.html
#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_run_test(test) do { int result = test(); tests_run++; if (result != 0) return result; } while (0)
static int tests_run;

static char output[256];
static char emitString[8];

// Different emit function
int emit(int type, int code, int value)
{
    sprintf(emitString, "%i:%i ", code, value);
    strcat(output, emitString);
    return 0;
}

#include "touchcursor.h"

// Should take code:value pairs
static void type(int num, ...)
{
    for (int i = 0; i < 256; i++) output[i] = 0;
    va_list arguments;
    va_start(arguments, num);
    for (int i = 0; i < num; i += 2)
    {
        int code = va_arg(arguments, int);
        int value = va_arg(arguments, int);
        processKey(EV_KEY, code, value);
    }
    va_end(arguments);
}

static int testNormalTyping()
{
    // Space down, up
    char* description = "sd, su";
    char* expected    = "57:1 57:0 ";
    type(4, KEY_SPACE, 1, KEY_SPACE, 0);
    if (strcmp(expected, output) != 0) {
        printf("[%s] failed. expected: '%s', output: '%s'\n", description, expected, output);
        return 1;
    }
    else {
        printf("[%s] passed. expected: '%s', output: '%s'\n", description, expected, output);
    }

    // Space down, up, down, up
    description = "sd, su, sd, su";
    expected    = "57:1 57:0 57:1 57:0 ";
    type(8, KEY_SPACE, 1, KEY_SPACE, 0, KEY_SPACE, 1, KEY_SPACE, 0);
    if (strcmp(expected, output) != 0) {
        printf("[%s] failed. expected: '%s', output: '%s'\n", description, expected, output);
        return 1;
    }
    else {
        printf("[%s] passed. expected: '%s', output: '%s'\n", description, expected, output);
    }

    // Other down, Space down, up, Other up
    description = "od, sd, su, ou";
    expected    = "31:1 57:1 57:0 31:0 ";
    type(8, KEY_S, 1, KEY_SPACE, 1, KEY_SPACE, 0, KEY_S, 0);
    if (strcmp(expected, output) != 0) {
        printf("[%s] failed. expected: '%s', output: '%s'\n", description, expected, output);
        return 1;
    }
    else {
        printf("[%s] passed. expected: '%s', output: '%s'\n", description, expected, output);
    }

    // Space down, Other down, up, Space up
    description = "sd, od, ou, su";
    expected    = "57:1 31:1 31:0 57:0 ";
    type(8, KEY_SPACE, 1, KEY_S, 1, KEY_S, 0, KEY_SPACE, 0);
    if (strcmp(expected, output) != 0) {
        printf("[%s] failed. expected: '%s', output: '%s'\n", description, expected, output);
        return 1;
    }
    else {
        printf("[%s] passed. expected: '%s', output: '%s'\n", description, expected, output);
    }

    // Mapped down, Space down, up, Mapped up
    description = "md, sd, su, mu";
    expected    = "36:1 57:1 57:0 36:0 ";
    type(8, KEY_J, 1, KEY_SPACE, 1, KEY_SPACE, 0, KEY_J, 0);
    if (strcmp(expected, output) != 0) {
        printf("[%s] failed. expected: '%s', output: '%s'\n", description, expected, output);
        return 1;
    }
    else {
        printf("[%s] passed. expected: '%s', output: '%s'\n", description, expected, output);
    }

    // Space down, Mapped down, up, Space up
    description = "sd, md, mu, su";
    expected    = "105:1 105:0 ";
    type(8, KEY_SPACE, 1, KEY_J, 1, KEY_J, 0, KEY_SPACE, 0);
    if (strcmp(expected, output) != 0) {
        printf("[%s] failed. expected: '%s', output: '%s'\n", description, expected, output);
        return 1;
    }
    else {
        printf("[%s] passed. expected: '%s', output: '%s'\n", description, expected, output);
    }

    // Mapped down, Space down, up, Different mapped down, up, Mapped up
    description = "m1d, sd, su, m2d, m2u, m1u";
    expected    = "36:1 57:1 57:0 23:1 23:0 36:0 ";
    type(12, KEY_J, 1, KEY_SPACE, 1, KEY_SPACE, 0, KEY_I, 1, KEY_I, 0, KEY_J, 0);
    if (strcmp(expected, output) != 0) {
        printf("[%s] failed. expected: '%s', output: '%s'\n", description, expected, output);
        return 1;
    }
    else {
        printf("[%s] passed. expected: '%s', output: '%s'\n", description, expected, output);
    }

    return 0;
}

static int testFastTyping()
{
    // Space down, mapped down, space up, mapped up
    // The mapped key should not be converted
    char* description = "sd, md, su, mu";
    char* expected    = "57:1 36:1 57:0 36:0 ";
    type(8, KEY_SPACE, 1, KEY_J, 1, KEY_SPACE, 0, KEY_J, 0);
    if (strcmp(expected, output) != 0) {
        printf("[%s] failed. expected: '%s', output: '%s'\n", description, expected, output);
        return 1;
    }
    else {
        printf("[%s] passed. expected: '%s', output: '%s'\n", description, expected, output);
    }

    // Mapped down, space down, mapped up, space up
    // The mapped key should not be converted
    // This is not out of order, remember space down does not emit anything
    description = "md, sd, mu, su";
    expected    = "36:1 36:0 57:1 57:0 ";
    type(8, KEY_J, 1, KEY_SPACE, 1, KEY_J, 0, KEY_SPACE, 0);
    if (strcmp(expected, output) != 0) {
        printf("[%s] failed. expected: '%s', output: '%s'\n", description, expected, output);
        return 1;
    }
    else {
        printf("[%s] passed. expected: '%s', output: '%s'\n", description, expected, output);
    }

    // Space down, mapped1 down, mapped2 down, space up, mapped1 up, mapped2 up
    // The mapped keys should be sent converted
    // Extra up events are sent, but that does not matter
    description = "sd, m1d, m2d, su, m1u, m2u";
    expected    = "105:1 103:1 105:0 103:0 36:0 23:0 ";
    type(12, KEY_SPACE, 1, KEY_J, 1, KEY_I, 1, KEY_SPACE, 0, KEY_J, 0, KEY_I, 0);
    if (strcmp(expected, output) != 0) {
        printf("[%s] failed. expected: '%s', output: '%s'\n", description, expected, output);
        return 1;
    }
    else {
        printf("[%s] passed. expected: '%s', output: '%s'\n", description, expected, output);
    }

    return 0;
}

static int runTests()
{
    mu_run_test(testNormalTyping);
    printf("Normal typing tests passed.\n");

    mu_run_test(testFastTyping);
    printf("Fast typing tests passed.\n");

    return 0;
}

int main()
{
    int result = runTests();
    if (result != 0) { printf("Some tests failed\n"); }
    else { printf("All tests passed!\n"); }
    printf("Tests run: %d\n", tests_run);
}


    

    // struct Tester {
        // Tester() {
            // isInUnitTest = true;
            // Options oldOptions = options;
            // options = Options(Options::defaults);

            // assert(isExtendedKey(VK_LEFT));
            // assert(isExtendedKey(VK_RMENU));
            // assert(!isExtendedKey(VK_F1));
            // assert(!isExtendedKey(0));

            // normal (slow) typing
            //resetOutput();
            //CHECK((SP, up,  SP,up, 0));
            //CHECK((SP, dn,  SP,up, 0));
            //CHECK((SP, up,  SP,up, SP,dn, SP,up, 0));
            //CHECK((x, dn,   SP,up, SP,dn, SP,up, x,dn, 0));
            //CHECK((x, up,   SP,up, SP,dn, SP,up, x,dn, x,up, 0));
            //CHECK((j, dn,   SP,up, SP,dn, SP,up, x,dn, x,up, j,dn, 0));
            //CHECK((j, up,   SP,up, SP,dn, SP,up, x,dn, x,up, j,dn, j,up, 0));


            
            
            
            // // overlapped slightly
            // resetOutput();
            // CHECK((SP, dn,  0));
            // CHECK((x, dn,   SP,dn, x,dn, 0));
            // CHECK((SP, up,  SP,dn, x,dn, SP,up, 0));
            // CHECK((x, up,   SP,dn, x,dn, SP,up, x,up, 0));

            // //... plus repeating spaces
            // resetOutput();
            // CHECK((SP, dn,  0));
            // CHECK((SP, dn,  0));
            // CHECK((j, dn,   0));
            // CHECK((SP, dn,  0));
            // CHECK((SP, up,  SP,dn, j,dn, SP,up, 0));
            // CHECK((j, up,   SP,dn, j,dn, SP,up, j,up, 0));

            // // key ups in waitMappedDown
            // resetOutput();
            // CHECK((SP, dn,  0));
            // CHECK((x, up,   x,up, 0));
            // CHECK((j, up,   x,up, j,up, 0));
            // CHECK((SP, up,  x,up, j,up, SP,dn, SP,up, 0));

            // // other keys in waitMappedUp
            // resetOutput();
            // CHECK((SP, dn,  0));
            // CHECK((j, dn,   0));
            // CHECK((x, up,   SP,dn, j,dn, x,up, 0));
            // CHECK((x, dn,   SP,dn, j,dn, x,up, x,dn, 0));
            // CHECK((j, dn,   SP,dn, j,dn, x,up, x,dn, j,dn, 0));
            // CHECK((SP, up,  SP,dn, j,dn, x,up, x,dn, j,dn, SP,up, 0));

            // resetOutput();
            // CHECK((SP, dn,  0));
            // CHECK((j, dn,   0));
            // CHECK((x, dn,   SP,dn, j,dn, x,dn, 0));
            // CHECK((SP, up,  SP,dn, j,dn, x,dn, SP,up, 0));


            // // activate mapping
            // resetOutput();
            // CHECK((SP, dn,  0));
            // CHECK((j, dn,   0));
            // CHECK((j, up,   LE,edn, LE,up, 0));
            // CHECK((SP, up,  LE,edn, LE,up, 0));

            // // autorepeat into mapping, and out
            // resetOutput();
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

            // // other keys during mapping
            // resetOutput();
            // CHECK((SP, dn,  0));
            // CHECK((j, dn,   0));
            // CHECK((j, up,   LE,edn, LE,up, 0));
            // CHECK((x, dn,   LE,edn, LE,up, x,dn, 0));
            // CHECK((x, up,   LE,edn, LE,up, x,dn, x,up, 0));
            // CHECK((j, dn,   LE,edn, LE,up, x,dn, x,up, LE,edn, 0));
            // CHECK((SP, up,  LE,edn, LE,up, x,dn, x,up, LE,edn, LE,up, 0));

            // // check space-emmitted states
            // // wmdse
            // resetOutput();
            // CHECK((SP, dn,  0));
            // CHECK((x, dn,   SP,dn, x,dn, 0));
            // CHECK((SP, dn,  SP,dn, x,dn, 0));
            // CHECK((x, dn,   SP,dn, x,dn, x,dn, 0));
            // CHECK((x, up,   SP,dn, x,dn, x,dn, x,up, 0));
            // CHECK((j, up,   SP,dn, x,dn, x,dn, x,up, j,up, 0));
            // CHECK((j, dn,   SP,dn, x,dn, x,dn, x,up, j,up, 0));
            // CHECK((j, up,   SP,dn, x,dn, x,dn, x,up, j,up, LE,edn, LE,up, 0));
            // CHECK((SP, up,  SP,dn, x,dn, x,dn, x,up, j,up, LE,edn, LE,up, 0)); //XXX should this emit a space (needs mappingSpaceEmitted state)

            // // wmuse
            // resetOutput();
            // CHECK((SP, dn,  0));
            // CHECK((x, dn,   SP,dn, x,dn, 0));
            // CHECK((j, dn,   SP,dn, x,dn, 0));
            // CHECK((SP, dn,  SP,dn, x,dn, 0));
            // CHECK((SP, up,  SP,dn, x,dn, j,dn, SP,up, 0));

            // resetOutput();
            // CHECK((SP, dn,  0));
            // CHECK((x, dn,   SP,dn, x,dn, 0));
            // CHECK((j, dn,   SP,dn, x,dn, 0));
            // CHECK((j, dn,   SP,dn, x,dn, LE,edn, LE,edn, 0));
            // CHECK((SP, up,  SP,dn, x,dn, LE,edn, LE,edn, LE,up, 0)); //XXX should this emit a space (needs mappingSpaceEmitted state)

            // resetOutput();
            // CHECK((SP, dn,  0));
            // CHECK((x, dn,   SP,dn, x,dn, 0));
            // CHECK((j, dn,   SP,dn, x,dn, 0));
            // CHECK((x, up,   SP,dn, x,dn, x,up, 0));
            // CHECK((j, up,   SP,dn, x,dn, x,up, LE,edn, LE,up, 0));
            // CHECK((SP, up,  SP,dn, x,dn, x,up, LE,edn, LE,up, 0)); //XXX should this emit a space (needs mappingSpaceEmitted state)
            
            // // run configure tests
            // resetOutput();
            // //idle
            // CHECK((F5, dn,  F5,dn, 0)); 
            // CHECK((SP, dn,  F5,dn, 0)); 
            // // wmd
            // CHECK((F5, dn,  F5,dn, '*',dn, 0)); 
            // CHECK((F5, up,  F5,dn, '*',dn, F5,up, 0));
            // CHECK((j, dn,   F5,dn, '*',dn, F5,up, 0));
            // // wmu
            // CHECK((F5, dn,  F5,dn, '*',dn, F5,up, '*',dn, 0));
            // CHECK((j, up,   F5,dn, '*',dn, F5,up, '*',dn, LE,edn, LE,up, 0));
            // // mapping
            // CHECK((F5, dn,  F5,dn, '*',dn, F5,up, '*',dn, LE,edn, LE,up, '*',dn, 0));
            // CHECK((SP, up,  F5,dn, '*',dn, F5,up, '*',dn, LE,edn, LE,up, '*',dn, 0));

            // resetOutput();
            // CHECK((SP, dn,  0)); 
            // // wmd
            // CHECK((x, dn,   SP,dn, x,dn, 0));
            // // wmd-se
            // CHECK((F5, dn,  SP,dn, x,dn, '*',dn, 0));
            // CHECK((j, dn,   SP,dn, x,dn, '*',dn, 0));
            // // wmu-se
            // CHECK((F5, dn,  SP,dn, x,dn, '*',dn, '*',dn, 0));
            // CHECK((SP, up,  SP,dn, x,dn, '*',dn, '*',dn, j,dn, SP,up, 0));

            // // Overlapping mapped keys
            // resetOutput();
            // CHECK((SP, dn,  0));
            // CHECK((m, dn,   0));
            // CHECK((j, dn,   DEL,edn, LE,edn, 0));
            // CHECK((j, up,   DEL,edn, LE,edn, LE,up, 0));
            // CHECK((m, up,   DEL,edn, LE,edn, LE,up, DEL,up, 0));
            // CHECK((SP, up,  DEL,edn, LE,edn, LE,up, DEL,up, 0));

            // // Overlapping mapped keys -- space up first.
            // // should release held mapped keys.  (Fixes sticky Shift bug.)
            // resetOutput();
            // CHECK((SP, dn,  0));
            // CHECK((m, dn,   0));
            // CHECK((j, dn,   DEL,edn, LE,edn, 0));
            // // release order is in vk code order
            // CHECK((SP, up,  DEL,edn, LE,edn, LE,up, DEL,up, 0));

            // // mapped modifier keys
            // options.keyMapping['C'] = ctrlFlag | 'C'; // ctrl+c
            // resetOutput();
            // CHECK((SP, dn,  0));
            // CHECK((c, dn,   0));
            // CHECK((c, up,   ctrl,dn, c,dn, ctrl,up, c,up, 0));
            // CHECK((c, dn,   ctrl,dn, c,dn, ctrl,up, c,up, ctrl,dn, c,dn, ctrl,up, 0));
            // CHECK((c, up,   ctrl,dn, c,dn, ctrl,up, c,up, ctrl,dn, c,dn, ctrl,up, c,up, 0));
            // CHECK((SP, up,  ctrl,dn, c,dn, ctrl,up, c,up, ctrl,dn, c,dn, ctrl,up, c,up, 0));
            // // with modifier already down:
            // resetOutput();
            // CHECK((SP, dn,  0));
            // CHECK((ctrl,dn, ctrl,dn, 0));
            // CHECK((c, dn,   ctrl,dn, 0));
            // CHECK((c, up,   ctrl,dn, c,dn, c,up, 0));
            // CHECK((ctrl,up, ctrl,dn, c,dn, c,up, ctrl,up, 0));
            // CHECK((SP,up,   ctrl,dn, c,dn, c,up, ctrl,up, 0));

            // // training mode
            // options.trainingMode = true;
            // options.beepForMistakes = false;
            // resetOutput();
            // CHECK((x, dn,   x,dn, 0));
            // CHECK((x, up,   x,dn, x,up, 0));
            // CHECK((LE, edn, x,dn, x,up, 0));
            // CHECK((LE, up,  x,dn, x,up, 0));
            // // with modifier mapping
            // resetOutput();
            // CHECK((c, dn,    c,dn, 0));
            // CHECK((c, up,    c,dn, c,up, 0));
            // CHECK((ctrl, dn, c,dn, c,up, ctrl,dn, 0));
            // CHECK((c, dn,    c,dn, c,up, ctrl,dn, 0));
            // CHECK((c, up,    c,dn, c,up, ctrl,dn, 0));
            // CHECK((ctrl, up, c,dn, c,up, ctrl,dn, ctrl,up, 0));

            // SM.printUnusedTransitions();
            // if (failures) exit(failures);
            // else OutputDebugString(L"Unit tests passed\n");

            // options = oldOptions;
            // isInUnitTest = false;
        // }
    // };

    // Tester tester;
// }