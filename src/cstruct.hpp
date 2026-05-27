#pragma once
#include <cstring>
#include <jeffe_lang.h>

/**
 * Represents a fat pointer to a C struct
 * The returned pointer is PRECEDED by a size_t indicating the allocated size
 * TODO: maybe switch to another allocator that saves the size of each alloc?
 * @param sz size to allocated
 * @return: a pointer to a block of sz
 */
inline void *cstruct_new(size_t sz)
{
    size_t *hdr = reinterpret_cast<size_t *>(new char[sizeof(size_t) + sz]);
    hdr[0] = sz;
    return &hdr[1];
}
/**
 * Duplicates a fat pointer
 */
inline void *cstruct_dup(void *ptr)
{
    size_t *hdr = reinterpret_cast<size_t *>(ptr) - 1;
    void *new_ptr = cstruct_new(hdr[0]);
    memcpy(new_ptr, ptr, hdr[0]);
    return new_ptr;
}

/**
 * Deletes a fat pointer.
 * @param ptr: the pointer to delete
 */
inline void cstruct_delete(void *ptr)
{
    size_t *hdr = reinterpret_cast<size_t *>(ptr) - 1;
    delete[] reinterpret_cast<char *>(hdr);
}