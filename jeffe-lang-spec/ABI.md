## Overview & Memory Topography

Every value in the language can be stored by a `struct jeffe_value`, which transparently wraps a `uint64_t`.

The `uint64_t` is divided as follows:
- top 8 bits: `enum jeffe_typetag`
- next 8 bits: extended data
- bottom 48 bits: payload

The following rules determine how various values are stored:
- **Inline Primitives ($\le 48$ bits):** Values fitting within 48 bits are stored directly in the lower bits of the container. Unused bits in this field must be zeroed or sign-extended according to the underlying primitive's semantic type.
	- **Pointer Sign-Extension:** Modern virtual address pointers actually fit into 48 bits on x64/arm64. The pointer is sign extended, so the upper 16 bits are ignored. 
- **Heap-Allocated Primitives ($> 48$ bits):** Values exceeding 48 bits are allocated via the heap provider. The resulting 64-bit address is truncated to its lower 48 bits for storage.
## Type Tags
The `TypeTag` is defined as

```c
enum jeffe_typetag {
    JEFFE_TYPETAG_NIL      = 0x00,
    JEFFE_TYPETAG_CHAR     = 0x01,
    JEFFE_TYPETAG_BOOL     = 0x02,
    JEFFE_TYPETAG_I32      = 0x03,
    JEFFE_TYPETAG_U32      = 0x04,
    JEFFE_TYPETAG_I64      = 0x05,
    JEFFE_TYPETAG_U64      = 0x06,
    JEFFE_TYPETAG_F32      = 0x07,
    JEFFE_TYPETAG_F64      = 0x08,
    JEFFE_TYPETAG_PTR      = 0x09,
    JEFFE_TYPETAG_CSTRUCT  = 0x0A,
    JEFFE_TYPETAG_ERRNUM   = 0x0B,
    JEFFE_TYPETAG_OBJ      = 0x0C,
};
```

## Inline Values
Inline values have their payload stored directly in the last 48 bits of the `jeffe_value`.

These types include:
- Nil
- Char
- Bool
- I32
- U32
- F32 (type-punned into a u32)
- Ptr (after 48 bit compression)
## Heap Values
Heap values have their payloads stored on the heap, with the resulting pointer stored in the last 48 bits of `jeffe_value`. Performance is worse than for inline values because:
1. allocations are required
2. copies require a `memcpy` on top of just copying the `jeffe_value`'s 64 bits
3. pointer-chasing hurts cache performance.

However, they are necessary for many APIs (e.g. POSIX results that return `size_t`). Use heap values sparingly in these cases.

These types include:
- I64
- U64
- F64
## CStructs
Cstructs are designed to store heap data owned by the value. See [[Operators#Core]] for more details about copying and destruction.

Use CStruct when you want to have an "owning" block of data that has C semantics (e.g. copying by value).

## ErrNums
ErrNums wrap a POSIX-style error code + error domain. They are constructed with an `i8` representing the error code, and a `jeffe_strerror_fn` that returns a string representation of the error code. The returned string should have a global lifetime.

In this way, error codes can overlap without ambiguity, since each `jeffe_strerror_fn` is unique per error domain.

## Objects
`jeffe-lang` uses a single function scheme for declaring new classes. The signature is:

```c
typedef struct jeffe_value (*jeffe_class_fn)(
    void **userdata, 
    int op, 
    size_t nargs, 
    const struct jeffe_value *args
);
```

Negative values of `op` are reserved for user extensions. Positive values are part of the ABI specification as part of enum `jeffe_op`:
```c
enum jeffe_op {
    JEFFE_OP_CTOR,
    JEFFE_OP_DTOR
};
```

*For a full list of operators, see [[Operators]]*

To create a new object, the `jeffe-lang` implementation should heap-allocate an association (i.e. an `objmeta`) between a `jeffe_class_fn` and a `void *userdata`. A pointer to the `userdata` field is then passed into the `jeffe_class_fn` for manipulation.

> ⚠️ the `objmeta` field is implementation private. It may contain extra fields to enable memory management and other things (e.g. refcounts, GC trackers)

The constructor is then executed via 
```c
class_fn(&userdata, JEFFE_OP_CTOR, 0, NULL)
```