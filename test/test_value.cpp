#include "util.hpp"

TEST_CASE("value nil constructor") {
    struct jeffe_value v = jeffe_value_nil();
    CHECK(jeffe_value_typetag(v) == JEFFE_TYPETAG_NIL);
    struct jeffe_value_holder h = jeffe_value_held_data(v);
    CHECK(h.typetag == JEFFE_TYPETAG_NIL);
}

TEST_CASE("value char constructor") {
    struct jeffe_value v = jeffe_value_char('a');
    CHECK(jeffe_value_typetag(v) == JEFFE_TYPETAG_CHAR);
    struct jeffe_value_holder h = jeffe_value_held_data(v);
    CHECK(h.typetag == JEFFE_TYPETAG_CHAR);
    CHECK(h.c == 'a');
}

TEST_CASE("value bool constructor") {
    struct jeffe_value v = jeffe_value_bool(true);
    CHECK(jeffe_value_typetag(v) == JEFFE_TYPETAG_BOOL);
    struct jeffe_value_holder h = jeffe_value_held_data(v);
    CHECK(h.typetag == JEFFE_TYPETAG_BOOL);
    CHECK(h.b == true);
}

TEST_CASE("value i32 constructor") {
    struct jeffe_value v = jeffe_value_i32(-42);
    CHECK(jeffe_value_typetag(v) == JEFFE_TYPETAG_I32);
    struct jeffe_value_holder h = jeffe_value_held_data(v);
    CHECK(h.typetag == JEFFE_TYPETAG_I32);
    CHECK(h.i32 == -42);
}

TEST_CASE("value u32 constructor") {
    struct jeffe_value v = jeffe_value_u32(42);
    CHECK(jeffe_value_typetag(v) == JEFFE_TYPETAG_U32);
    struct jeffe_value_holder h = jeffe_value_held_data(v);
    CHECK(h.typetag == JEFFE_TYPETAG_U32);
    CHECK(h.u32 == 42);
}

TEST_CASE("value f32 constructor") {
    struct jeffe_value v = jeffe_value_f32(3.14f);
    CHECK(jeffe_value_typetag(v) == JEFFE_TYPETAG_F32);
    struct jeffe_value_holder h = jeffe_value_held_data(v);
    CHECK(h.typetag == JEFFE_TYPETAG_F32);
    CHECK(h.f32 == 3.14f);
}

TEST_CASE("value f64 constructor") {
    struct jeffe_value v = jeffe_value_f64(3.14159);
    CHECK(jeffe_value_typetag(v) == JEFFE_TYPETAG_F64);
    struct jeffe_value_holder h = jeffe_value_held_data(v);
    CHECK(h.typetag == JEFFE_TYPETAG_F64);
    CHECK(h.f64 == 3.14159);
}

TEST_CASE("value i64 constructor") {
    struct jeffe_value v = jeffe_value_i64(-42LL);
    CHECK(jeffe_value_typetag(v) == JEFFE_TYPETAG_I64);
    struct jeffe_value_holder h = jeffe_value_held_data(v);
    CHECK(h.typetag == JEFFE_TYPETAG_I64);
    CHECK(h.i64 == -42LL);
}

TEST_CASE("value u64 constructor") {
    struct jeffe_value v = jeffe_value_u64(42ULL);
    CHECK(jeffe_value_typetag(v) == JEFFE_TYPETAG_U64);
    struct jeffe_value_holder h = jeffe_value_held_data(v);
    CHECK(h.typetag == JEFFE_TYPETAG_U64);
    CHECK(h.u64 == 42ULL);
}

TEST_CASE("value ptr constructor") {
    void* ptr = (void*)0x12345678;
    struct jeffe_value v = jeffe_value_ptr(ptr);
    CHECK(jeffe_value_typetag(v) == JEFFE_TYPETAG_PTR);
    struct jeffe_value_holder h = jeffe_value_held_data(v);
    CHECK(h.typetag == JEFFE_TYPETAG_PTR);
    CHECK(h.ptr == ptr);
}

TEST_CASE("value cstruct constructor") {
    struct jeffe_value v = jeffe_value_cstruct(16);
    CHECK(jeffe_value_typetag(v) == JEFFE_TYPETAG_CSTRUCT);
    struct jeffe_value_holder h = jeffe_value_held_data(v);
    CHECK(h.typetag == JEFFE_TYPETAG_CSTRUCT);
    CHECK(h.cstruct != nullptr);
}

static const char* dummy_strerror(int8_t err) { return "error"; }

TEST_CASE("value errnum constructor") {
    struct jeffe_value v = jeffe_value_errnum(1, dummy_strerror);
    CHECK(jeffe_value_typetag(v) == JEFFE_TYPETAG_ERRNUM);
    struct jeffe_value_holder h = jeffe_value_held_data(v);
    CHECK(h.typetag == JEFFE_TYPETAG_ERRNUM);
    CHECK(h.errnum.errnum == 1);
    CHECK(h.errnum.fn == dummy_strerror);
}

static struct jeffe_value dummy_class_fn(void **userdata, int op, size_t argn, const struct jeffe_value *argv) {
    return jeffe_value_nil();
}

TEST_CASE("value obj constructor") {
    struct jeffe_value v = jeffe_value_obj(dummy_class_fn, 0, nullptr);
    CHECK(jeffe_value_typetag(v) == JEFFE_TYPETAG_OBJ);
    struct jeffe_value_holder h = jeffe_value_held_data(v);
    CHECK(h.typetag == JEFFE_TYPETAG_OBJ);
    CHECK(h.obj.fn == dummy_class_fn);
    CHECK(h.obj.userdata != nullptr);
}

TEST_CASE("jeffe_as_index conversions") {
    intptr_t out = 0;

    // Successful conversions
    CHECK(jeffe_as_index(jeffe_value_char('A'), &out) == true);
    CHECK(out == 65);

    CHECK(jeffe_as_index(jeffe_value_i32(-500), &out) == true);
    CHECK(out == -500);

    CHECK(jeffe_as_index(jeffe_value_u32(1000), &out) == true);
    CHECK(out == 1000);

    if (sizeof(intptr_t) >= 8) {
        CHECK(jeffe_as_index(jeffe_value_i64(5000000000LL), &out) == true);
        CHECK(out == 5000000000LL);

        CHECK(jeffe_as_index(jeffe_value_u64(9000000000ULL), &out) == true);
        CHECK(out == 9000000000ULL);
    } else {
        CHECK(jeffe_as_index(jeffe_value_i64(5000000000LL), &out) == true);
        CHECK(out == static_cast<intptr_t>(5000000000LL));

        CHECK(jeffe_as_index(jeffe_value_u64(9000000000ULL), &out) == true);
        CHECK(out == static_cast<intptr_t>(9000000000ULL));
    }

    // Failures
    CHECK(jeffe_as_index(jeffe_value_nil(), &out) == false);
    CHECK(jeffe_as_index(jeffe_value_bool(true), &out) == false);
    CHECK(jeffe_as_index(jeffe_value_f32(1.5f), &out) == false);
    CHECK(jeffe_as_index(jeffe_value_f64(2.5), &out) == false);
    
    // Null output pointer defensive check
    CHECK(jeffe_as_index(jeffe_value_i32(10), nullptr) == false);
}