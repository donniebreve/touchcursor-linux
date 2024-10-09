#include <linux/input.h>
#include <linux/uinput.h>
#include <stdio.h>

#include "config.h"
#include "emit.h"
#include "keys.h"
#include "mapper.h"
#include "queue.h"

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
    return keymap[code].sequence[0] != 0;
}

/**
 * Sends a mapped key sequence.
 * */
static void send_mapped_key(int code, int value)
{
    struct key_output output = keymap[code];
    for (int i = 0; i < MAX_SEQUENCE; i++)
    {
        if (output.sequence[i] == 0)
        {
            break;
        }
        emit(EV_KEY, output.sequence[i], value);
    }
    if (value == 0)
    {
        removeKeyFromQueue(code);
    }
}

/**
 * Sends all keys in the queue.
 * */
static void send_mapped_queue(int value)
{
    int length = lengthOfQueue();
    for (int i = 0; i < length; i++)
    {
        send_mapped_key(dequeue(), value);
    }
}

/**
 * Sends a remapped key.
 * */
static void send_remapped_key(int code, int value)
{
    if (remap[code] != 0)
    {
        code = remap[code];
    }
    emit(EV_KEY, code, value);
    if (value == 0)
    {
        removeKeyFromQueue(code);
    }
}

/**
 * Sends all keys in the queue.
 * */
static void send_remapped_queue(int value)
{
    int length = lengthOfQueue();
    for (int i = 0; i < length; i++)
    {
        send_remapped_key(dequeue(), value);
    }
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
                send_remapped_key(code, value);
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
                        send_remapped_key(code, 1);
                    }
                    send_remapped_key(code, 0);
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
                    send_remapped_key(code, value);
                }
            }
            else
            {
                if (!isModifier(code) && isDown(value))
                {
                    if (!hyperEmitted)
                    {
                        send_remapped_key(hyperKey, 1);
                        hyperEmitted = 1;
                    }
                }
                send_remapped_key(code, value);
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
                        send_remapped_key(hyperKey, 1);
                    }
                    send_remapped_queue(1);
                    send_remapped_key(hyperKey, 0);
                }
            }
            else if (isMapped(code))
            {
                state = map;
                if (isDown(value))
                {
                    if (lengthOfQueue() != 0)
                    {
                        send_mapped_key(peek(), 1);
                    }
                    enqueue(code);
                    send_mapped_key(code, value);
                }
                else
                {
                    send_mapped_queue(1);
                    send_mapped_key(code, value);
                }
            }
            else
            {
                state = map;
                send_remapped_key(code, value);
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
                    send_mapped_queue(0);
                }
            }
            else if (isMapped(code))
            {
                if (isDown(value))
                {
                    enqueue(code);
                }
                send_mapped_key(code, value);
            }
            else
            {
                send_remapped_key(code, value);
            }
            break;
        }
    }
    /* printf("processKey(out): state=%i\n", state); */
}
