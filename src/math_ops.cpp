#include "math_helpers.hpp"

template <typename F, typename P>
static jeffe_value invoke_op(jeffe_value a, jeffe_value b, jeffe_op op, jeffe_op rop,
                             P primitive_fn, F validate)
{
    jeffe_value ret = try_invoke_meta(a, op, b);
    if (!(jeffe_value_typetag(ret) == JEFFE_TYPETAG_ERRNUM &&
          jeffe_value_held_data(ret).errnum.errnum == JEFFE_ERRNO_NOTIMPL))
    {
        return ret;
    }

    ret = try_invoke_meta(b, rop, a);
    if (!(jeffe_value_typetag(ret) == JEFFE_TYPETAG_ERRNUM &&
          jeffe_value_held_data(ret).errnum.errnum == JEFFE_ERRNO_NOTIMPL))
    {
        return ret;
    }

    jeffe_typetag tagA = jeffe_value_typetag(a);
    jeffe_typetag tagB = jeffe_value_typetag(b);

    if (!validate(tagA, tagB))
    {
        return jeffe_value_errnum(JEFFE_ERRNO_UNDEFINED, jeffe_builtin_strerror);
    }

    jeffe_value_holder holderA = jeffe_value_held_data(a);
    jeffe_value_holder holderB = jeffe_value_held_data(b);

    jeffe_typetag common = get_common_type(tagA, tagB);
    jeffe_value_holder promA = cast_to(holderA, common);
    jeffe_value_holder promB = cast_to(holderB, common);
    return primitive_fn(promA, promB);
}

template <typename Op>
static jeffe_value apply_primitive_op(const jeffe_value_holder &a, const jeffe_value_holder &b, Op op)
{
    jeffe_value_holder res;
    res.typetag = a.typetag;
    switch (a.typetag)
    {
    case JEFFE_TYPETAG_I32: res.i32 = op(a.i32, b.i32); break;
    case JEFFE_TYPETAG_U32: res.u32 = op(a.u32, b.u32); break;
    case JEFFE_TYPETAG_I64: res.i64 = op(a.i64, b.i64); break;
    case JEFFE_TYPETAG_U64: res.u64 = op(a.u64, b.u64); break;
    case JEFFE_TYPETAG_F32: res.f32 = op(a.f32, b.f32); break;
    case JEFFE_TYPETAG_F64: res.f64 = op(a.f64, b.f64); break;
    default: break;
    }
    return build_from_promoted(res);
}

template <typename Op>
static jeffe_value apply_primitive_bit_op(const jeffe_value_holder &a, const jeffe_value_holder &b, Op op)
{
    jeffe_value_holder res;
    res.typetag = a.typetag;
    switch (a.typetag)
    {
    case JEFFE_TYPETAG_I32: res.i32 = op(a.i32, b.i32); break;
    case JEFFE_TYPETAG_U32: res.u32 = op(a.u32, b.u32); break;
    case JEFFE_TYPETAG_I64: res.i64 = op(a.i64, b.i64); break;
    case JEFFE_TYPETAG_U64: res.u64 = op(a.u64, b.u64); break;
    default: break;
    }
    return build_from_promoted(res);
}

struct jeffe_mod_op
{
    template <typename T>
    auto operator()(T x, T y) const -> decltype(x % y)
    {
        return x % y;
    }
    float operator()(float x, float y) const
    {
        return std::fmod(x, y);
    }
    double operator()(double x, double y) const
    {
        return std::fmod(x, y);
    }
};

static bool validate_numeric(jeffe_typetag tA, jeffe_typetag tB)
{
    return tA != JEFFE_TYPETAG_BOOL && tB != JEFFE_TYPETAG_BOOL &&
           is_numeric_or_char(tA) && is_numeric_or_char(tB);
}

static bool validate_bitwise(jeffe_typetag tA, jeffe_typetag tB)
{
    return is_integral_non_bool(tA) && is_integral_non_bool(tB);
}

jeffe_value jeffe_add(jeffe_value a, jeffe_value b)
{
    return invoke_op(a, b, JEFFE_OP_ADD, JEFFE_OP_RADD,
                     [](const jeffe_value_holder &pA, const jeffe_value_holder &pB) {
                         return apply_primitive_op(pA, pB, [](auto x, auto y) { return x + y; });
                     },
                     validate_numeric);
}

jeffe_value jeffe_sub(jeffe_value a, jeffe_value b)
{
    return invoke_op(a, b, JEFFE_OP_SUB, JEFFE_OP_RSUB,
                     [](const jeffe_value_holder &pA, const jeffe_value_holder &pB) {
                         return apply_primitive_op(pA, pB, [](auto x, auto y) { return x - y; });
                     },
                     validate_numeric);
}

jeffe_value jeffe_mul(jeffe_value a, jeffe_value b)
{
    return invoke_op(a, b, JEFFE_OP_MUL, JEFFE_OP_RMUL,
                     [](const jeffe_value_holder &pA, const jeffe_value_holder &pB) {
                         return apply_primitive_op(pA, pB, [](auto x, auto y) { return x * y; });
                     },
                     validate_numeric);
}

jeffe_value jeffe_div(jeffe_value a, jeffe_value b)
{
    return invoke_op(a, b, JEFFE_OP_DIV, JEFFE_OP_RDIV,
                     [](const jeffe_value_holder &pA, const jeffe_value_holder &pB) {
                         return apply_primitive_op(pA, pB, [](auto x, auto y) { return x / y; });
                     },
                     validate_numeric);
}

jeffe_value jeffe_mod(jeffe_value a, jeffe_value b)
{
    return invoke_op(a, b, JEFFE_OP_MOD, JEFFE_OP_RMOD,
                     [](const jeffe_value_holder &pA, const jeffe_value_holder &pB) {
                         return apply_primitive_op(pA, pB, jeffe_mod_op{});
                     },
                     validate_numeric);
}

