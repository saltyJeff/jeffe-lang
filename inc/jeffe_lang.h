#ifndef JEFFE_LANG_H_
#define JEFFE_LANG_H_

#if defined(JEFFE_STATIC)
#define JEFFE_API
#elif defined(_WIN32) || defined(__CYGWIN__)
#ifdef JEFFE_EXPORT
#define JEFFE_API __declspec(dllexport)
#else
#define JEFFE_API __declspec(dllimport)
#endif
#else
#if __GNUC__ >= 4
#define JEFFE_API __attribute__((visibility("default")))
#else
#define JEFFE_API
#endif
#endif

#include <stdbool.h>
#include <stdint.h>
#include <uchar.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * A type representing a 64-bit tagged value.
 */
struct jeffe_value
{
    uint64_t v;
};
/**
 * Creates a new nil value.
 * @return the nil value
 */
JEFFE_API struct jeffe_value jeffe_value_nil();
/**
 * Creates a new char value.
 * @param c the 32-bit character
 * @return the char value
 */
JEFFE_API struct jeffe_value jeffe_value_char(char32_t c);

/**
 * Creates a new boolean value.
 * @param b the boolean
 * @return the boolean value
 */
JEFFE_API struct jeffe_value jeffe_value_bool(bool b);

/**
 * Creates a new 32-bit signed integer value.
 * @param i the 32-bit signed integer
 * @return the i32 value
 */
JEFFE_API struct jeffe_value jeffe_value_i32(int32_t i);

/**
 * Creates a new 32-bit unsigned integer value.
 * @param ui the 32-bit unsigned integer
 * @return the u32 value
 */
JEFFE_API struct jeffe_value jeffe_value_u32(uint32_t ui);

/**
 * Creates a new 32-bit float value.
 * @param f the 32-bit float
 * @return the f32 value
 */
JEFFE_API struct jeffe_value jeffe_value_f32(float f);

/**
 * Creates a new 64-bit float value.
 * @param d the 64-bit float
 * @return the f64 value
 */
JEFFE_API struct jeffe_value jeffe_value_f64(double d);

/**
 * Creates a new 64-bit signed integer value.
 * @param l the 64-bit signed integer
 * @return the i64 value
 */
JEFFE_API struct jeffe_value jeffe_value_i64(int64_t l);

/**
 * Creates a new 64-bit unsigned integer value.
 * @param ul the 64-bit unsigned integer
 * @return the u64 value
 */
JEFFE_API struct jeffe_value jeffe_value_u64(uint64_t ul);

/**
 * Creates a new pointer value.
 * @param ptr the pointer
 * @return the ptr value
 */
JEFFE_API struct jeffe_value jeffe_value_ptr(void *ptr);

/**
 * Creates a new cstruct value.
 * @param sz the size of the structure
 * @return the cstruct value
 */
JEFFE_API struct jeffe_value jeffe_value_cstruct(size_t sz);
/**
 * A function type for stringifying error numbers.
 * @param err the error number
 * @return the error string. The string must have a global lifetime
 */
typedef const char *(*jeffe_strerror_fn)(int8_t err);

/**
 * Creates a new errnum value.
 * @param err the error number
 * @param fn the stringify function
 * @return the errnum value
 */
JEFFE_API struct jeffe_value jeffe_value_errnum(int8_t err, jeffe_strerror_fn fn);
/**
 * Represents a class definition function
 * @warning argv is caller-owned. Use assign/clone in order to extend the lifetime of arguments.
 */
typedef struct jeffe_value (*jeffe_class_fn)(void **userdata, int op, size_t argn, const struct jeffe_value *argv);
/**
 * Creates a new object value.
 * @param fn the class definition function
 * @param argn the number of arguments
 * @param argv the arguments
 * @return the object value
 */
JEFFE_API struct jeffe_value jeffe_value_obj(jeffe_class_fn fn, size_t argn, const struct jeffe_value *argv);
/**
 * Represents the tag of a value type.
 */
enum jeffe_typetag
{
    JEFFE_TYPETAG_NIL,
    JEFFE_TYPETAG_CHAR,
    JEFFE_TYPETAG_BOOL,
    JEFFE_TYPETAG_I32,
    JEFFE_TYPETAG_U32,
    JEFFE_TYPETAG_F32,
    JEFFE_TYPETAG_F64,
    JEFFE_TYPETAG_I64,
    JEFFE_TYPETAG_U64,
    JEFFE_TYPETAG_PTR,
    JEFFE_TYPETAG_CSTRUCT,
    JEFFE_TYPETAG_ERRNUM,
    JEFFE_TYPETAG_OBJ
};
/**
 * Returns the typetag for a value
 * @param v the value
 * @return the value's typetag
 */
JEFFE_API enum jeffe_typetag jeffe_value_typetag(struct jeffe_value v);

/**
 * Returns the string name of a typetag
 * @param tag the typetag
 * @return the typetag's name
 */
JEFFE_API const char *jeffe_strtypetag(enum jeffe_typetag tag);

struct jeffe_value_holder
{
    enum jeffe_typetag typetag;
    union
    {
        char32_t c;
        bool b;
        int32_t i32;
        uint32_t u32;
        float f32;
        double f64;
        int64_t i64;
        uint64_t u64;
        void *ptr;
        void *cstruct;
        struct
        {
            int8_t errnum;
            jeffe_strerror_fn fn;
        } errnum;
        struct
        {
            void **userdata;
            jeffe_class_fn fn;
        } obj;
    };
};
/**
 * Gets the data held in a jeffe_value.
 * Use jeffe_value_type() to determine which member to access.
 * @param v the value
 * @return the held data
 */
JEFFE_API struct jeffe_value_holder jeffe_value_held_data(struct jeffe_value v);

/**
 * Represents an operation passed to a class definition function.
 */
enum jeffe_op
{
    JEFFE_OP_CTOR,
    JEFFE_OP_DTOR,
    JEFFE_OP_GET,
    JEFFE_OP_SET,
    JEFFE_OP_IS_ERR,
    JEFFE_OP_CLASSNAME
};
/**
 * Destroys a jeffe_value.
 * @warning do not use the passed-in jeffe value afterwards.
 * @param val the value to destroy
 */
JEFFE_API void jeffe_destroy(struct jeffe_value val);
/**
 * Performs a = rhs.
 * @param rhs
 * @return the assigned value
 */
JEFFE_API struct jeffe_value jeffe_copy(struct jeffe_value rhs);
/**
 * Performs obj[key]
 * @param obj the object
 * @param key the key
 * @return the resulting value
 */
JEFFE_API struct jeffe_value jeffe_get(struct jeffe_value obj, struct jeffe_value key);
/**
 * Performs obj[key] = val
 * @param obj the object
 * @param key the key
 * @param val the value to set
 * @return the resulting value (typically nil)
 */
JEFFE_API struct jeffe_value jeffe_set(struct jeffe_value obj, struct jeffe_value key, struct jeffe_value val);
/**
 * Checks if the value should be interpreted as an error.
 * True if the value is an errnum, or an object which returns true for JEFFE_OP_IS_ERR
 * @param v the value
 * @return true if the value is an error
 */
JEFFE_API bool jeffe_is_err(struct jeffe_value v);
/**
 * Returns the typename of an object
 * @param v the object
 * @return the class name
 */
JEFFE_API const char *jeffe_typename(struct jeffe_value v);

enum jeffe_builtin_errno
{
    JEFFE_ERRNO_UNDEFINED
};
JEFFE_API const char *jeffe_builtin_strerror(int8_t err);
#ifdef __cplusplus
}
#endif
#endif