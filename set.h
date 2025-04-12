#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

typedef struct Node {
    int value;
    struct Node *next;
} Node;

typedef struct {
    Node **buckets;
    int size;
} Set;

unsigned hash(int value, int size);

Set *createSet(int size);

int contains(Set *set, int value);

int add(Set *set, int value);

int removeValue(Set *set, int value);

Set *setUnion(Set *a, Set *b);

void printSet(Set *set);

void freeSet(Set *set);