#include "util.hpp"

TEST_CASE("tuple creation and length") {
    // Empty tuple
    struct jeffe_value empty = jeffe_tuple(0, nullptr);
    CHECK(std::string(jeffe_typename(empty)) == "tuple");
    
    struct jeffe_value len_val = jeffe_len(empty);
    CHECK(jeffe_value_typetag(len_val) == JEFFE_TYPETAG_U64);
    CHECK(jeffe_value_held_data(len_val).u64 == 0);
    
    jeffe_destroy(empty);

    // Multi-element tuple
    struct jeffe_value elements[3] = {
        jeffe_value_i32(10),
        jeffe_value_bool(true),
        jeffe_value_f64(3.14)
    };
    struct jeffe_value t = jeffe_tuple(3, elements);
    
    len_val = jeffe_len(t);
    CHECK(jeffe_value_typetag(len_val) == JEFFE_TYPETAG_U64);
    CHECK(jeffe_value_held_data(len_val).u64 == 3);
    
    jeffe_destroy(t);
}

TEST_CASE("tuple element access") {
    struct jeffe_value elements[3] = {
        jeffe_value_i32(10),
        jeffe_value_bool(true),
        jeffe_value_f64(3.14)
    };
    struct jeffe_value t = jeffe_tuple(3, elements);
    
    // Valid gets
    struct jeffe_value v0 = jeffe_get(t, jeffe_value_i32(0));
    CHECK(jeffe_value_typetag(v0) == JEFFE_TYPETAG_I32);
    CHECK(jeffe_value_held_data(v0).i32 == 10);
    
    struct jeffe_value v1 = jeffe_get(t, jeffe_value_i32(1));
    CHECK(jeffe_value_typetag(v1) == JEFFE_TYPETAG_BOOL);
    CHECK(jeffe_value_held_data(v1).b == true);
    
    struct jeffe_value v2 = jeffe_get(t, jeffe_value_i32(2));
    CHECK(jeffe_value_typetag(v2) == JEFFE_TYPETAG_F64);
    CHECK(jeffe_value_held_data(v2).f64 == 3.14);
    jeffe_destroy(v2); // Heap allocated f64 needs destruction
    
    // Out of bounds get
    struct jeffe_value err = jeffe_get(t, jeffe_value_i32(3));
    CHECK(jeffe_value_typetag(err) == JEFFE_TYPETAG_ERRNUM);
    CHECK(jeffe_value_held_data(err).errnum.errnum == JEFFE_ERRNO_OUTOFBOUNDS);
    
    // Out of bounds negative index get
    struct jeffe_value err_neg = jeffe_get(t, jeffe_value_i32(-1));
    CHECK(jeffe_value_typetag(err_neg) == JEFFE_TYPETAG_ERRNUM);
    CHECK(jeffe_value_held_data(err_neg).errnum.errnum == JEFFE_ERRNO_OUTOFBOUNDS);

    // Invalid index type get
    struct jeffe_value err_type = jeffe_get(t, jeffe_value_bool(true));
    CHECK(jeffe_value_typetag(err_type) == JEFFE_TYPETAG_ERRNUM);
    CHECK(jeffe_value_held_data(err_type).errnum.errnum == JEFFE_ERRNO_ARGTYPE);
    
    // Direct invoke argument count checks
    jeffe_value_holder t_holder = jeffe_value_held_data(t);
    void **userdata = t_holder.obj.userdata;
    
    struct jeffe_value err_argn0 = jeffe_tuple_class_fn(userdata, JEFFE_OP_GET, 0, nullptr);
    CHECK(jeffe_value_typetag(err_argn0) == JEFFE_TYPETAG_ERRNUM);
    CHECK(jeffe_value_held_data(err_argn0).errnum.errnum == JEFFE_ERRNO_ARGTYPE);
    
    struct jeffe_value args2[2] = {jeffe_value_i32(0), jeffe_value_i32(0)};
    struct jeffe_value err_argn2 = jeffe_tuple_class_fn(userdata, JEFFE_OP_GET, 2, args2);
    CHECK(jeffe_value_typetag(err_argn2) == JEFFE_TYPETAG_ERRNUM);
    CHECK(jeffe_value_held_data(err_argn2).errnum.errnum == JEFFE_ERRNO_ARGTYPE);

    struct jeffe_value err_iter_argn0 = jeffe_tuple_class_fn(userdata, JEFFE_OP_ITER, 0, nullptr);
    CHECK(jeffe_value_typetag(err_iter_argn0) == JEFFE_TYPETAG_ERRNUM);
    CHECK(jeffe_value_held_data(err_iter_argn0).errnum.errnum == JEFFE_ERRNO_ARGTYPE);
    
    jeffe_destroy(t);
}

