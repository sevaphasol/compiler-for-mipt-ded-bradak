#include <stdlib.h>

//-------------------------------------------------------------------//

#include "node_allocator.h"
#include "custom_assert.h"

//———————————————————————————————————————————————————————————————————//

static node_allocator_status_t big_array_realloc (node_allocator_t* node_allocator);
static node_allocator_status_t arrays_calloc     (node_allocator_t* node_allocator);

//———————————————————————————————————————————————————————————————————//

node_allocator_status_t node_allocator_ctor(node_allocator_t* allocator,
                                            size_t array_len)
{
    VERIFY(!allocator,
           return NODE_ALLOCATOR_STRUCT_NULL_PTR_ERROR);

    //-------------------------------------------------------------------//

    allocator->n_arrays  = 1;
    allocator->big_array = (node_t**) calloc(allocator->n_arrays, sizeof(node_t**));
    VERIFY(!allocator->big_array,
           return NODE_ALLOCATOR_STD_CALLOC_ERROR);

    //-------------------------------------------------------------------//

    allocator->array_len = array_len;
    allocator->big_array[0] = (node_t*) calloc(array_len, sizeof(node_t));
    VERIFY(!allocator->big_array[0],
            return NODE_ALLOCATOR_STD_CALLOC_ERROR);

    //-------------------------------------------------------------------//

    allocator->free_place = 0;

    //-------------------------------------------------------------------//

    return NODE_ALLOCATOR_SUCCESS;
}

//===================================================================//

node_allocator_status_t node_allocator_dtor(node_allocator_t* allocator)
{
    VERIFY(!allocator, return NODE_ALLOCATOR_STRUCT_NULL_PTR_ERROR);

    //-------------------------------------------------------------------//

    for (int i = 0; i < allocator->n_arrays; i++)
    {
        free(allocator->big_array[i]);
    }

    //-------------------------------------------------------------------//

    free(allocator->big_array);

    //-------------------------------------------------------------------//

    allocator->free_place = 0;
    allocator->n_arrays   = 0;
    allocator->array_len  = 0;

    //-------------------------------------------------------------------//

    return NODE_ALLOCATOR_SUCCESS;
}

//===================================================================//

node_t* node_ctor(node_allocator_t* allocator,
                  value_type_t      value_type,
                  value_t           value,
                  size_t            line_number,
                  node_t*           left,
                  node_t*           right)
{
    VERIFY(!allocator, return nullptr);

    //-------------------------------------------------------------------//

    if (allocator->free_place >= allocator->n_arrays * allocator->array_len)
    {
        VERIFY(big_array_realloc(allocator), return nullptr);
        VERIFY(arrays_calloc    (allocator), return nullptr);
    }

    //-------------------------------------------------------------------//

    size_t cur_array      = allocator->free_place / allocator->array_len;
    size_t rel_free_place = allocator->free_place % allocator->array_len;

    node_t* new_node = &allocator->big_array[cur_array][rel_free_place];

    allocator->free_place++;

    //-------------------------------------------------------------------//

    new_node->value_type  = value_type;
    new_node->value       = value;
    new_node->left        = left;
    new_node->right       = right;
    new_node->line_number = line_number;

    //-------------------------------------------------------------------//

    return new_node;
}

//===================================================================//

node_allocator_status_t big_array_realloc(node_allocator_t* allocator)
{
    ASSERT(allocator);

    //-------------------------------------------------------------------//

    node_t** new_big_array = (node_t**) realloc(allocator->big_array,
                                                (allocator->n_arrays + 1) * sizeof(node_t*));
    VERIFY(!new_big_array,
           return BIG_ARRAY_REALLOC_ERROR);

    allocator->big_array = new_big_array;
    allocator->n_arrays++;

    //-------------------------------------------------------------------//

    return NODE_ALLOCATOR_SUCCESS;
}

//===================================================================//

node_allocator_status_t arrays_calloc(node_allocator_t* allocator)
{
    ASSERT(allocator);

    //-------------------------------------------------------------------//

    allocator->big_array[allocator->n_arrays - 1] = (node_t*)
         calloc(allocator->array_len, sizeof(node_t));

    VERIFY(!allocator->big_array[allocator->n_arrays - 1],
        return NODE_ALLOCATOR_STD_CALLOC_ERROR);

    return NODE_ALLOCATOR_SUCCESS;
}

//———————————————————————————————————————————————————————————————————//
