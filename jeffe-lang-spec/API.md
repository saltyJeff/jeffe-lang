See [[ABI]] which describes the API for the primitive/object factory methods.

Each operator (see [[Operators]]) is defined on the `struct jeffe_value` type.

For objects, invoke the corresponding operator using the class's `jeffe_class_fn` function.

Each primitive and object factory method has a corresponding C function, e.g.
```c
struct jeffe_value jeffe_value_i32(int32_t i);
struct jeffe_value jeffe_value_errnum(int8_t errnum, jeffe_strerror_fn strerror_func);
struct jeffe_value jeffe_value_obj(jeffe_class_fn fn, size_t argn, const struct jeffe_value *argv);
```