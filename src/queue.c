#include <stdlib.h>
#include "queue.h"

Queue* queue_create(void) {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    if (q) {
        q->front = 0;
        q->rear = -1;
        q->size = 0;
    }
    return q;
}

bool queue_is_empty(Queue* q) {
    return q->size == 0;
}

bool queue_is_full(Queue* q) {
    return q->size == QUEUE_MAX_SIZE;
}

bool queue_enqueue(Queue* q, int value) {
    if (queue_is_full(q)) {
        return false;
    }
    
    q->rear = (q->rear + 1) % QUEUE_MAX_SIZE;
    q->data[q->rear] = value;
    q->size++;
    return true;
}

int queue_dequeue(Queue* q) {
    if (queue_is_empty(q)) {
        return -1;
    }
    
    int value = q->data[q->front];
    q->front = (q->front + 1) % QUEUE_MAX_SIZE;
    q->size--;
    return value;
}

int queue_peek(Queue* q) {
    if (queue_is_empty(q)) {
        return -1;
    }
    return q->data[q->front];
}

int queue_size(Queue* q) {
    return q->size;
}

void queue_free(Queue* q) {
    if (q) {
        free(q);
    }
}
