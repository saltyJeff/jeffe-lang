#include "util.hpp"
#include <string>

static struct jeffe_value err_obj_class_fn(void **userdata, int op, size_t argn, const struct jeffe_value *argv) {
    if (op == JEFFE_OP_IS_ERR) {
        return jeffe_value_bool(true);
    }
    if (op == JEFFE_OP_CLASSNAME) {
        return jeffe_value_ptr((void*)"ErrorObj");
    }
    return jeffe_value_nil();
}

static struct jeffe_value norm_obj_class_fn(void **userdata, int op, size_t argn, const struct jeffe_value *argv) {
    if (op == JEFFE_OP_IS_ERR) {
        return jeffe_value_bool(false);
    }
    if (op == JEFFE_OP_CLASSNAME) {
        return jeffe_value_ptr((void*)"NormObj");
    }
    return jeffe_value_nil();
}

static const char* dummy_strerror(int8_t err) { return "error"; }

TEST_CASE("jeffe_is_err") {
    // Non-errors
    CHECK_FALSE(jeffe_is_err(jeffe_value_nil()));
    CHECK_FALSE(jeffe_is_err(jeffe_value_i32(0)));
    
    // Builtin error
    struct jeffe_value errnum = jeffe_value_errnum(1, dummy_strerror);
    CHECK(jeffe_is_err(errnum));
    
    // Custom error object
    struct jeffe_value err_obj = jeffe_value_obj(err_obj_class_fn, 0, nullptr);
    CHECK(jeffe_is_err(err_obj));
    jeffe_destroy(err_obj);
    
    // Custom non-error object
    struct jeffe_value norm_obj = jeffe_value_obj(norm_obj_class_fn, 0, nullptr);
    CHECK_FALSE(jeffe_is_err(norm_obj));
    jeffe_destroy(norm_obj);
}

TEST_CASE("jeffe_typename") {
    CHECK(std::string(jeffe_typename(jeffe_value_nil())) == "NIL");
    CHECK(std::string(jeffe_typename(jeffe_value_i32(0))) == "I32");
    
    struct jeffe_value err_obj = jeffe_value_obj(err_obj_class_fn, 0, nullptr);
    CHECK(std::string(jeffe_typename(err_obj)) == "ErrorObj");
    jeffe_destroy(err_obj);
    
    struct jeffe_value norm_obj = jeffe_value_obj(norm_obj_class_fn, 0, nullptr);
    CHECK(std::string(jeffe_typename(norm_obj)) == "NormObj");
    jeffe_destroy(norm_obj);
}
