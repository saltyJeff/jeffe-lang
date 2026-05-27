#include "util.hpp"
#include <cmath>

static jeffe_value mock_math_class_fn(void **userdata, int op, size_t argn, const jeffe_value *argv)
{
    if (op == JEFFE_OP_CTOR)
    {
        if (argn == 1)
        {
            int32_t *val = new int32_t(jeffe_value_held_data(argv[0]).i32);
            *userdata = val;
        }
        return jeffe_value_nil();
    }
    if (op == JEFFE_OP_DTOR)
    {
        if (*userdata)
        {
            delete static_cast<int32_t *>(*userdata);
            *userdata = nullptr;
        }
        return jeffe_value_nil();
    }
    if (op == JEFFE_OP_ADD)
    {
        if (argn == 1)
        {
            int32_t val = *static_cast<int32_t *>(*userdata);
            jeffe_value rhs = argv[0];
            if (jeffe_value_typetag(rhs) == JEFFE_TYPETAG_I32)
            {
                return jeffe_value_i32(val + jeffe_value_held_data(rhs).i32);
            }
        }
    }
    if (op == JEFFE_OP_RADD)
    {
        if (argn == 1)
        {
            int32_t val = *static_cast<int32_t *>(*userdata);
            jeffe_value lhs = argv[0];
            if (jeffe_value_typetag(lhs) == JEFFE_TYPETAG_I32)
            {
                return jeffe_value_i32(jeffe_value_held_data(lhs).i32 + val);
            }
        }
    }
    if (op == JEFFE_OP_CMP)
    {
        if (argn == 1)
        {
            int32_t val = *static_cast<int32_t *>(*userdata);
            jeffe_value rhs = argv[0];
            if (jeffe_value_typetag(rhs) == JEFFE_TYPETAG_I32)
            {
                int32_t rval = jeffe_value_held_data(rhs).i32;
                if (val < rval) return jeffe_value_i32(-1);
                if (val > rval) return jeffe_value_i32(1);
                return jeffe_value_i32(0);
            }
        }
    }
    return jeffe_value_errnum(JEFFE_ERRNO_NOTIMPL, jeffe_builtin_strerror);
}

TEST_CASE("math_arithmetic_primitives")
{
    // Integer addition
    jeffe_value i1 = jeffe_value_i32(10);
    jeffe_value i2 = jeffe_value_i32(20);
    jeffe_value r_add = jeffe_add(i1, i2);
    CHECK(jeffe_value_typetag(r_add) == JEFFE_TYPETAG_I32);
    CHECK(jeffe_value_held_data(r_add).i32 == 30);

    // Float-integer mixed addition (promotes to F32 under standard rules)
    jeffe_value f1 = jeffe_value_f32(1.5f);
    jeffe_value r_mixed = jeffe_add(f1, i1);
    CHECK(jeffe_value_typetag(r_mixed) == JEFFE_TYPETAG_F32);
    CHECK(jeffe_value_held_data(r_mixed).f32 == 11.5f);



    // Modulo float support
    jeffe_value f_div = jeffe_value_f32(5.5f);
    jeffe_value f_mod = jeffe_value_f32(2.0f);
    jeffe_value r_mod = jeffe_mod(f_div, f_mod);
    CHECK(jeffe_value_typetag(r_mod) == JEFFE_TYPETAG_F32);
    CHECK(jeffe_value_held_data(r_mod).f32 == 1.5f);

    // Boolean mathematics is forbidden
    jeffe_value b1 = jeffe_value_bool(true);
    jeffe_value err_bool = jeffe_add(b1, i1);
    CHECK(jeffe_value_typetag(err_bool) == JEFFE_TYPETAG_ERRNUM);
    CHECK(jeffe_value_held_data(err_bool).errnum.errnum == JEFFE_ERRNO_UNDEFINED);
}

