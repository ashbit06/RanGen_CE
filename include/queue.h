#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>
#include <stdbool.h>

#define QUEUE_MAX_SIZE 256

typedef struct {
    int data[QUEUE_MAX_SIZE];
    int front;
    int rear;
    int size;
} Queue;

/**
 * Initialize an empty queue
 */
Queue* queue_create(void);

/**
 * Check if queue is empty
 */
bool queue_is_empty(Queue* q);

/**
 * Check if queue is full
 */
bool queue_is_full(Queue* q);

/**
 * Add element to the rear of queue
 * Returns true if successful, false if queue is full
 */
bool queue_enqueue(Queue* q, int value);

/**
 * Remove and return element from front of queue
 * Returns the value, or -1 if queue is empty
 */
int queue_dequeue(Queue* q);

/**
 * Peek at front element without removing it
 * Returns the value, or -1 if queue is empty
 */
int queue_peek(Queue* q);

/**
 * Get current size of queue
 */
int queue_size(Queue* q);

/**
 * Free queue memory
 */
void queue_free(Queue* q);

#endif // QUEUE_H
