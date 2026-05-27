#include "cstruct.hpp"
#include "objmeta.hpp"
#include "value.hpp"
#include <jeffe_lang.h>

void jeffe_destroy(jeffe_value val)
{
    switch (jeffe_value_typetag(val))
    {
    case JEFFE_TYPETAG_F64:
    case JEFFE_TYPETAG_I64:
    case JEFFE_TYPETAG_U64: {
        delete ptr_uncompress(val.v & PAYLOAD_MASK);
        break;
    }
    case JEFFE_TYPETAG_CSTRUCT: {
        cstruct_delete(ptr_uncompress(val.v & PAYLOAD_MASK));
        break;
    }
    case JEFFE_TYPETAG_OBJ: {
        objmeta *meta = objmeta::from_userdata((void **)ptr_uncompress(val.v & PAYLOAD_MASK));
        meta->invoke(JEFFE_OP_DTOR, 0, NULL);
        delete meta;
        break;
    }
    case JEFFE_TYPETAG_NIL:
    case JEFFE_TYPETAG_CHAR:
    case JEFFE_TYPETAG_BOOL:
    case JEFFE_TYPETAG_I32:
    case JEFFE_TYPETAG_U32:
    case JEFFE_TYPETAG_F32:
    case JEFFE_TYPETAG_PTR:
    case JEFFE_TYPETAG_ERRNUM: break;
    }
}
jeffe_value jeffe_copy(jeffe_value rhs)
{
    switch (jeffe_value_typetag(rhs))
    {
    case JEFFE_TYPETAG_F64: return jeffe_value_f64(*reinterpret_cast<double *>(ptr_uncompress(rhs.v & PAYLOAD_MASK)));
    case JEFFE_TYPETAG_I64: return jeffe_value_i64(*reinterpret_cast<int64_t *>(ptr_uncompress(rhs.v & PAYLOAD_MASK)));
    case JEFFE_TYPETAG_U64: return jeffe_value_u64(*reinterpret_cast<uint64_t *>(ptr_uncompress(rhs.v & PAYLOAD_MASK)));
    case JEFFE_TYPETAG_CSTRUCT:
        return value_builder(JEFFE_TYPETAG_CSTRUCT, 0, ptr_compress(cstruct_dup(ptr_uncompress(rhs.v & PAYLOAD_MASK))));
    case JEFFE_TYPETAG_OBJ:
    case JEFFE_TYPETAG_NIL:
    case JEFFE_TYPETAG_CHAR:
    case JEFFE_TYPETAG_BOOL:
    case JEFFE_TYPETAG_I32:
    case JEFFE_TYPETAG_U32:
    case JEFFE_TYPETAG_F32:
    case JEFFE_TYPETAG_PTR:
    case JEFFE_TYPETAG_ERRNUM: return rhs;
    }
}
jeffe_value jeffe_get(jeffe_value obj, jeffe_value key)
{
    if (jeffe_value_typetag(obj) != JEFFE_TYPETAG_OBJ)
    {
        return jeffe_value_errnum(JEFFE_ERRNO_UNDEFINED, jeffe_builtin_strerror);
    }
    objmeta *meta = objmeta::from_userdata((void **)ptr_uncompress(obj.v & PAYLOAD_MASK));
    return meta->invoke(JEFFE_OP_GET, 1, &key);
}
jeffe_value jeffe_set(jeffe_value obj, jeffe_value key, jeffe_value val)
{
    if (jeffe_value_typetag(obj) != JEFFE_TYPETAG_OBJ)
    {
        return jeffe_value_errnum(JEFFE_ERRNO_UNDEFINED, jeffe_builtin_strerror);
    }
    objmeta *meta = objmeta::from_userdata((void **)ptr_uncompress(obj.v & PAYLOAD_MASK));
    jeffe_value args[2] = {key, val};
    return meta->invoke(JEFFE_OP_SET, 2, args);
}