TEST_CASE("tuple iteration empty") {
    struct jeffe_value empty = jeffe_tuple(0, nullptr);
    struct jeffe_value iter_res = jeffe_iter(empty, jeffe_value_nil());
    CHECK(jeffe_value_typetag(iter_res) == JEFFE_TYPETAG_NIL);
    jeffe_destroy(empty);
}

TEST_CASE("tuple iteration multi-element") {
    struct jeffe_value elements[3] = {
        jeffe_value_i32(100),
        jeffe_value_char('A'),
        jeffe_value_i32(200)
    };
    struct jeffe_value t = jeffe_tuple(3, elements);
    
    // 1st iteration
    struct jeffe_value it1 = jeffe_iter(t, jeffe_value_nil());
    CHECK(std::string(jeffe_typename(it1)) == "tuple");
    struct jeffe_value val1 = jeffe_get(it1, jeffe_value_i32(0));
    struct jeffe_value next1 = jeffe_get(it1, jeffe_value_i32(1));
    
    CHECK(jeffe_value_typetag(val1) == JEFFE_TYPETAG_I32);
    CHECK(jeffe_value_held_data(val1).i32 == 100);
    
    CHECK(jeffe_value_typetag(next1) != JEFFE_TYPETAG_NIL);
    
    // 2nd iteration
    struct jeffe_value it2 = jeffe_iter(t, next1);
    CHECK(std::string(jeffe_typename(it2)) == "tuple");
    struct jeffe_value val2 = jeffe_get(it2, jeffe_value_i32(0));
    struct jeffe_value next2 = jeffe_get(it2, jeffe_value_i32(1));
    
    CHECK(jeffe_value_typetag(val2) == JEFFE_TYPETAG_CHAR);
    CHECK(jeffe_value_held_data(val2).c == 'A');
    
    CHECK(jeffe_value_typetag(next2) != JEFFE_TYPETAG_NIL);
    
    // 3rd iteration (last)
    struct jeffe_value it3 = jeffe_iter(t, next2);
    CHECK(std::string(jeffe_typename(it3)) == "tuple");
    struct jeffe_value val3 = jeffe_get(it3, jeffe_value_i32(0));
    struct jeffe_value next3 = jeffe_get(it3, jeffe_value_i32(1));
    
    CHECK(jeffe_value_typetag(val3) == JEFFE_TYPETAG_I32);
    CHECK(jeffe_value_held_data(val3).i32 == 200);
    
    CHECK(jeffe_value_typetag(next3) == JEFFE_TYPETAG_NIL);
    
    // Post-last iteration (starts iteration again since next3 is nil)
    struct jeffe_value it4 = jeffe_iter(t, next3);
    CHECK(std::string(jeffe_typename(it4)) == "tuple");
    struct jeffe_value val4 = jeffe_get(it4, jeffe_value_i32(0));
    CHECK(jeffe_value_typetag(val4) == JEFFE_TYPETAG_I32);
    CHECK(jeffe_value_held_data(val4).i32 == 100);
    jeffe_destroy(it4);
    
    // Destroy iteration result objects
    jeffe_destroy(it1);
    jeffe_destroy(it2);
    jeffe_destroy(it3);
    
    // Destroy the main tuple
    jeffe_destroy(t);
}

TEST_CASE("tuple heap element destruction") {
    // Create elements: heap types (i64, u64, f64, cstruct)
    struct jeffe_value elements[4] = {
        jeffe_value_i64(1234567890123LL),
        jeffe_value_u64(9876543210987ULL),
        jeffe_value_f64(2.7182818284),
        jeffe_value_cstruct(32)
    };
    
    // Create tuple: this makes copies of all elements
    // Heap primitives are deep-copied
    struct jeffe_value t = jeffe_tuple(4, elements);
    
    // Destroy original heap primitive elements since deep copies were made
    jeffe_destroy(elements[0]);
    jeffe_destroy(elements[1]);
    jeffe_destroy(elements[2]);
    jeffe_destroy(elements[3]);
    
    // Now destroy the tuple: this should destroy all its internal values
    jeffe_destroy(t);
}

