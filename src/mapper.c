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
 * Converts input key to touch cursor key
 * */
static int convert(int code)
{
    return keymap[code];
}

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
    return keymap[code] != 0;
}

/**
 * Checks if the key has been permanently mapped.
 * */
static int isRemapped(int code)
{
    return remap[code] != 0;
}

/**
 * Processes a key input event. Converts and emits events as necessary.
 * */
void processKey(int type, int code, int value)
{
    // printf("processKey: code=%i value=%i state=%i\n", code, value, state);
    if (isRemapped(code))
    {
        code = remap[code];
    }
    switch (state)
    {
        case idle: // 0
            {
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
                            emit(EV_KEY, hyperKey, 1);
                        }
                        emit(EV_KEY, hyperKey, 0);
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
                    if (!isModifier(code) && isDown(value))
                    {
                        if (!hyperEmitted)
                        {
                            emit(EV_KEY, hyperKey, 1);
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
                            emit(EV_KEY, hyperKey, 1);
                        }
                        int length = lengthOfQueue();
                        for (int i = 0; i < length; i++)
                        {
                            emit(EV_KEY, dequeue(), 1);
                        }
                        emit(EV_KEY, hyperKey, 0);
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
}
