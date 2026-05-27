#include "doctest.h"
#include <jeffe_lang.h>

TEST_CASE("value nil constructor") {
    struct jeffe_value v = jeffe_value_nil();
    CHECK(jeffe_value_type(v) == JEFFE_TYPETAG_NIL);
}

TEST_CASE("value char constructor") {
    struct jeffe_value v = jeffe_value_char('a');
    CHECK(jeffe_value_type(v) == JEFFE_TYPETAG_CHAR);
}

TEST_CASE("value bool constructor") {
    struct jeffe_value v = jeffe_value_bool(true);
    CHECK(jeffe_value_type(v) == JEFFE_TYPETAG_BOOL);
}

TEST_CASE("value i32 constructor") {
    struct jeffe_value v = jeffe_value_i32(-42);
    CHECK(jeffe_value_type(v) == JEFFE_TYPETAG_I32);
}

TEST_CASE("value u32 constructor") {
    struct jeffe_value v = jeffe_value_u32(42);
    CHECK(jeffe_value_type(v) == JEFFE_TYPETAG_U32);
}

TEST_CASE("value f32 constructor") {
    struct jeffe_value v = jeffe_value_f32(3.14f);
    CHECK(jeffe_value_type(v) == JEFFE_TYPETAG_F32);
}

TEST_CASE("value f64 constructor") {
    struct jeffe_value v = jeffe_value_f64(3.14159);
    CHECK(jeffe_value_type(v) == JEFFE_TYPETAG_F64);
}

TEST_CASE("value i64 constructor") {
    struct jeffe_value v = jeffe_value_i64(-42LL);
    CHECK(jeffe_value_type(v) == JEFFE_TYPETAG_I64);
}

TEST_CASE("value u64 constructor") {
    struct jeffe_value v = jeffe_value_u64(42ULL);
    CHECK(jeffe_value_type(v) == JEFFE_TYPETAG_U64);
}

TEST_CASE("value ptr constructor") {
    void* ptr = (void*)0x12345678;
    struct jeffe_value v = jeffe_value_ptr(ptr);
    CHECK(jeffe_value_type(v) == JEFFE_TYPETAG_PTR);
}

TEST_CASE("value cstruct constructor") {
    struct jeffe_value v = jeffe_value_cstruct(16);
    CHECK(jeffe_value_type(v) == JEFFE_TYPETAG_CSTRUCT);
}

static const char* dummy_strerror(int8_t err) { return "error"; }

TEST_CASE("value errnum constructor") {
    struct jeffe_value v = jeffe_value_errnum(1, dummy_strerror);
    CHECK(jeffe_value_type(v) == JEFFE_TYPETAG_ERRNUM);
}

static struct jeffe_value dummy_class_fn(void **userdata, int op, size_t argn, const struct jeffe_value *argv) {
    return jeffe_value_nil();
}

TEST_CASE("value obj constructor") {
    struct jeffe_value v = jeffe_value_obj(dummy_class_fn, 0, nullptr);
    CHECK(jeffe_value_type(v) == JEFFE_TYPETAG_OBJ);
}