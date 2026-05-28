#include "objmeta.hpp"
#include <jeffe_lang.h>

jeffe_value jeffe_len(jeffe_value obj)
{
    if (jeffe_value_typetag(obj) != JEFFE_TYPETAG_OBJ)
    {
        return jeffe_value_errnum(JEFFE_ERRNO_UNDEFINED, jeffe_builtin_strerror);
    }
    objmeta *meta = objmeta::from_value(obj);
    return meta->invoke(JEFFE_OP_LEN, 0, nullptr);
}

jeffe_value jeffe_iter(jeffe_value obj, jeffe_value iter_obj)
{
    if (jeffe_value_typetag(obj) != JEFFE_TYPETAG_OBJ)
    {
        return jeffe_value_errnum(JEFFE_ERRNO_UNDEFINED, jeffe_builtin_strerror);
    }
    objmeta *meta = objmeta::from_value(obj);
    return meta->invoke(JEFFE_OP_ITER, 1, &iter_obj);
}
