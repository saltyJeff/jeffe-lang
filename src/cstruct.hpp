#pragma once
#include <jeffe_lang.h>

/// represents a jank smart pointer.
/// the size of the allocation is stored BEFORE the returned heap data pointer in a header

void *cstruct_new(size_t sz)
{
    size_t *hdr = reinterpret_cast<size_t *>(new char[sizeof(size_t) + sz]);
    hdr[0] = sz;
    return &hdr[1];
}

void cstruct_delete(void *ptr)
{
    size_t *hdr = reinterpret_cast<size_t *>(ptr) - 1;
    delete[] reinterpret_cast<char *>(hdr);
}