#pragma once
#include <jeffe_lang.h>

static const uint64_t PAYLOAD_MASK = 0x0000FFFFFFFFFFFFul;
static const int TYPETAG_SHIFT = 56;
static uint64_t ptr_compress(void *ptr)
{
    return (uint64_t)((intptr_t)ptr & PAYLOAD_MASK);
}
static void *ptr_uncompress(uint64_t comp_ptr)
{
    return (void *)(intptr_t)((int64_t)(comp_ptr << 16) >> 16);
}
