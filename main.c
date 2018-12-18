
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include "jcontext.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

jmem_heap_free_t* firstPosition;
static inline jmem_heap_free_t *__attr_always_inline___ __attr_pure___
jmem_heap_get_region_end(jmem_heap_free_t *curr_p) /**< current region */
{
    return (jmem_heap_free_t *) ((uint8_t *) curr_p + curr_p->size);
} /* jmem_heap_get_region_end */

static __attr_hot___ void *
jmem_heap_alloc_block_internal(const size_t size) {

    /* Align size. */
    const size_t required_size = ((size + JMEM_ALIGNMENT - 1) / JMEM_ALIGNMENT) * JMEM_ALIGNMENT;
    jmem_heap_free_t *data_space_p = NULL;

    /* Fast path for 8 byte chunks, first region is guaranteed to be sufficient. */
    if (required_size == JMEM_ALIGNMENT
        && likely (firstPosition->next_offset != JMEM_HEAP_END_OF_LIST)) {
        data_space_p = JMEM_HEAP_GET_ADDR_FROM_OFFSET (firstPosition->next_offset);

        jmem_heap_allocated_size += JMEM_ALIGNMENT;\
        if (data_space_p->size == JMEM_ALIGNMENT) {
            firstPosition->next_offset = data_space_p->next_offset;
        } else {
            jmem_heap_free_t *remaining_p;
            remaining_p = JMEM_HEAP_GET_ADDR_FROM_OFFSET (firstPosition->next_offset) + 1;
            remaining_p->size = data_space_p->size - JMEM_ALIGNMENT;
            remaining_p->next_offset = data_space_p->next_offset;
            firstPosition->next_offset = JMEM_HEAP_GET_OFFSET_FROM_ADDR (remaining_p);
        }


        if (unlikely (data_space_p == jmem_heap_list_skip_p)) {
            jmem_heap_list_skip_p = JMEM_HEAP_GET_ADDR_FROM_OFFSET (firstPosition->next_offset);
        }
    }
        /* Slow path for larger regions. */
    else {
        uint32_t current_offset = firstPosition->next_offset;
        jmem_heap_free_t *prev_p = &firstPosition;

        while (current_offset != JMEM_HEAP_END_OF_LIST) {
            jmem_heap_free_t *current_p = JMEM_HEAP_GET_ADDR_FROM_OFFSET (current_offset);
            const uint32_t next_offset = current_p->next_offset;

            if (current_p->size >= required_size) {
                /* Region is sufficiently big, store address. */
                data_space_p = current_p;
                jmem_heap_allocated_size += required_size;

                /* Region was larger than necessary. */
                if (current_p->size > required_size) {
                    /* Get address of remaining space. */
                    jmem_heap_free_t *const remaining_p = (jmem_heap_free_t *) ((uint8_t *) current_p + required_size);

                    /* Update metadata. */
                    remaining_p->size = current_p->size - (uint32_t) required_size;
                    remaining_p->next_offset = next_offset;

                    /* Update list. */
                    prev_p->next_offset = JMEM_HEAP_GET_OFFSET_FROM_ADDR (remaining_p);
                }
                    /* Block is an exact fit. */
                else {
                    /* Remove the region from the list. */
                    prev_p->next_offset = next_offset;
                }

                jmem_heap_list_skip_p = prev_p;

                /* Found enough space. */
                break;
            }

            /* Next in list. */
            prev_p = current_p;
            current_offset = next_offset;
        }
    }

    if (unlikely (!data_space_p)) {
        return NULL;
    }

    return (void *) data_space_p;
} /* jmem_heap_alloc_block_internal */

void __attr_hot___
jmem_heap_free_block(void *ptr, /**< pointer to beginning of data space of the block */
                     const size_t size) /**< size of allocated region */
{

    /* checking that ptr points to the heap */
    jmem_heap_free_t *block_p = (jmem_heap_free_t *) ptr;
    jmem_heap_free_t *prev_p;
    jmem_heap_free_t *next_p;


    if (block_p > jmem_heap_list_skip_p) {
        prev_p = jmem_heap_list_skip_p;
    } else {
        prev_p = &firstPosition;
    }

    const uint32_t block_offset = JMEM_HEAP_GET_OFFSET_FROM_ADDR (block_p);

    /* Find position of region in the list. */
    while (prev_p->next_offset < block_offset) {
        next_p = JMEM_HEAP_GET_ADDR_FROM_OFFSET (prev_p->next_offset);
        prev_p = next_p;
    }

    next_p = JMEM_HEAP_GET_ADDR_FROM_OFFSET (prev_p->next_offset);

    /* Realign size */
    const size_t aligned_size = (size + JMEM_ALIGNMENT - 1) / JMEM_ALIGNMENT * JMEM_ALIGNMENT;

    /* Update prev. */
    if (jmem_heap_get_region_end(prev_p) == block_p) {
        /* Can be merged. */
        prev_p->size += (uint32_t) aligned_size;
        block_p = prev_p;
    } else {
        block_p->size = (uint32_t) aligned_size;
        prev_p->next_offset = block_offset;
    }
    /* Update next. */
    if (jmem_heap_get_region_end(block_p) == next_p) {
        /* Can be merged. */
        block_p->size += next_p->size;
        block_p->next_offset = next_p->next_offset;
    } else {
        block_p->next_offset = JMEM_HEAP_GET_OFFSET_FROM_ADDR (next_p);
    }

    jmem_heap_list_skip_p = prev_p;
    jmem_heap_allocated_size -= aligned_size;

} /* jmem_heap_free_block */


inline void *__attr_hot___ __attr_always_inline___
jmem_heap_alloc_block(const size_t size)  /**< required memory size */
{
    return jmem_heap_alloc_block_internal(size);
} /* jmem_heap_alloc_block */

int main() {
    char* path = "/Users/tunan/oops";
    size_t size = JMEM_HEAP_AREA_SIZE;
    int fd = open(path, O_RDWR|O_TRUNC);
    lseek(fd, size, SEEK_SET);
    write(fd,"",1);
    void* area = mmap(NULL, 524280, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    int ret = close(fd);
    if(ret == -1) {
        printf("oops");
        exit(1);
    }
    firstPosition = (jmem_heap_free_t *)&area[0];
    printf("position of first %d\n", &area[0]);
    JERRY_HEAP_CONTEXT(area) = &area[7];
    jmem_heap_free_t *const region_p = (jmem_heap_free_t *) JERRY_HEAP_CONTEXT (area);
    region_p->size = JMEM_HEAP_AREA_SIZE;
    region_p->next_offset = JMEM_HEAP_END_OF_LIST;
    firstPosition->size = 0;
    firstPosition->next_offset = JMEM_HEAP_GET_OFFSET_FROM_ADDR (region_p);
    jmem_heap_list_skip_p = &firstPosition;
    uint8_t *a[32];
    for (int i = 0; i < 32; i++) {
        a[i] = (uint8_t *) jmem_heap_alloc_block(8);
        memset(a[i], i, 8);
    }
    for (int i = 0; i < 32 ; i++) {
        printf("%d\n", a[i]);
    }


}

