#include "internal.h"

void jeffe_destroy(jeffe_value val)
{
    switch (jeffe_value_type(val))
    {
    case JEFFE_TYPETAG_F64:
    case JEFFE_TYPETAG_I64:
    case JEFFE_TYPETAG_U64:
        free(ptr_uncompress(val.v & PAYLOAD_MASK));
        break;
    case JEFFE_TYPETAG_CSTRUCT: {
        cstruct_header *hdr = (cstruct_header*)
            ((intptr_t)ptr_uncompress(val.v & PAYLOAD_MASK) - offsetof(cstruct_header, sz));
        free(hdr);
        break;
    }
    case JEFFE_TYPETAG_OBJ: {
        objmeta *meta = (objmeta*)
            ((intptr_t)ptr_uncompress(val.v & PAYLOAD_MASK) - offsetof(objmeta, userdata));
        meta->class_fn(&meta->userdata, JEFFE_OP_DTOR, 0, NULL);
        objmeta_free(meta);
        break;
    }
    case JEFFE_TYPETAG_NIL:
    case JEFFE_TYPETAG_CHAR:
    case JEFFE_TYPETAG_BOOL:
    case JEFFE_TYPETAG_I32:
    case JEFFE_TYPETAG_U32:
    case JEFFE_TYPETAG_F32:
    case JEFFE_TYPETAG_PTR:
    case JEFFE_TYPETAG_ERRNUM:
        break;
    }
}