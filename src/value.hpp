#pragma once
#include <jeffe_lang.h>

static const uint64_t PAYLOAD_MASK = 0x0000FFFFFFFFFFFFul;
static const int TYPETAG_SHIFT = 56;
inline uint64_t ptr_compress(void *ptr)
{
    return (uint64_t)((intptr_t)ptr & PAYLOAD_MASK);
}
inline void *ptr_uncompress(uint64_t comp_ptr)
{
    return (void *)(intptr_t)((int64_t)(comp_ptr << 16) >> 16);
}

inline jeffe_value value_builder(jeffe_typetag tag, int8_t meta, uint64_t payload)
{
    uint64_t v = ((uint64_t)tag << TYPETAG_SHIFT) | (((uint64_t)meta & 0xFF) << 48) | (payload & PAYLOAD_MASK);
    jeffe_value ret;
    ret.v = v;
    return ret;
}