jeffe_value jeffe_radd(jeffe_value a, jeffe_value b)
{
    return try_invoke_meta(a, JEFFE_OP_RADD, b);
}

jeffe_value jeffe_rsub(jeffe_value a, jeffe_value b)
{
    return try_invoke_meta(a, JEFFE_OP_RSUB, b);
}

jeffe_value jeffe_rmul(jeffe_value a, jeffe_value b)
{
    return try_invoke_meta(a, JEFFE_OP_RMUL, b);
}

jeffe_value jeffe_rdiv(jeffe_value a, jeffe_value b)
{
    return try_invoke_meta(a, JEFFE_OP_RDIV, b);
}

jeffe_value jeffe_rmod(jeffe_value a, jeffe_value b)
{
    return try_invoke_meta(a, JEFFE_OP_RMOD, b);
}

jeffe_value jeffe_log_not(jeffe_value a)
{
    jeffe_value ret = try_invoke_meta(a, JEFFE_OP_LOG_NOT);
    if (!(jeffe_value_typetag(ret) == JEFFE_TYPETAG_ERRNUM &&
          jeffe_value_held_data(ret).errnum.errnum == JEFFE_ERRNO_NOTIMPL))
    {
        return ret;
    }

    jeffe_value_holder h = jeffe_value_held_data(a);
    bool truthy = true;
    switch (h.typetag)
    {
    case JEFFE_TYPETAG_NIL: truthy = false; break;
    case JEFFE_TYPETAG_BOOL: truthy = h.b; break;
    case JEFFE_TYPETAG_CHAR: truthy = (h.c != 0); break;
    case JEFFE_TYPETAG_I32: truthy = (h.i32 != 0); break;
    case JEFFE_TYPETAG_U32: truthy = (h.u32 != 0); break;
    case JEFFE_TYPETAG_I64: truthy = (h.i64 != 0); break;
    case JEFFE_TYPETAG_U64: truthy = (h.u64 != 0); break;
    case JEFFE_TYPETAG_F32: truthy = (h.f32 != 0.0f); break;
    case JEFFE_TYPETAG_F64: truthy = (h.f64 != 0.0); break;
    case JEFFE_TYPETAG_PTR: truthy = (h.ptr != nullptr); break;
    case JEFFE_TYPETAG_CSTRUCT: truthy = (h.cstruct != nullptr); break;
    default: truthy = true; break;
    }
    return jeffe_value_bool(!truthy);
}

jeffe_value jeffe_shl(jeffe_value a, jeffe_value b)
{
    return invoke_op(a, b, JEFFE_OP_SHL, JEFFE_OP_SHR,
                     [](const jeffe_value_holder &pA, const jeffe_value_holder &pB) {
                         return apply_primitive_bit_op(pA, pB, [](auto x, auto y) { return x << y; });
                     },
                     validate_bitwise);
}

jeffe_value jeffe_shr(jeffe_value a, jeffe_value b)
{
    return invoke_op(a, b, JEFFE_OP_SHR, JEFFE_OP_SHL,
                     [](const jeffe_value_holder &pA, const jeffe_value_holder &pB) {
                         return apply_primitive_bit_op(pA, pB, [](auto x, auto y) { return x >> y; });
                     },
                     validate_bitwise);
}

jeffe_value jeffe_bit_and(jeffe_value a, jeffe_value b)
{
    return invoke_op(a, b, JEFFE_OP_BIT_AND, JEFFE_OP_BIT_AND,
                     [](const jeffe_value_holder &pA, const jeffe_value_holder &pB) {
                         return apply_primitive_bit_op(pA, pB, [](auto x, auto y) { return x & y; });
                     },
                     validate_bitwise);
}

jeffe_value jeffe_bit_or(jeffe_value a, jeffe_value b)
{
    return invoke_op(a, b, JEFFE_OP_BIT_OR, JEFFE_OP_BIT_OR,
                     [](const jeffe_value_holder &pA, const jeffe_value_holder &pB) {
                         return apply_primitive_bit_op(pA, pB, [](auto x, auto y) { return x | y; });
                     },
                     validate_bitwise);
}

jeffe_value jeffe_bit_not(jeffe_value a)
{
    jeffe_value ret = try_invoke_meta(a, JEFFE_OP_BIT_NOT);
    if (!(jeffe_value_typetag(ret) == JEFFE_TYPETAG_ERRNUM &&
          jeffe_value_held_data(ret).errnum.errnum == JEFFE_ERRNO_NOTIMPL))
    {
        return ret;
    }

    jeffe_typetag tag = jeffe_value_typetag(a);
    if (!is_integral_non_bool(tag))
    {
        return jeffe_value_errnum(JEFFE_ERRNO_UNDEFINED, jeffe_builtin_strerror);
    }

    jeffe_value_holder holderA = jeffe_value_held_data(a);
    jeffe_value_holder prom;
    prom.typetag = holderA.typetag == JEFFE_TYPETAG_CHAR ? JEFFE_TYPETAG_U32 : holderA.typetag;

    switch (prom.typetag)
    {
    case JEFFE_TYPETAG_U32:
        prom.u32 = ~static_cast<uint32_t>(holderA.typetag == JEFFE_TYPETAG_CHAR ? holderA.c : holderA.u32);
        break;
    case JEFFE_TYPETAG_I32: prom.i32 = ~holderA.i32; break;
    case JEFFE_TYPETAG_I64: prom.i64 = ~holderA.i64; break;
    case JEFFE_TYPETAG_U64: prom.u64 = ~holderA.u64; break;
    default: break;
    }

    return build_from_promoted(prom);
}