TEST_CASE("tuple comparative comparison") {
    // equal tuples
    jeffe_value el1[2] = {jeffe_value_i32(1), jeffe_value_i32(2)};
    jeffe_value t1 = jeffe_tuple(2, el1);

    jeffe_value el2[2] = {jeffe_value_i32(1), jeffe_value_i32(2)};
    jeffe_value t2 = jeffe_tuple(2, el2);

    jeffe_value c_eq = jeffe_cmp(t1, t2);
    CHECK(jeffe_value_typetag(c_eq) == JEFFE_TYPETAG_I32);
    CHECK(jeffe_value_held_data(c_eq).i32 == 0);

    // unequal tuples (different values)
    jeffe_value el3[2] = {jeffe_value_i32(1), jeffe_value_i32(3)};
    jeffe_value t3 = jeffe_tuple(2, el3);

    jeffe_value c_lt = jeffe_cmp(t1, t3);
    CHECK(jeffe_value_typetag(c_lt) == JEFFE_TYPETAG_I32);
    CHECK(jeffe_value_held_data(c_lt).i32 == -1); // t1 < t3 since 2 < 3

    jeffe_value c_gt = jeffe_cmp(t3, t1);
    CHECK(jeffe_value_typetag(c_gt) == JEFFE_TYPETAG_I32);
    CHECK(jeffe_value_held_data(c_gt).i32 == 1); // t3 > t1 since 3 > 2

    // unequal tuples (different lengths)
    jeffe_value el4[3] = {jeffe_value_i32(1), jeffe_value_i32(2), jeffe_value_i32(3)};
    jeffe_value t4 = jeffe_tuple(3, el4);

    jeffe_value c_len_lt = jeffe_cmp(t1, t4);
    CHECK(jeffe_value_typetag(c_len_lt) == JEFFE_TYPETAG_I32);
    CHECK(jeffe_value_held_data(c_len_lt).i32 == -1); // t1 < t4 since t1 is prefix of t4 but shorter

    jeffe_value c_len_gt = jeffe_cmp(t4, t1);
    CHECK(jeffe_value_typetag(c_len_gt) == JEFFE_TYPETAG_I32);
    CHECK(jeffe_value_held_data(c_len_gt).i32 == 1); // t4 > t1 since t4 is prefix of t1 but longer

    jeffe_destroy(t1);
    jeffe_destroy(t2);
    jeffe_destroy(t3);
    jeffe_destroy(t4);

    // tuples containing pointer elements (orderable)
    jeffe_value p1 = jeffe_value_ptr(reinterpret_cast<void*>(0x1234));
    jeffe_value p2 = jeffe_value_ptr(reinterpret_cast<void*>(0x5678));
    jeffe_value p3 = jeffe_value_ptr(reinterpret_cast<void*>(0x1234));

    jeffe_value el_non_eq1[1] = {p1};
    jeffe_value t_non_eq1 = jeffe_tuple(1, el_non_eq1);

    jeffe_value el_non_eq2[1] = {p3};
    jeffe_value t_non_eq2 = jeffe_tuple(1, el_non_eq2);

    jeffe_value c_non_eq = jeffe_cmp(t_non_eq1, t_non_eq2);
    CHECK(jeffe_value_typetag(c_non_eq) == JEFFE_TYPETAG_I32);
    CHECK(jeffe_value_held_data(c_non_eq).i32 == 0); // identical pointer elements -> equal tuples (0)

    jeffe_value el_non_ne[1] = {p2};
    jeffe_value t_non_ne = jeffe_tuple(1, el_non_ne);

    jeffe_value c_non_ne = jeffe_cmp(t_non_eq1, t_non_ne);
    CHECK(jeffe_value_typetag(c_non_ne) == JEFFE_TYPETAG_I32);
    CHECK(jeffe_value_held_data(c_non_ne).i32 == -1); // orderable pointer elements (0x1234 < 0x5678) -> -1

    jeffe_destroy(t_non_eq1);
    jeffe_destroy(t_non_eq2);
    jeffe_destroy(t_non_ne);

    // tuples containing cstruct elements (non-orderable)
    jeffe_value cs1 = jeffe_value_cstruct(16);
    jeffe_value cs2 = jeffe_value_cstruct(32);

    jeffe_value el_cs1[1] = {cs1};
    jeffe_value t_cs1 = jeffe_tuple(1, el_cs1);

    jeffe_value el_cs2[1] = {cs2};
    jeffe_value t_cs2 = jeffe_tuple(1, el_cs2);

    jeffe_value c_cs = jeffe_cmp(t_cs1, t_cs2);
    CHECK(jeffe_value_typetag(c_cs) == JEFFE_TYPETAG_ERRNUM);
    CHECK(jeffe_value_held_data(c_cs).errnum.errnum == JEFFE_ERRNO_NOTORDERED); // different cstructs -> not orderable error

    jeffe_destroy(cs1);
    jeffe_destroy(cs2);
    jeffe_destroy(t_cs1);
    jeffe_destroy(t_cs2);
}


