## Rust
See [[ABI]] which describes the API for the primitive/object factory methods.

Each operator (see [[Operators]]) is defined on the `Value` type.

```rust
impl Value
{
	pub fn operator(&self, arg0: &Value, arg1: &Value) -> Value
	{
		// check if self is a primitive or an object
		// for primitives, create a new Value based on the underlying values
		// for objects, invoke the corresponding operator using class_fn
	}
}
```
## C
> ⚠️UNSTABLE, NOT YET IMPLEMENTED

Each primitive and object factory method has a corresponding C function, e.g.
```c
jeffe_lang_any jeffe_lang_i32(int32_t i);
jeffe_lang_any jeffe_lang_errnum(int8_t errnum, jeffe_lang_strerror_t strerror_func);
jeffe_lang_any jeffe_lang_object(jeffe_lang_classfn_t class_fn, size_t nargs, const jefe_lang_any *args)
```