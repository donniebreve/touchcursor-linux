#include <stdio.h>
#include <linux/input.h>
#include <linux/uinput.h>

#include "queue.h"
#include "keys.h"
#include "config.h"
#include "emit.h"
#include "mapper.h"

// The state machine state
enum states state = idle;

// Flag if the hyper key has been emitted
static int hyperEmitted;

/**
 * Checks if the key is the hyper key.
 * */
static int isHyper(int code)
{
    return code == hyperKey;
}

/**
 * Checks if the key has been mapped.
 * */
static int isMapped(int code)
{
    return keymap[code].codes[0] != 0;
}

/**
 * Converts the input code to the mapped code.
 * */
static struct mapped_keycodes getMapped(int code)
{
    return keymap[code];
}

/**
 * Converts the input code to the remapped code.
 * */
static int getRemapped(int code)
{
    if (remap[code] != 0)
    {
        return remap[code];
    }
    return code;
}

/**
 * Processes a key input event. Converts and emits events as necessary.
 * */
void processKey(int type, int code, int value)
{
    /* printf("processKey(in): code=%i value=%i state=%i\n", code, value, state); */
    switch (state)
    {
        case idle: // 0
            {
                if (isHyper(code) && isDown(value))
                {
                    state = hyper;
                    hyperEmitted = 0;
                    clearQueue();
                }
                else
                {
                    emit(EV_KEY, getRemapped(code), value);
                }
                break;
            }
        case hyper: // 1
            {
                if (isHyper(code))
                {
                    if (!isDown(value))
                    {
                        state = idle;
                        if (!hyperEmitted)
                        {
                            emit(EV_KEY, getRemapped(code), 1);
                        }
                        emit(EV_KEY, getRemapped(code), 0);
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
                        emit(EV_KEY, getRemapped(code), value);
                    }
                }
                else
                {
                    if (!isModifier(code) && isDown(value))
                    {
                        if (!hyperEmitted)
                        {
                            emit(EV_KEY, getRemapped(hyperKey), 1);
                            hyperEmitted = 1;
                        }
                        emit(EV_KEY, getRemapped(code), value);
                    }
                    else
                    {
                        emit(EV_KEY, getRemapped(code), value);
                    }
                }
                break;
            }
        case delay: // 2
            {
                if (isHyper(code))
                {
                    if (!isDown(value))
                    {
                        state = idle;
                        if (!hyperEmitted)
                        {
                            emit(EV_KEY, getRemapped(hyperKey), 1);
                        }
                        int length = lengthOfQueue();
                        for (int i = 0; i < length; i++)
                        {
                            emit(EV_KEY, getRemapped(dequeue()), 1);
                        }
                        emit(EV_KEY, getRemapped(hyperKey), 0);
                    }
                }
                else if (isMapped(code))
                {
                    state = map;
                    if (isDown(value))
                    {
                        if (lengthOfQueue() != 0)
                        {
                            emit_codes(EV_KEY, getMapped(peek()), 1);
                        }
                        enqueue(code);
                        emit_codes(EV_KEY, getMapped(code), value);
                    }
                    else
                    {
                        int length = lengthOfQueue();
                        for (int i = 0; i < length; i++)
                        {
                            emit_codes(EV_KEY, getMapped(dequeue()), 1);
                        }
                        emit_codes(EV_KEY, getMapped(code), value);
                    }
                }
                else
                {
                    state = map;
                    emit(EV_KEY, getRemapped(code), value);
                }
                break;
            }
        case map: // 3
            {
                if (isHyper(code))
                {
                    if (!isDown(value))
                    {
                        state = idle;
                        int length = lengthOfQueue();
                        for (int i = 0; i < length; i++)
                        {
                            emit_codes(EV_KEY, getMapped(dequeue()), 0);
                        }
                    }
                }
                else if (isMapped(code))
                {
                    if (isDown(value))
                    {
                        enqueue(code);
                        emit_codes(EV_KEY, getMapped(code), value);
                    }
                    else
                    {
                        emit_codes(EV_KEY, getMapped(code), value);
                    }
                }
                else
                {
                    emit(EV_KEY, getRemapped(code), value);
                }
                break;
            }
    }
    /* printf("processKey(out): state=%i\n", state); */
}
