//
// Created by 王宗祥 on 2018/12/6.
//

#include "jcontext.h"

#define JERRY_GLOBAL_HEAP_SECTION

jmem_heap_free_t* jmem_heap_list_skip_p;
jmem_heap_t jerry_global_heap __attribute__ ((aligned (8))) JERRY_GLOBAL_HEAP_SECTION;
size_t jmem_heap_allocated_size;

