#include "queue.h"

#define length 8

static int store[length];
static int tail = 0;
static int head = 0;

void clearQueue()
{
    for (int i = 0; i < length; i++)
    {
        store[i] = 0;
    }
}

int lengthOfQueue()
{
    return ((tail + length) - head) % length;
}

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

int peek()
{
    if (head == tail)
    {
        return 0;
    }
    return store[head];
}