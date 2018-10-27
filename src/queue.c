#include "queue.h"

#define length 8

// The queue store
static int store[length];

// The queue indexes
static int tail = 0;
static int head = 0;

/**
 * Clears the queue.
 */
void clearQueue()
{
    for (int i = 0; i < length; i++)
    {
        store[i] = 0;
    }
}

/**
 * Returns the current length of the queue.
 */
int lengthOfQueue()
{
    return ((tail + length) - head) % length;
}

/**
 * Pushes the value on the queue, if the value does not already exist in the queue.
 */
void enqueue(int value)
{
    for (int i = head; i != tail; i = (i + 1) % length)
    {
        if (store[i] == value)
        {
            return;
        }
    }
    int index = (tail + 1) % length;
    if (index == head)
    {
        return;
    }
    store[tail] = value;
    tail = index;
}

/**
 * Removes the first value from the queue and returns it.
 */
int dequeue()
{
    if (head == tail)
    {
        return 0;
    }
    int value = store[head];
    head = (head + 1) % length;
    return value;
}

/**
 * Returns the first value in the queue without removing it.
 */
int peek()
{
    if (head == tail)
    {
        return 0;
    }
    return store[head];
}