#ifndef LIST_H
#define LIST_H

struct List {
    void* obj;
    struct List* tail;
};

struct List* list_new();
int list_push(struct List** list, void* obj);
void* list_pop(struct List** list);
struct List* list_find(struct List* list, void* obj);
int list_delete(struct List** list, void* obj);

#endif
