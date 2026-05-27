#pragma once
#include "objmeta.hpp"
#include "value.hpp"
#include <cmath>
#include <jeffe_lang.h>

template <typename T>
inline void set_promoted_value(jeffe_value_holder &res, T val)
{
    switch (res.typetag)
    {
    case JEFFE_TYPETAG_F64: res.f64 = static_cast<double>(val); break;
    case JEFFE_TYPETAG_F32: res.f32 = static_cast<float>(val); break;
    case JEFFE_TYPETAG_I64: res.i64 = static_cast<int64_t>(val); break;
    case JEFFE_TYPETAG_U64: res.u64 = static_cast<uint64_t>(val); break;
    case JEFFE_TYPETAG_U32: res.u32 = static_cast<uint32_t>(val); break;
    case JEFFE_TYPETAG_I32: res.i32 = static_cast<int32_t>(val); break;
    default: break;
    }
}

inline jeffe_value_holder cast_to(const jeffe_value_holder &val, jeffe_typetag target)
{
    jeffe_value_holder res;
    res.typetag = target;
    switch (val.typetag)
    {
    case JEFFE_TYPETAG_F64: set_promoted_value(res, val.f64); break;
    case JEFFE_TYPETAG_F32: set_promoted_value(res, val.f32); break;
    case JEFFE_TYPETAG_I64: set_promoted_value(res, val.i64); break;
    case JEFFE_TYPETAG_U64: set_promoted_value(res, val.u64); break;
    case JEFFE_TYPETAG_CHAR: set_promoted_value(res, static_cast<uint32_t>(val.c)); break;
    case JEFFE_TYPETAG_U32: set_promoted_value(res, val.u32); break;
    case JEFFE_TYPETAG_I32: set_promoted_value(res, static_cast<uint32_t>(val.i32)); break;
    default: break;
    }
    return res;
}

inline jeffe_typetag get_common_type(jeffe_typetag tagA, jeffe_typetag tagB)
{
    if (tagA == JEFFE_TYPETAG_F64 || tagB == JEFFE_TYPETAG_F64)
    {
        return JEFFE_TYPETAG_F64;
    }
    if (tagA == JEFFE_TYPETAG_F32 || tagB == JEFFE_TYPETAG_F32)
    {
        return JEFFE_TYPETAG_F32;
    }
    if (tagA == JEFFE_TYPETAG_U64 || tagB == JEFFE_TYPETAG_U64)
    {
        return JEFFE_TYPETAG_U64;
    }
    if (tagA == JEFFE_TYPETAG_I64 || tagB == JEFFE_TYPETAG_I64)
    {
        return JEFFE_TYPETAG_I64;
    }
    if (tagA == JEFFE_TYPETAG_U32 || tagB == JEFFE_TYPETAG_U32 || tagA == JEFFE_TYPETAG_CHAR || tagB == JEFFE_TYPETAG_CHAR)
    {
        return JEFFE_TYPETAG_U32;
    }
    return JEFFE_TYPETAG_I32;
}

inline jeffe_value build_from_promoted(const jeffe_value_holder &val)
{
    switch (val.typetag)
    {
    case JEFFE_TYPETAG_I32: return jeffe_value_i32(val.i32);
    case JEFFE_TYPETAG_U32: return jeffe_value_u32(val.u32);
    case JEFFE_TYPETAG_I64: return jeffe_value_i64(val.i64);
    case JEFFE_TYPETAG_U64: return jeffe_value_u64(val.u64);
    case JEFFE_TYPETAG_F32: return jeffe_value_f32(val.f32);
    case JEFFE_TYPETAG_F64: return jeffe_value_f64(val.f64);
    default: return jeffe_value_nil();
    }
}

inline bool is_zero(const jeffe_value_holder &val)
{
    switch (val.typetag)
    {
    case JEFFE_TYPETAG_I32: return val.i32 == 0;
    case JEFFE_TYPETAG_U32: return val.u32 == 0;
    case JEFFE_TYPETAG_I64: return val.i64 == 0;
    case JEFFE_TYPETAG_U64: return val.u64 == 0;
    case JEFFE_TYPETAG_F32: return val.f32 == 0.0f;
    case JEFFE_TYPETAG_F64: return val.f64 == 0.0;
    default: return false;
    }
}

inline bool is_numeric_or_char(jeffe_typetag t)
{
    return t == JEFFE_TYPETAG_CHAR || t == JEFFE_TYPETAG_I32 || t == JEFFE_TYPETAG_U32 ||
           t == JEFFE_TYPETAG_I64 || t == JEFFE_TYPETAG_U64 || t == JEFFE_TYPETAG_F32 ||
           t == JEFFE_TYPETAG_F64;
}

inline bool is_integral_non_bool(jeffe_typetag tag)
{
    return tag == JEFFE_TYPETAG_CHAR || tag == JEFFE_TYPETAG_I32 || tag == JEFFE_TYPETAG_U32 ||
           tag == JEFFE_TYPETAG_I64 || tag == JEFFE_TYPETAG_U64;
}

template <typename... Args>
inline jeffe_value try_invoke_meta(jeffe_value val, jeffe_op op, Args... args)
{
    if (jeffe_value_typetag(val) == JEFFE_TYPETAG_OBJ)
    {
        objmeta *meta = objmeta::from_value(val);
        if constexpr (sizeof...(args) > 0)
        {
            jeffe_value argv[] = {args...};
            return meta->invoke(op, sizeof...(args), argv);
        }
        else
        {
            return meta->invoke(op, 0, nullptr);
        }
    }
    return jeffe_value_errnum(JEFFE_ERRNO_NOTIMPL, jeffe_builtin_strerror);
}
