#include <string.h>
#include <linux/uinput.h>

#include "keys.h"

/**
 * Checks if the key is a modifier key.
 */
int isModifier(int code)
{
    switch (code)
    {
        case KEY_ESC:
        case KEY_BACKSPACE:
        case KEY_TAB:
        case KEY_ENTER:
        case KEY_LEFTSHIFT:
        case KEY_RIGHTSHIFT:
        case KEY_LEFTCTRL:
        case KEY_RIGHTCTRL:
        case KEY_LEFTALT:
        case KEY_RIGHTALT:
        case KEY_CAPSLOCK:
        case KEY_NUMLOCK:
        case KEY_SCROLLLOCK:
            return 1;
        default:
            return 0;
    }
}

/**
 * Converts a key string (e.g. "KEY_I") to its corresponding code.
 */
int convertKeyStringToCode(char* keyString)
{
    // Special keys
    if (strcmp(keyString, "KEY_BACKSPACE") == 0) return KEY_BACKSPACE;
    else if (strcmp(keyString, "KEY_TAB") == 0) return KEY_TAB;
    else if (strcmp(keyString, "KEY_ENTER") == 0) return KEY_ENTER;
    else if (strcmp(keyString, "KEY_CAPSLOCK") == 0) return KEY_CAPSLOCK;
    else if (strcmp(keyString, "KEY_ESC") == 0) return KEY_ESC;
    else if (strcmp(keyString, "KEY_PAGEUP") == 0) return KEY_PAGEUP;
    else if (strcmp(keyString, "KEY_PAGEDOWN") == 0) return KEY_PAGEDOWN;
    else if (strcmp(keyString, "KEY_END") == 0) return KEY_END;
    else if (strcmp(keyString, "KEY_HOME") == 0) return KEY_HOME;
    else if (strcmp(keyString, "KEY_LEFT") == 0) return KEY_LEFT;
    else if (strcmp(keyString, "KEY_UP") == 0) return KEY_UP;
    else if (strcmp(keyString, "KEY_RIGHT") == 0) return KEY_RIGHT;
    else if (strcmp(keyString, "KEY_DOWN") == 0) return KEY_DOWN;
    else if (strcmp(keyString, "KEY_SPACE") == 0) return KEY_SPACE;
    else if (strcmp(keyString, "KEY_PRINT") == 0) return KEY_PRINT;
    else if (strcmp(keyString, "KEY_INSERT") == 0) return KEY_INSERT;
    else if (strcmp(keyString, "KEY_DELETE") == 0) return KEY_DELETE;
    // Numbers
    else if (strcmp(keyString, "KEY_0") == 0) return KEY_0;
    else if (strcmp(keyString, "KEY_1") == 0) return KEY_1;
    else if (strcmp(keyString, "KEY_2") == 0) return KEY_2;
    else if (strcmp(keyString, "KEY_3") == 0) return KEY_3;
    else if (strcmp(keyString, "KEY_4") == 0) return KEY_4;
    else if (strcmp(keyString, "KEY_5") == 0) return KEY_5;
    else if (strcmp(keyString, "KEY_6") == 0) return KEY_6;
    else if (strcmp(keyString, "KEY_7") == 0) return KEY_7;
    else if (strcmp(keyString, "KEY_8") == 0) return KEY_8;
    else if (strcmp(keyString, "KEY_9") == 0) return KEY_9;
    // Normal character keys
    else if (strcmp(keyString, "KEY_A") == 0) return KEY_A;
    else if (strcmp(keyString, "KEY_B") == 0) return KEY_B;
    else if (strcmp(keyString, "KEY_C") == 0) return KEY_C;
    else if (strcmp(keyString, "KEY_D") == 0) return KEY_D;
    else if (strcmp(keyString, "KEY_E") == 0) return KEY_E;
    else if (strcmp(keyString, "KEY_F") == 0) return KEY_F;
    else if (strcmp(keyString, "KEY_G") == 0) return KEY_G;
    else if (strcmp(keyString, "KEY_H") == 0) return KEY_H;
    else if (strcmp(keyString, "KEY_I") == 0) return KEY_I;
    else if (strcmp(keyString, "KEY_J") == 0) return KEY_J;
    else if (strcmp(keyString, "KEY_K") == 0) return KEY_K;
    else if (strcmp(keyString, "KEY_L") == 0) return KEY_L;
    else if (strcmp(keyString, "KEY_M") == 0) return KEY_M;
    else if (strcmp(keyString, "KEY_N") == 0) return KEY_N;
    else if (strcmp(keyString, "KEY_O") == 0) return KEY_O;
    else if (strcmp(keyString, "KEY_P") == 0) return KEY_P;
    else if (strcmp(keyString, "KEY_Q") == 0) return KEY_Q;
    else if (strcmp(keyString, "KEY_R") == 0) return KEY_R;
    else if (strcmp(keyString, "KEY_S") == 0) return KEY_S;
    else if (strcmp(keyString, "KEY_T") == 0) return KEY_T;
    else if (strcmp(keyString, "KEY_U") == 0) return KEY_U;
    else if (strcmp(keyString, "KEY_V") == 0) return KEY_V;
    else if (strcmp(keyString, "KEY_W") == 0) return KEY_W;
    else if (strcmp(keyString, "KEY_X") == 0) return KEY_X;
    else if (strcmp(keyString, "KEY_Y") == 0) return KEY_Y;
    else if (strcmp(keyString, "KEY_Z") == 0) return KEY_Z;
    // More special keys
    else if (strcmp(keyString, "KEY_LEFTMETA") == 0) return KEY_LEFTMETA;
    else if (strcmp(keyString, "KEY_RIGHTMETA") == 0) return KEY_RIGHTMETA;
    else if (strcmp(keyString, "KEY_LEFTSHIFT") == 0) return KEY_LEFTSHIFT;
    else if (strcmp(keyString, "KEY_RIGHTSHIFT") == 0) return KEY_RIGHTSHIFT;
    else if (strcmp(keyString, "KEY_LEFTCTRL") == 0) return KEY_LEFTCTRL;
    else if (strcmp(keyString, "KEY_RIGHTCTRL") == 0) return KEY_RIGHTCTRL;
    else if (strcmp(keyString, "KEY_LEFTALT") == 0) return KEY_LEFTALT;
    else if (strcmp(keyString, "KEY_RIGHTALT") == 0) return KEY_RIGHTALT;
    else if (strcmp(keyString, "KEY_GRAVE") == 0) return KEY_GRAVE;
    else if (strcmp(keyString, "KEY_F1") == 0) return KEY_F1;
    else if (strcmp(keyString, "KEY_F2") == 0) return KEY_F2;
    else if (strcmp(keyString, "KEY_F3") == 0) return KEY_F3;
    else if (strcmp(keyString, "KEY_F4") == 0) return KEY_F4;
    else if (strcmp(keyString, "KEY_F5") == 0) return KEY_F5;
    else if (strcmp(keyString, "KEY_F6") == 0) return KEY_F6;
    else if (strcmp(keyString, "KEY_F7") == 0) return KEY_F7;
    else if (strcmp(keyString, "KEY_F8") == 0) return KEY_F8;
    else if (strcmp(keyString, "KEY_F9") == 0) return KEY_F9;
    else if (strcmp(keyString, "KEY_F10") == 0) return KEY_F10;
    else if (strcmp(keyString, "KEY_F11") == 0) return KEY_F11;
    else if (strcmp(keyString, "KEY_F12") == 0) return KEY_F12;
    else if (strcmp(keyString, "KEY_F13") == 0) return KEY_F13;
    else if (strcmp(keyString, "KEY_F14") == 0) return KEY_F14;
    else if (strcmp(keyString, "KEY_F15") == 0) return KEY_F15;
    else if (strcmp(keyString, "KEY_F16") == 0) return KEY_F16;
    else if (strcmp(keyString, "KEY_F17") == 0) return KEY_F17;
    else if (strcmp(keyString, "KEY_F18") == 0) return KEY_F18;
    else if (strcmp(keyString, "KEY_F19") == 0) return KEY_F19;
    else if (strcmp(keyString, "KEY_F20") == 0) return KEY_F20;
    else if (strcmp(keyString, "KEY_F21") == 0) return KEY_F21;
    else if (strcmp(keyString, "KEY_F22") == 0) return KEY_F22;
    else if (strcmp(keyString, "KEY_F23") == 0) return KEY_F23;
    else if (strcmp(keyString, "KEY_F24") == 0) return KEY_F24;
    else if (strcmp(keyString, "KEY_NUMLOCK") == 0) return KEY_NUMLOCK;
    else if (strcmp(keyString, "KEY_SCROLLLOCK") == 0) return KEY_SCROLLLOCK;
    else return 0;
}
