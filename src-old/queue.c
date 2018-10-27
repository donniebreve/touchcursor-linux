#include "queue.h"

#define length 8

static int store[length];
static int tail = 0;
static int head = 0;

void initq()
{
    for (int i = 0; i < length; i++)
    {
        store[i] = 0;
    }
}

int enqueue(int value)
{
    for (int i = head; i != tail; i = (i + 1) % length)
    {
        if (store[i] == value)
        {
            return 1;
        }
    }
    int index = (tail + 1) % length;
    if (index == head)
    {
        return 0;
    }
    store[tail] = value;
    tail = index;
    return 1;
}

int dequeue(int* value)
{
    if (head == tail)
    {
        return 0;
    }
    *value = store[head];
    head = (head + 1) % length;
    return 1;
}

int peek(int* value)
{
    if (head == tail)
    {
        return 0;
    }
    *value = store[head];
    return 1;
}