TEST_CASE("math_bitwise_primitives")
{
    jeffe_value val1 = jeffe_value_i32(0b1100);
    jeffe_value val2 = jeffe_value_i32(0b1010);

    // Bitwise AND
    jeffe_value r_and = jeffe_bit_and(val1, val2);
    CHECK(jeffe_value_typetag(r_and) == JEFFE_TYPETAG_I32);
    CHECK(jeffe_value_held_data(r_and).i32 == 0b1000);

    // Bitwise OR
    jeffe_value r_or = jeffe_bit_or(val1, val2);
    CHECK(jeffe_value_typetag(r_or) == JEFFE_TYPETAG_I32);
    CHECK(jeffe_value_held_data(r_or).i32 == 0b1110);

    // Bitwise NOT
    jeffe_value r_not = jeffe_bit_not(jeffe_value_i32(0));
    CHECK(jeffe_value_typetag(r_not) == JEFFE_TYPETAG_I32);
    CHECK(jeffe_value_held_data(r_not).i32 == -1);

    // Bitwise shift left
    jeffe_value r_shl = jeffe_shl(jeffe_value_i32(1), jeffe_value_i32(3));
    CHECK(jeffe_value_typetag(r_shl) == JEFFE_TYPETAG_I32);
    CHECK(jeffe_value_held_data(r_shl).i32 == 8);

    // Bitwise on float fails
    jeffe_value float_val = jeffe_value_f32(1.0f);
    jeffe_value err_bit = jeffe_bit_and(float_val, val1);
    CHECK(jeffe_value_typetag(err_bit) == JEFFE_TYPETAG_ERRNUM);
}

TEST_CASE("math_logical_not")
{
    // Bool truthy / falsy
    CHECK(jeffe_value_held_data(jeffe_log_not(jeffe_value_bool(true))).b == false);
    CHECK(jeffe_value_held_data(jeffe_log_not(jeffe_value_bool(false))).b == true);

    // Integer truthy / falsy
    CHECK(jeffe_value_held_data(jeffe_log_not(jeffe_value_i32(42))).b == false);
    CHECK(jeffe_value_held_data(jeffe_log_not(jeffe_value_i32(0))).b == true);

    // Nil falsy
    CHECK(jeffe_value_held_data(jeffe_log_not(jeffe_value_nil())).b == true);
}

TEST_CASE("math_comparisons")
{
    // Numeric comparisons
    jeffe_value c1 = jeffe_cmp(jeffe_value_i32(5), jeffe_value_i32(10));
    CHECK(jeffe_value_held_data(c1).i32 == -1);

    jeffe_value c2 = jeffe_cmp(jeffe_value_i32(10), jeffe_value_i32(5));
    CHECK(jeffe_value_held_data(c2).i32 == 1);

    jeffe_value c3 = jeffe_cmp(jeffe_value_i32(5), jeffe_value_i32(5));
    CHECK(jeffe_value_held_data(c3).i32 == 0);

    // Bool comparisons (false < true)
    jeffe_value c_bool = jeffe_cmp(jeffe_value_bool(false), jeffe_value_bool(true));
    CHECK(jeffe_value_held_data(c_bool).i32 == -1);

    // Total ordering for different types
    jeffe_value c_diff = jeffe_cmp(jeffe_value_bool(true), jeffe_value_i32(10));
    CHECK(jeffe_value_held_data(c_diff).i32 != 0);
}

TEST_CASE("math_objects")
{
    // Object arithmetic
    jeffe_value initial = jeffe_value_i32(100);
    jeffe_value obj = jeffe_value_obj(mock_math_class_fn, 1, &initial);

    jeffe_value operand = jeffe_value_i32(50);
    jeffe_value res_add = jeffe_add(obj, operand);
    CHECK(jeffe_value_typetag(res_add) == JEFFE_TYPETAG_I32);
    CHECK(jeffe_value_held_data(res_add).i32 == 150);

    // Reverse arithmetic: primitive + object
    jeffe_value res_radd = jeffe_add(operand, obj);
    CHECK(jeffe_value_typetag(res_radd) == JEFFE_TYPETAG_I32);
    CHECK(jeffe_value_held_data(res_radd).i32 == 150);

    // Object comparison
    jeffe_value cmp_res = jeffe_cmp(obj, jeffe_value_i32(120));
    CHECK(jeffe_value_held_data(cmp_res).i32 == -1);

    jeffe_value cmp_res_r = jeffe_cmp(jeffe_value_i32(80), obj);
    CHECK(jeffe_value_held_data(cmp_res_r).i32 == -1); // 80 < 100

    jeffe_destroy(obj);
}
