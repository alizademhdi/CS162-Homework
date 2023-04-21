/*
 * mm_alloc.c
 *
 */

#include "mm_alloc.h"
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <stdio.h>


s_block_ptr head_pointer = NULL;


void split_block (s_block_ptr block, size_t size)
{
    if (block == NULL || size <= 0)
        return;


    if(block->size >= size + sizeof(s_block)) {
        s_block_ptr ptr = (s_block_ptr) (block->ptr + size);
        
        if (block->next)
            (block->next)->prev = ptr;

        ptr->prev = block;
        ptr->next = block->next;
        block->next = ptr;
        ptr->size = block->size - size - sizeof(s_block);
        block->size = size;
        ptr->ptr = block->ptr + size + sizeof(s_block);
        
        mm_free(ptr->ptr);
        memset(block->ptr, 0, block->size);
    }
}


s_block_ptr extend_heap (s_block_ptr last , size_t s)
{
    void *ptr = sbrk(s + sizeof(s_block));

    if (ptr == (void *) -1)
        return NULL;

    s_block_ptr block = (s_block_ptr) ptr;

    if (last)
        last->next = block;
    else
        head_pointer = block;

    block->next = NULL;
    block->prev = last;
    block->is_free = 0;
    block->size = s;
    block->ptr = ptr + sizeof(s_block);

    memset(block->ptr, 0, block->size);

    return block->ptr;
}

s_block_ptr get_block (void *ptr)
{
    for (s_block_ptr head = head_pointer; head; head = head->next) {
        if (head->ptr == ptr)
            return head;
    }
    return NULL;
}


void fusion(s_block_ptr block)
{
    if (block->prev != NULL && (block->prev)->is_free == 1) {
        (block->prev)->is_free = block->is_free;
        (block->prev)->next = block->next;
        (block->prev)->size = (block->prev)->size + sizeof(s_block) + block->size;
        if (block->next != NULL)
            (block->next)->prev = block->prev;
    }

    if (block->next != NULL && (block->next)->is_free == 1) {
        block->next = (block->next)->next;
        (block->next)->prev = block;
        block->size = block->size + sizeof(s_block) +(block->next)->size;
    }
}


void *initial_heap(size_t size) {

    if (sbrk(size + sizeof(s_block)) == (void *)-1)
        return NULL;

    head_pointer = sbrk(0);

    head_pointer->next = NULL;
    head_pointer->prev = NULL;
    head_pointer->size = size ;
    head_pointer->is_free = 0;
    head_pointer->ptr = head_pointer + sizeof(s_block);

    memset(head_pointer->ptr, 0, head_pointer->size);
    return head_pointer->ptr;
}


void* mm_malloc(size_t size)
{
    if (size == 0)
        return NULL;

    if (head_pointer == NULL)
        return extend_heap(NULL, size);

    s_block_ptr prev = NULL;

    for (s_block_ptr head = head_pointer; head; head = head->next)
    {
        if (head->is_free == 1 && head->size >= size) {
            head->is_free = 0;
            split_block(head, size);
			return head->ptr;
        }
        prev = head;
    }

    return extend_heap(prev, size);
}


void* mm_realloc(void* ptr, size_t size)
{
    if (ptr == NULL)
        return mm_malloc(size);
    
    if (size == 0)
        return NULL;

    s_block_ptr block = get_block(ptr);
    if (block){
        void *new = mm_malloc(size);

        memcpy(new, ptr, block->size);
        block->is_free = 1;
        fusion(block);
        return new;
    }
    return NULL;

}


void mm_free(void* ptr)
{
    if (ptr == NULL)
        return;
    
    s_block_ptr block = get_block(ptr);
    
    if(block){
        block->is_free = 1;
        fusion(block);
    }
}
