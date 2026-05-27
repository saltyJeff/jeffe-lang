#include "objmeta.hpp"
#include <jeffe_lang.h>

bool jeffe_is_err(jeffe_value v)
{
    switch (jeffe_value_typetag(v))
    {
    case JEFFE_TYPETAG_ERRNUM: return true;
    case JEFFE_TYPETAG_OBJ:
    {
        objmeta *meta = objmeta::from_value(v);
        jeffe_value is_err_ret = meta->invoke(JEFFE_OP_IS_ERR, 0, nullptr);
        jeffe_value_holder holder = jeffe_value_held_data(is_err_ret);
        return (holder.typetag == JEFFE_TYPETAG_BOOL) && holder.b;
    }
    default: return false;
    }
}

const char *jeffe_typename(jeffe_value v)
{
    jeffe_typetag tag = jeffe_value_typetag(v);
    if (tag == JEFFE_TYPETAG_OBJ)
    {
        objmeta *meta = objmeta::from_value(v);
        jeffe_value ret = meta->invoke(JEFFE_OP_CLASSNAME, 0, nullptr);
        jeffe_value_holder holder = jeffe_value_held_data(ret);
        if (holder.typetag == JEFFE_TYPETAG_PTR)
        {
            return reinterpret_cast<const char *>(holder.ptr);
        }
    }
    return jeffe_strtypetag(tag);
}