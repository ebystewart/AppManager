#include "glthread.h"
#include <stdlib.h>

void init_glthread(glthread_t *glthread)
{
    glthread->left  = NULL;
    glthread->right = NULL;

}

void glthread_add_next(glthread_t *base_glthread, glthread_t *new_glthread)
{
    if(!base_glthread->right)
    {
        new_glthread->left = base_glthread;
        base_glthread->right = new_glthread;
        return;
    }

    glthread_t *temp = base_glthread->right;
    base_glthread->right = new_glthread;
    new_glthread->left = base_glthread;
    new_glthread->right = temp;
    temp->left = new_glthread;
}

void glthread_add_before(glthread_t *base_glthread, glthread_t *new_glthread)
{

}

void glthread_add_last(glthread_t *base_thread, glthread_t *new_glthread)
{

}

void remove_glthread(glthread_t *glthread)
{

}

void delete_glthread_list(glthread_t *base_glthread)
{

}

unsigned int get_glthread_list_count(glthread_t *base_glthread)
{

}

void glthread_priority_insert(glthread_t *base_glthread, glthread_t *glthread,
                                int (*comp_fn)(void *, void *), int count)
{

}