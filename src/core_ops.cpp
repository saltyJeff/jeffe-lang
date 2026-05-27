#include "cstruct.hpp"
#include "objmeta.hpp"
#include "value.hpp"

void jeffe_destroy(jeffe_value val)
{
    switch (jeffe_value_type(val))
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