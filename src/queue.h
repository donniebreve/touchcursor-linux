#ifndef queue_h
#define queue_h

/**
 * Clears the queue.
 * */
void clearQueue();

/**
 * Returns the current length of the queue.
 * */
int lengthOfQueue();

/**
 * Pushes the value on the queue, if the value does not already exist in the queue.
 * */
void enqueue(int value);

/**
 * Removes the first value from the queue and returns it.
 * */
int dequeue();

/**
 * Returns the first value in the queue without removing it.
 * */
int peek();

/**
 * Remove key from the queue.
 * */
void removeKeyFromQueue(int value);

#endif
