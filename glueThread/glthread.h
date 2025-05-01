#ifndef GL_THREAD_H
#define GL_THREAD_H

typedef struct glthread_{
    struct glthread_ *left;
    struct glthread_ *right;
}glthread_t;

void init_glthread(glthread_t *glthread);

void glthread_add_next(glthread_t *base_glthread, glthread_t *new_glthread);

void glthread_add_before(glthread_t *base_glthread, glthread_t *new_glthread);

void glthread_add_last(glthread_t *base_thread, glthread_t *new_glthread);

void remove_glthread(glthread_t *glthread);

void delete_glthread_list(glthread_t *base_glthread);

unsigned int get_glthread_list_count(glthread_t *base_glthread);

void glthread_priority_insert(glthread_t *base_glthread, glthread_t *glthread,
                                int (*comp_fn)(void *, void *), int count);


#define IS_GLTHREAD_LIST_EMPTY(glthread_ptr)    \
    (((glthread_ptr)->right == NULL) && ((glthread_ptr)->left == NULL))

#define GLTHREAD_TO_STRUCT(fn_name, structure_name, field_name)             \
    static inline structure_name *fn_name(glthread_t *glthread_ptr){        \
        return (structure_name *)((char *)(glthread_ptr) - (char *)&(((structure_name *)0)->field_name)); \
    }   
    
#define BASE(glthread_ptr) ((glthread_ptr)->right)

#define ITERATE_GLTHREAD_BEGIN(glthread_ptr_start, glthread_ptr)        \
{                                                                       \
    glthread_t *glthread_tmp = NULL;                                    \
    glthread_ptr = BASE(glthread_ptr_start);                            \
    for(; glthread_ptr != NULL; glthread_ptr = glthread_tmp){           \
        glthread_tmp = (glthread_ptr)->right;                           \

#define ITERATE_GLTHREAD_END(glthread_ptr_start, glthread_ptr)          \
        }}

#define GLTHREAD_GET_USER_DATA_FROM_OFFSET(glthread_ptr, offset)        \
        (void *)((char *)(glthread_ptr) - offset)


#endif //GL_THREAD_H