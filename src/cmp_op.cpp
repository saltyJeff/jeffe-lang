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

    if (jeffe_value_typetag(a) == JEFFE_TYPETAG_OBJ && jeffe_value_typetag(b) == JEFFE_TYPETAG_OBJ)
    {
        void *ptrA = ptr_uncompress(a.v & PAYLOAD_MASK);
        void *ptrB = ptr_uncompress(b.v & PAYLOAD_MASK);
        return jeffe_value_i32(static_cast<int32_t>(reinterpret_cast<intptr_t>(ptrA) - reinterpret_cast<intptr_t>(ptrB)));
    }

    jeffe_typetag tagA = jeffe_value_typetag(a);
    jeffe_typetag tagB = jeffe_value_typetag(b);

    if (is_numeric_or_char(tagA) && is_numeric_or_char(tagB))
    {
        jeffe_value_holder holderA = jeffe_value_held_data(a);
        jeffe_value_holder holderB = jeffe_value_held_data(b);

        jeffe_typetag common = get_common_type(tagA, tagB);
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

    if (tagA == tagB)
    {
        switch (tagA)
        {
        case JEFFE_TYPETAG_NIL: return jeffe_value_i32(0);
        case JEFFE_TYPETAG_BOOL:
        {
            bool bA = static_cast<bool>(a.v & PAYLOAD_MASK);
            bool bB = static_cast<bool>(b.v & PAYLOAD_MASK);
            return jeffe_value_i32(static_cast<int32_t>(bA) - static_cast<int32_t>(bB));
        }
        case JEFFE_TYPETAG_PTR:
        case JEFFE_TYPETAG_CSTRUCT:
        {
            void *ptrA = ptr_uncompress(a.v & PAYLOAD_MASK);
            void *ptrB = ptr_uncompress(b.v & PAYLOAD_MASK);
            return jeffe_value_i32(static_cast<int32_t>(reinterpret_cast<intptr_t>(ptrA) - reinterpret_cast<intptr_t>(ptrB)));
        }
        case JEFFE_TYPETAG_ERRNUM:
        {
            int8_t errA = static_cast<int8_t>((a.v >> 48) & 0xFF);
            int8_t errB = static_cast<int8_t>((b.v >> 48) & 0xFF);
            if (errA != errB)
            {
                return jeffe_value_i32(errA - errB);
            }
            void *fnA = ptr_uncompress(a.v & PAYLOAD_MASK);
            void *fnB = ptr_uncompress(b.v & PAYLOAD_MASK);
            return jeffe_value_i32(static_cast<int32_t>(reinterpret_cast<intptr_t>(fnA) - reinterpret_cast<intptr_t>(fnB)));
        }
        default: return jeffe_value_i32(0);
        }
    }

    return jeffe_value_i32(static_cast<int32_t>(tagA) - static_cast<int32_t>(tagB));
}
