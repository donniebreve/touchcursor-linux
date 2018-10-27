/**
 * Touch Cursor for Linux
 * Replicates the touch cursor style movement under linux (works under wayland)
 *
 * Special thanks to Thomas Bocek for the starting point for this application.
 * Special thanks to Martin Stone for the inspiration and Touch Cursor source.
 *
 * Running
 * make sure the file has the sudo execution permission, or run as sudo
 * pass the /dev/input/event# path of your keyboard
 */

#include <stdio.h>
#include <linux/input.h>
#include <linux/uinput.h>

#include "emit.h"
#include "queue.h"

// The state machine states
static enum states
{
    idle,
    hyper,
    delay,
    map
} state;

// Flag if the hyper key has been emitted
static int hyperEmitted;

/**
 * Converts input key to touch cursor key
 * To do: make this configurable
 */
static int convert(int code)
{
    switch (code)
    {
        case KEY_I: return KEY_UP;
        case KEY_J: return KEY_LEFT;
        case KEY_K: return KEY_DOWN;
        case KEY_L: return KEY_RIGHT;
        case KEY_U: return KEY_HOME;
        case KEY_O: return KEY_END;
        case KEY_P: return KEY_BACKSPACE;
        case KEY_H: return KEY_PAGEUP;
        case KEY_N: return KEY_PAGEDOWN;
        case KEY_M: return KEY_DELETE;
        default: return code;
    }
}

/**
 * Checks if the key is the hyper key.
 */
static int isHyper(int code)
{
    return code == KEY_SPACE;
}

/**
 * Checks if the event is key down.
 * Linux input sends value=2 for repeated key down.
 * We treat them as keydown events for processing.
 */
static int isDown(int value)
{
    return value == 1 || value == 2;
}

/**
 * Checks if the key has been mapped.
 */
int isMapped(int code)
{
    switch (code)
    {
        case KEY_I:
        case KEY_J:
        case KEY_K:
        case KEY_L:
        case KEY_U:
        case KEY_O:
        case KEY_P:
        case KEY_H:
        case KEY_N:
        case KEY_M:
            return 1;
        default:
            return 0;
    }
}

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
        case KEY_LEFTCTRL:
        case KEY_GRAVE:
        case KEY_LEFTSHIFT:
        case KEY_RIGHTSHIFT:
        case KEY_LEFTALT:
        case KEY_CAPSLOCK:
        case KEY_F1:
        case KEY_F2:
        case KEY_F3:
        case KEY_F4:
        case KEY_F5:
        case KEY_F6:
        case KEY_F7:
        case KEY_F8:
        case KEY_F9:
        case KEY_F10:
        case KEY_NUMLOCK:
        case KEY_SCROLLLOCK:
            return 1;
        default:
            return 0;
    }
}

/**
 * Processes a key input event. Converts and emits events as necessary.
 */
void processKey(int type, int code, int value)
{
    switch (state)
    {
        case idle: // 0
            if (isHyper(code))
            {
                if (isDown(value))
                {
                    state = hyper;
                    hyperEmitted = 0;
                    clearQueue();
                }
                else
                {
                    emit(EV_KEY, code, value);
                }
            }
            else
            {
                emit(EV_KEY, code, value);
            }
            break;

        case hyper: // 1
            if (isHyper(code))
            {
                if (!isDown(value))
                {
                    state = idle;
                    if (!hyperEmitted)
                    {
                        emit(EV_KEY, KEY_SPACE, 1);
                    }
                    emit(EV_KEY, KEY_SPACE, 0);
                }
            }
            else if (isMapped(code))
            {
                if (isDown(value))
                {
                    state = delay;
                    enqueue(code);
                }
                else
                {
                    emit(EV_KEY, code, value);
                }
            }
            else
            {
                if (isDown(value))
                {
                    if (!hyperEmitted)
                    {
                        emit(EV_KEY, KEY_SPACE, 1);
                        hyperEmitted = 1;
                    }
                    emit(EV_KEY, code, value);
                }
                else
                {
                    emit(EV_KEY, code, value);
                }
            }
            break;

        case delay: // 2
            if (isHyper(code))
            {
                if (!isDown(value))
                {
                    state = idle;
                    if (!hyperEmitted)
                    {
                        emit(EV_KEY, KEY_SPACE, 1);
                    }
                    int length = lengthOfQueue();
                    for (int i = 0; i < length; i++)
                    {
                        emit(EV_KEY, dequeue(), 1);
                    }
                    emit(EV_KEY, KEY_SPACE, 0);
                }
            }
            else if (isMapped(code))
            {
                state = map;
                if (isDown(value))
                {
                    if (lengthOfQueue() != 0)
                    {
                        emit(EV_KEY, convert(peek()), 1);
                    }
                    enqueue(code);
                    emit(EV_KEY, convert(code), value);
                }
                else
                {
                    int length = lengthOfQueue();
                    for (int i = 0; i < length; i++)
                    {
                        emit(EV_KEY, convert(dequeue()), 1);
                    }
                    emit(EV_KEY, convert(code), value);
                }
            }
            else
            {
                state = map;
                emit(EV_KEY, code, value);
            }
            break;

        case map: // 3
            if (isHyper(code))
            {
                if (!isDown(value))
                {
                    state = idle;
                    int length = lengthOfQueue();
                    for (int i = 0; i < length; i++)
                    {
                        emit(EV_KEY, convert(dequeue()), 0);
                    }
                }
            }
            else if (isMapped(code))
            {
                if (isDown(value))
                {
                    enqueue(code);
                    emit(EV_KEY, convert(code), value);
                }
                else
                {
                    emit(EV_KEY, convert(code), value);
                }
            }
            else
            {
                emit(EV_KEY, code, value);
            }
            break;
    }
}