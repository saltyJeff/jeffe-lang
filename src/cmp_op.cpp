#include "math_helpers.hpp"
#include <tuple>

template <typename T>
static int signum(T val) { return (val > T(0)) - (val < T(0)); }

template <typename T>
static int compare_values(const T& a, const T& b) { return (a > b) - (a < b); }

static int val_to_cmp_result(jeffe_value val)
{
    jeffe_value_holder h = jeffe_value_held_data(val);
    switch (h.typetag)
    {
    case JEFFE_TYPETAG_I32:  return signum(h.i32);
    case JEFFE_TYPETAG_U32:  return signum(h.u32);
    case JEFFE_TYPETAG_I64:  return signum(h.i64);
    case JEFFE_TYPETAG_U64:  return signum(h.u64);
    case JEFFE_TYPETAG_F32:  return signum(h.f32);
    case JEFFE_TYPETAG_F64:  return signum(h.f64);
    case JEFFE_TYPETAG_BOOL: return signum(h.b);
    case JEFFE_TYPETAG_CHAR: return signum(h.c);
    default:                 return 0;
    }
}

static jeffe_value not_ordered_err()
{
    return jeffe_value_errnum(JEFFE_ERRNO_NOTORDERED, jeffe_builtin_strerror);
}

jeffe_value jeffe_cmp(jeffe_value a, jeffe_value b)
{
    for (auto [x, y, sign] : {std::tuple{a, b, 1}, std::tuple{b, a, -1}})
    {
        jeffe_value ret = try_invoke_meta(x, JEFFE_OP_CMP, y);
        jeffe_value_holder h = jeffe_value_held_data(ret);
        if (!(h.typetag == JEFFE_TYPETAG_ERRNUM && h.errnum.errnum == JEFFE_ERRNO_NOTIMPL))
        {
            return (h.typetag == JEFFE_TYPETAG_ERRNUM && h.errnum.errnum == JEFFE_ERRNO_NOTORDERED) 
                   ? ret : jeffe_value_i32(sign * val_to_cmp_result(ret));
        }
    }

    jeffe_value_holder holderA = jeffe_value_held_data(a);
    jeffe_value_holder holderB = jeffe_value_held_data(b);

    if (is_numeric_or_char(holderA.typetag) && is_numeric_or_char(holderB.typetag))
    {
        jeffe_typetag common = get_common_type(holderA.typetag, holderB.typetag);
        jeffe_value_holder promA = cast_to(holderA, common);
        jeffe_value_holder promB = cast_to(holderB, common);

        switch (common)
        {
        case JEFFE_TYPETAG_I32:  return jeffe_value_i32(compare_values(promA.i32, promB.i32));
        case JEFFE_TYPETAG_U32:  return jeffe_value_i32(compare_values(promA.u32, promB.u32));
        case JEFFE_TYPETAG_I64:  return jeffe_value_i32(compare_values(promA.i64, promB.i64));
        case JEFFE_TYPETAG_U64:  return jeffe_value_i32(compare_values(promA.u64, promB.u64));
        case JEFFE_TYPETAG_F32:  return jeffe_value_i32(compare_values(promA.f32, promB.f32));
        case JEFFE_TYPETAG_F64:  return jeffe_value_i32(compare_values(promA.f64, promB.f64));
        default: break;
        }
    }

    if (holderA.typetag == holderB.typetag)
    {
        switch (holderA.typetag)
        {
        case JEFFE_TYPETAG_NIL:     return jeffe_value_i32(0);
        case JEFFE_TYPETAG_BOOL:    return jeffe_value_i32(static_cast<int>(holderA.b) - static_cast<int>(holderB.b));
        case JEFFE_TYPETAG_PTR:     return jeffe_value_i32((holderA.ptr > holderB.ptr) - (holderA.ptr < holderB.ptr));
        case JEFFE_TYPETAG_CSTRUCT: return (holderA.cstruct == holderB.cstruct) ? jeffe_value_i32(0) : not_ordered_err();
        case JEFFE_TYPETAG_OBJ:     return (holderA.obj.userdata == holderB.obj.userdata) ? jeffe_value_i32(0) : not_ordered_err();
        case JEFFE_TYPETAG_ERRNUM:
            return ((holderA.errnum.errnum == holderB.errnum.errnum) && (holderA.errnum.fn == holderB.errnum.fn)) ? jeffe_value_i32(0) : not_ordered_err();
        default:                    return not_ordered_err();
        }
    }

    return jeffe_value_i32(signum(static_cast<int>(holderA.typetag) - static_cast<int>(holderB.typetag)));
}
