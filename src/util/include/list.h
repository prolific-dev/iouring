#ifndef __LIST_H__
#define __LIST_H__

typedef struct Node
{
    void  *data;
    struct Node *next;
} Node;

typedef struct List
{
    Node *head;
    int   size;
} List;

void list_init   (List **list);
void list_destroy(List **list);

void list_add(List *list, void *data);
void list_del(List *list, void *data);

void list_clear(List *list);

#endif
