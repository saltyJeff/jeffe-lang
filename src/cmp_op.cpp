#include "math_helpers.hpp"

jeffe_value jeffe_cmp(jeffe_value a, jeffe_value b)
{
    jeffe_value ret = try_invoke_meta(a, JEFFE_OP_CMP, b);
    if (!(jeffe_value_typetag(ret) == JEFFE_TYPETAG_ERRNUM &&
          jeffe_value_held_data(ret).errnum.errnum == JEFFE_ERRNO_NOTIMPL))
    {
        return ret;
    }

    ret = try_invoke_meta(b, JEFFE_OP_CMP, a);
    if (!(jeffe_value_typetag(ret) == JEFFE_TYPETAG_ERRNUM &&
          jeffe_value_held_data(ret).errnum.errnum == JEFFE_ERRNO_NOTIMPL))
    {
        jeffe_value_holder holder = jeffe_value_held_data(ret);
        if (holder.typetag == JEFFE_TYPETAG_I32)
        {
            return jeffe_value_i32(-holder.i32);
        }
        return ret;
    }

    jeffe_value_holder holderA = jeffe_value_held_data(a);
    jeffe_value_holder holderB = jeffe_value_held_data(b);

    if (holderA.typetag == JEFFE_TYPETAG_OBJ && holderB.typetag == JEFFE_TYPETAG_OBJ)
    {
        return jeffe_value_i32(static_cast<int32_t>(reinterpret_cast<intptr_t>(holderA.obj.userdata) - reinterpret_cast<intptr_t>(holderB.obj.userdata)));
    }

    if (is_numeric_or_char(holderA.typetag) && is_numeric_or_char(holderB.typetag))
    {
        jeffe_typetag common = get_common_type(holderA.typetag, holderB.typetag);
        jeffe_value_holder promA = cast_to(holderA, common);
        jeffe_value_holder promB = cast_to(holderB, common);

        switch (common)
        {
        case JEFFE_TYPETAG_I32:
            return jeffe_value_i32((promA.i32 > promB.i32) - (promA.i32 < promB.i32));
        case JEFFE_TYPETAG_U32:
            return jeffe_value_i32((promA.u32 > promB.u32) - (promA.u32 < promB.u32));
        case JEFFE_TYPETAG_I64:
            return jeffe_value_i32((promA.i64 > promB.i64) - (promA.i64 < promB.i64));
        case JEFFE_TYPETAG_U64:
            return jeffe_value_i32((promA.u64 > promB.u64) - (promA.u64 < promB.u64));
        case JEFFE_TYPETAG_F32:
            return jeffe_value_i32((promA.f32 > promB.f32) - (promA.f32 < promB.f32));
        case JEFFE_TYPETAG_F64:
            return jeffe_value_i32((promA.f64 > promB.f64) - (promA.f64 < promB.f64));
        default: break;
        }
    }

    if (holderA.typetag == holderB.typetag)
    {
        switch (holderA.typetag)
        {
        case JEFFE_TYPETAG_NIL: return jeffe_value_i32(0);
        case JEFFE_TYPETAG_BOOL:
        {
            return jeffe_value_i32(static_cast<int32_t>(holderA.b) - static_cast<int32_t>(holderB.b));
        }
        case JEFFE_TYPETAG_PTR:
        {
            return jeffe_value_i32(static_cast<int32_t>(reinterpret_cast<intptr_t>(holderA.ptr) - reinterpret_cast<intptr_t>(holderB.ptr)));
        }
        case JEFFE_TYPETAG_CSTRUCT:
        {
            return jeffe_value_i32(static_cast<int32_t>(reinterpret_cast<intptr_t>(holderA.cstruct) - reinterpret_cast<intptr_t>(holderB.cstruct)));
        }
        case JEFFE_TYPETAG_ERRNUM:
        {
            if (holderA.errnum.errnum != holderB.errnum.errnum)
            {
                return jeffe_value_i32(holderA.errnum.errnum - holderB.errnum.errnum);
            }
            return jeffe_value_i32(static_cast<int32_t>(reinterpret_cast<intptr_t>(reinterpret_cast<void*>(holderA.errnum.fn)) - reinterpret_cast<intptr_t>(reinterpret_cast<void*>(holderB.errnum.fn))));
        }
        default: return jeffe_value_i32(0);
        }
    }

    return jeffe_value_i32(static_cast<int32_t>(holderA.typetag) - static_cast<int32_t>(holderB.typetag));
}
