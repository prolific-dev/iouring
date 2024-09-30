#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

void list_del(List *list, void *data)
{
    Node *node = list->head;
    Node *prev = NULL;

    while (node)
    {
        if (node->data == data)
        {
            if (prev)
                prev->next = node->next;
            else
                list->head = node->next;

            free(node);
            list->size--;
            break;
        }

        prev = node;
        node = node->next;
    }
}

void list_clear(List *list)
{
    Node *node = list->head;
    Node *next = NULL;

    while (node)
    {
        next = node->next;
        free(node);
        node = next;
    }

    list->head = NULL;
    list->size = 0;
}

void list_add(List *list, void *data)
{
    Node *node = (Node *)malloc(sizeof(Node));
    node->data = data;
    node->next = list->head;
    list->head = node;
    list->size++;
}

void list_destroy(List **list)
{
    Node *node = (*list)->head;
    Node *next = NULL;

    while (node)
    {
        next = node->next;
        free(node);
        node = next;
    }

    free(*list);
    *list = NULL;
}

void list_init(List **list)
{
    *list = (List *)malloc(sizeof(List));
    (*list)->head = NULL;
    (*list)->size = 0;
}