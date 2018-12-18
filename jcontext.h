//
// Created by 王宗祥 on 2018/12/6.
//

#include <stdint.h>
#include <stdio.h>

#ifndef UNTITLED4_JCONTEXT_H
#define UNTITLED4_JCONTEXT_H

#endif //UNTITLED4_JCONTEXT_H

#define __attr_hot___ __attribute__((hot))
#define JMEM_ALIGNMENT (1u << 8)
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)
#define JMEM_HEAP_GET_ADDR_FROM_OFFSET(u) ((jmem_heap_free_t *) (JERRY_HEAP_CONTEXT (area) + (u)))
#define JMEM_HEAP_GET_OFFSET_FROM_ADDR(p) ((uint32_t) ((uint8_t *) (p) - JERRY_HEAP_CONTEXT (area)))

#define JMEM_HEAP_AREA_SIZE 524280
#define __attr_always_inline___ __attribute__((always_inline))
#define __attr_pure___ __attribute__((pure))
#define JMEM_HEAP_END_OF_LIST ((uint32_t) 0xffffffff)
#define JERRY_HEAP_CONTEXT(field) (jerry_global_heap.field)
typedef struct
{
    uint32_t next_offset; /**< Offset of next region in list */
    uint32_t size; /**< Size of region */
} jmem_heap_free_t;


typedef struct
{
    jmem_heap_free_t first; /**< first node in free region list */
//    uint8_t area[512 * 1024 -8]; /**< heap area */
    uint8_t * area;
} jmem_heap_t;

typedef struct
{
    int a;
    char b;
} test_struct;

extern jmem_heap_free_t* jmem_heap_list_skip_p;
extern size_t jmem_heap_allocated_size;
extern jmem_heap_t jerry_global_heap;




