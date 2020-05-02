#include <stdlib.h>

#include "list.h"

struct List* list_new() {
    struct List* ret;
    if ((ret = malloc(sizeof(*ret)))) {
        ret->obj = NULL;
        ret->tail = NULL;
    }
    return ret;
}

int list_push(struct List** list, void* obj) {
    struct List* newHead;
    if ((newHead = list_new())) {
        newHead->obj = obj;
        newHead->tail = *list;
        *list = newHead;
        return 1;
    }
    return 0;
}

void* list_pop(struct List** list) {
    void* ret = NULL;
    struct List* tmp;
    if (*list) {
        tmp = *list;
        ret = tmp->obj;
        *list = tmp->tail;
        free(tmp);
    }
    return ret;
}

struct List* list_find(struct List* list, void* obj) {
    struct List* cur = list;
    while (cur && cur->obj != obj) {
        cur = cur->tail;
    }
    return cur;
}

int list_delete(struct List** list, void* obj) {
    struct List* prev = NULL;
    struct List* cur = *list;
    while (cur && cur->obj != obj) {
        prev = cur;
        cur = cur->tail;
    }
    if (!cur) return 0;
    if (!prev) {
        list_pop(list);
        return 1;
    }
    prev->tail = cur->tail;
    free(cur);
    return 1;
}

