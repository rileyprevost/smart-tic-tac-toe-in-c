#include "set.h"

unsigned int hash(int value, int size) {
    return (unsigned int)value % size;
}

Set* createSet(int size) {
    Set *set = malloc(sizeof(Set));
    set->buckets = calloc(size, sizeof(Node*));
    set->size = size;
    return set;
}

int contains(Set *set, int value) {
    unsigned int index = hash(value, set->size);
    Node *node = set->buckets[index];
    while (node) {
        if (node->value == value) return 1;
        node = node->next;
    }
    return 0;
}

int add(Set *set, int value) {
    if (contains(set, value)) return 0;

    unsigned int index = hash(value, set->size);
    Node *newNode = malloc(sizeof(Node));
    newNode->value = value;
    newNode->next = set->buckets[index];
    set->buckets[index] = newNode;
    return 1;
}

int removeValue(Set *set, int value) {
    unsigned int index = hash(value, set->size);
    Node *node = set->buckets[index], *prev = NULL;

    while (node) {
        if (node->value == value) {
            if (prev)
                prev->next = node->next;
            else
                set->buckets[index] = node->next;
            free(node);
            return 1;
        }
        prev = node;
        node = node->next;
    }
    return 0;
}

Set *setUnion(Set *a, Set *b) {
    int resultSize = a->size > b->size ? a->size : b->size;
    Set *result = createSet(resultSize);

    for (int i = 0; i < a->size; i++) {
        Node *node = a->buckets[i];
        while (node) {
            add(result, node->value);
            node = node->next;
        }
    }

    for (int i = 0; i < b->size; i++) {
        Node *node = b->buckets[i];
        while (node) {
            add(result, node->value);
            node = node->next;
        }
    }

    return result;
}

void printSet(Set *set) {
    for (int i = 0; i < set->size; i++) {
        Node *node = set->buckets[i];
        while (node) {
            char buf[2];
            buf[0] = node->value + '0';
            buf[1] = '\0';
            write(STDOUT_FILENO, buf, 1);
            node = node->next;
        }
    }
    write(STDOUT_FILENO, "\n\0", 1);
}

void freeSet(Set *set) {
    for (int i = 0; i < set->size; i++) {
        Node *node = set->buckets[i];
        while (node) {
            Node *temp = node;
            node = node->next;
            free(temp);
        }
    }
    free(set->buckets);
    free(set);
}