#include "value.hpp"
#include <jeffe_lang.h>

JEFFE_API bool jeffe_as_index(struct jeffe_value v, intptr_t *out)
{
    if (out == nullptr)
    {
        return false;
    }

    jeffe_value_holder holder = jeffe_value_held_data(v);
    switch (holder.typetag)
    {
    case JEFFE_TYPETAG_CHAR:
        *out = static_cast<intptr_t>(holder.c);
        return true;
    case JEFFE_TYPETAG_I32:
        *out = static_cast<intptr_t>(holder.i32);
        return true;
    case JEFFE_TYPETAG_U32:
        *out = static_cast<intptr_t>(holder.u32);
        return true;
    case JEFFE_TYPETAG_I64:
        *out = static_cast<intptr_t>(holder.i64);
        return true;
    case JEFFE_TYPETAG_U64:
        *out = static_cast<intptr_t>(holder.u64);
        return true;
    default:
        return false;
    }
}
