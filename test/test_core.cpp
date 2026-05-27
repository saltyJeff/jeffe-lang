#include "util.hpp"

static int dtor_called = 0;
static int get_called = 0;
static int set_called = 0;

static struct jeffe_value core_obj_class_fn(void **userdata, int op, size_t argn, const struct jeffe_value *argv) {
    if (op == JEFFE_OP_DTOR) {
        dtor_called++;
    } else if (op == JEFFE_OP_GET) {
        get_called++;
        if (argn == 1) {
            return argv[0]; // Echo back the key as value
        }
    } else if (op == JEFFE_OP_SET) {
        set_called++;
        if (argn == 2) {
            return argv[1]; // Echo back the val as return
        }
    }
    return jeffe_value_nil();
}

TEST_CASE("jeffe_destroy") {
    dtor_called = 0;
    
    // Destroy an object
    struct jeffe_value obj = jeffe_value_obj(core_obj_class_fn, 0, nullptr);
    jeffe_destroy(obj);
    CHECK(dtor_called == 1);
    
    // Destroy primitives (should not crash)
    jeffe_destroy(jeffe_value_nil());
    jeffe_destroy(jeffe_value_i32(42));
    jeffe_destroy(jeffe_value_bool(true));
    jeffe_destroy(jeffe_value_char('c'));
    
    // Destroy heap types (should not crash)
    jeffe_destroy(jeffe_value_f64(3.14));
    jeffe_destroy(jeffe_value_i64(42));
    jeffe_destroy(jeffe_value_u64(42));
    jeffe_destroy(jeffe_value_cstruct(16));
}

TEST_CASE("jeffe_copy") {
    // Copy primitive
    struct jeffe_value i1 = jeffe_value_i32(42);
    struct jeffe_value i2 = jeffe_copy(i1);
    CHECK(jeffe_value_typetag(i2) == JEFFE_TYPETAG_I32);
    CHECK(jeffe_value_held_data(i2).i32 == 42);
    
    // Copy heap primitives
    struct jeffe_value f1 = jeffe_value_f64(3.14);
    struct jeffe_value f2 = jeffe_copy(f1);
    CHECK(jeffe_value_typetag(f2) == JEFFE_TYPETAG_F64);
    CHECK(jeffe_value_held_data(f2).f64 == 3.14);
    // Compare the actual pointers in the tagged value
    CHECK((f1.v & 0xFFFFFFFFFFFFULL) != (f2.v & 0xFFFFFFFFFFFFULL)); 
    
    jeffe_destroy(f1);
    jeffe_destroy(f2);
    
    // Copy cstruct
    struct jeffe_value c1 = jeffe_value_cstruct(16);
    struct jeffe_value c2 = jeffe_copy(c1);
    CHECK(jeffe_value_typetag(c2) == JEFFE_TYPETAG_CSTRUCT);
    CHECK((c1.v & 0xFFFFFFFFFFFFULL) != (c2.v & 0xFFFFFFFFFFFFULL));
    
    jeffe_destroy(c1);
    jeffe_destroy(c2);
    
    // Copy object (should just copy the ref)
    struct jeffe_value obj1 = jeffe_value_obj(core_obj_class_fn, 0, nullptr);
    struct jeffe_value obj2 = jeffe_copy(obj1);
    CHECK((obj1.v & 0xFFFFFFFFFFFFULL) == (obj2.v & 0xFFFFFFFFFFFFULL));
    
    jeffe_destroy(obj1); // only destroy one since they share the reference
}

TEST_CASE("jeffe_get") {
    get_called = 0;
    
    struct jeffe_value obj = jeffe_value_obj(core_obj_class_fn, 0, nullptr);
    struct jeffe_value key = jeffe_value_i32(123);
    
    struct jeffe_value ret = jeffe_get(obj, key);
    CHECK(get_called == 1);
    CHECK(jeffe_value_typetag(ret) == JEFFE_TYPETAG_I32);
    CHECK(jeffe_value_held_data(ret).i32 == 123);
    
    jeffe_destroy(obj);
    
    // test on non-object
    struct jeffe_value prim = jeffe_value_i32(123);
    struct jeffe_value err = jeffe_get(prim, key);
    CHECK(jeffe_value_typetag(err) == JEFFE_TYPETAG_ERRNUM);
}

TEST_CASE("jeffe_set") {
    set_called = 0;
    
    struct jeffe_value obj = jeffe_value_obj(core_obj_class_fn, 0, nullptr);
    struct jeffe_value key = jeffe_value_i32(123);
    struct jeffe_value val = jeffe_value_f32(1.23f);
    
    struct jeffe_value ret = jeffe_set(obj, key, val);
    CHECK(set_called == 1);
    CHECK(jeffe_value_typetag(ret) == JEFFE_TYPETAG_F32);
    CHECK(jeffe_value_held_data(ret).f32 == 1.23f);
    
    jeffe_destroy(obj);
    
    // test on non-object
    struct jeffe_value prim = jeffe_value_i32(123);
    struct jeffe_value err = jeffe_set(prim, key, val);
    CHECK(jeffe_value_typetag(err) == JEFFE_TYPETAG_ERRNUM);
}
