#pragma once
#include "value.hpp"
#include <cstddef>
#include <cstdint>
#include <jeffe_lang.h>

class objmeta
{
private:
    uint64_t gc_tags : 16;
    uint64_t class_fn_compressed : 48;
    void *userdata = nullptr;
public:
    objmeta(jeffe_class_fn fn)
        : class_fn_compressed(ptr_compress(reinterpret_cast<void *>(fn))), gc_tags(0), userdata(nullptr)
    {
    }
    void **userdata_ptr()
    {
        return &userdata;
    }
    jeffe_class_fn class_fn() const
    {
        return reinterpret_cast<jeffe_class_fn>(ptr_uncompress(class_fn_compressed));
    }
    jeffe_value invoke(int op, size_t argn, const jeffe_value *argv)
    {
        return class_fn()(userdata_ptr(), op, argn, argv);
    }
    static objmeta *from_userdata(void **userdata_ptr)
    {
        return reinterpret_cast<objmeta *>((char *)userdata_ptr - offsetof(objmeta, userdata));
    }
};