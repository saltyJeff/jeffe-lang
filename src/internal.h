#pragma once
#include <jeffe_lang.h>
#include <stdatomic.h>
#include <stdlib.h>


typedef struct jeffe_value jeffe_value;
typedef enum jeffe_typetag jeffe_typetag;
typedef enum jeffe_op jeffe_op;

typedef struct objmeta
{
    jeffe_class_fn class_fn;
    void *userdata;
    uint64_t thread_id;
    uint32_t strong_count;
    atomic_uint_fast16_t weak_count;
    atomic_uint_fast16_t thread_weak_count;
} objmeta;

// TODO: slab allocator or smthn
static objmeta *objmeta_new()
{
    return (objmeta *)malloc(sizeof(objmeta));
}
static void objmeta_free(objmeta *meta)
{
    free(meta);
}
typedef struct
{
    size_t sz;
    uint8_t data[];
} cstruct_header;
static const uint64_t PAYLOAD_MASK = 0x0000FFFFFFFFFFFFul;
static const int TYPETAG_SHIFT = 56;
static uint64_t ptr_compress(void *ptr)
{
    return (uint64_t)((intptr_t)ptr & PAYLOAD_MASK);
}
static void *ptr_uncompress(uint64_t comp_ptr)
{
    return (void *)((intptr_t)(comp_ptr << 16) >> 16);
}
