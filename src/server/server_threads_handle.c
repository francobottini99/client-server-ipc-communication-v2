#include "server_threads_handle.h"

/**
 * @brief Handle thread node
 * 
 */
struct th_node
{
    pthread_t tid;          // Thread id
    struct th_node *next;   // Next handle thread
};

// First handle thread
struct th_node *first_handler = NULL;

pthread_t* handler_create(void)
{
    struct th_node* last_th = handler_get_last();
    struct th_node* new = calloc(1, sizeof(struct th_node));

    new->next = NULL;
    
    if(!last_th)
        first_handler = new;
    else
        last_th->next = new;

    return &new->tid;
}

void handler_destroy(pthread_t tid)
{
    if (!first_handler)
        return;
    
    struct th_node *th = handler_get_by_tid(tid);

    if (!th)
        return;

    struct th_node *parent = handler_get_parent(th);

    if(!parent)
        first_handler = th->next;
    else if (!th->next)
        parent->next = NULL;  
    else 
        parent->next = th->next;

    free(th);
}

void handler_destroy_all(void)
{
    struct th_node* it = first_handler;

    while (it)
    {
        struct th_node* aux = it;
        it = it->next;
        free(aux);
    }
}

struct th_node* handler_get_last(void) 
{
    struct th_node* last_th = first_handler;

    if(!last_th)
        return NULL;
    
    while (last_th->next)
        last_th = last_th->next;

    return last_th;
}

struct th_node* handler_get_by_tid(pthread_t tid)
{
    for (struct th_node* th = first_handler; th; th = th->next) 
        if (th->tid == tid) 
            return th;

    return NULL;
}

struct th_node* handler_get_parent(struct th_node* th)
{
    struct th_node *parent = first_handler;

    if(!parent || th == parent)
        return NULL;

    while (parent->next != th)
        parent = parent->next;
    
    return parent;
}

void handler_wait_all(void)
{
    for (struct th_node* th = first_handler; th; th = th->next)
        pthread_join(th->tid, NULL);
}