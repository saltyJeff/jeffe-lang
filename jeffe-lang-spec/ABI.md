## Overview & Memory Topography

Every value in the language can be stored by a `struct jeffe_value`, which transparently wraps a `uint64_t`.

The `uint64_t` is divided as follows:
- top 8 bits: `enum jeffe_typetag`
- next 8 bits: extended data
- bottom 48 bits: payload

The following rules determine how various values are stored:
- **Inline Primitives ($\le 48$ bits):** Values fitting within 48 bits are stored directly in the lower bits of the container. Unused bits in this field must be zeroed or sign-extended according to the underlying primitive's semantic type.
	- **Pointer Sign-Extension:** Virtual addresses in modern 64-bit architectures utilize a 48-bit address space, canonicalized by propagating bit 47 across the remaining upper 16 bits. When extracting pointers from a `struct jeffe_value`, implementations must manually sign-extend the 48-bit payload back to a full 64-bit address space before dereferencing
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

## C ABI Implementation

```c
#include <stdint.h>

// helper
static inline uint64_t canonicalize_ptr(uint64_t payload) { 
	return (uint64_t)(((int64_t)(payload << 16)) >> 16);
}

struct jeffe_value {
    uint64_t v;
};

static inline uint8_t jeffe_value_type(struct jeffe_value val) {
    return (uint8_t)(val.v >> 56);
}

static inline uint64_t jeffe_value_payload(struct jeffe_value val) {
    return val.v & 0x0000FFFFFFFFFFFFULL;
}
```

### Type Conversion Mechanics & Primitive Definitions

Data exchange between native types and `Value` relies on standard `From` and `TryFrom` trait implementations.
### Inline Primitive Example: `I32`

The following snippet demonstrates the layout for types matching or falling below the 48-bit immediate threshold. This exact boilerplates repeats for `U32`, `Bool`, `Char`, and `F32` types using their corresponding tags. For `Nil`, `new()` takes no arguments, and `val()` returns `void`.

```c
struct jeffe_value jeffe_value_i32(int32_t val) {
    uint64_t tag = ((uint64_t)JEFFE_TYPETAG_I32) << 56;
    uint64_t payload = ((uint64_t)(uint32_t)val) & 0x0000FFFFFFFFFFFFULL;
    struct jeffe_value v;
    v.v = tag | payload;
    return v;
}

int32_t jeffe_i32_val(struct jeffe_value val) {
    return (int32_t)jeffe_value_payload(val);
}
```

### Heap-Allocated Primitive Example: `I64`

Types exceeding 48 bits must manage a heap allocation. This implementation structure must be repeated for `U64` and `F64`.

```c
#include <stdlib.h>

struct jeffe_value jeffe_value_i64(int64_t val) {
    int64_t *boxed = (int64_t *)malloc(sizeof(int64_t));
    *boxed = val;
    uint64_t tag = ((uint64_t)JEFFE_TYPETAG_I64) << 56;
    uint64_t payload = ((uint64_t)boxed) & 0x0000FFFFFFFFFFFFULL;
    struct jeffe_value v;
    v.v = tag | payload;
    return v;
}

int64_t jeffe_i64_val(struct jeffe_value val) {
    uint64_t ptr_bits = canonicalize_ptr(jeffe_value_payload(val));
    return *(int64_t *)ptr_bits;
}

void jeffe_i64_drop(struct jeffe_value val) {
    uint64_t ptr_bits = canonicalize_ptr(jeffe_value_payload(val));
    free((void *)ptr_bits);
}
```

### Native Memory Management: `CStruct`

`CStruct` encapsulates unstructured heap blocks managed via standard system allocators (`malloc`/`free`). Unlike basic raw pointers, `CStruct` retains ownership of its target allocation and frees it automatically when the instance drops out of scope.

```c
struct jeffe_value jeffe_value_cstruct(size_t struct_sz) {
    void *raw_ptr = malloc(struct_sz);
    if (!raw_ptr) {
        // Handle allocation failure
    }
    
    uint64_t tag = ((uint64_t)JEFFE_TYPETAG_CSTRUCT) << 56;
    uint64_t payload = ((uint64_t)raw_ptr) & 0x0000FFFFFFFFFFFFULL;
    struct jeffe_value v;
    v.v = tag | payload;
    return v;
}

void *jeffe_cstruct_val(struct jeffe_value val) {
    return (void *)canonicalize_ptr(jeffe_value_payload(val));
}

void jeffe_cstruct_drop(struct jeffe_value val) {
    free(jeffe_cstruct_val(val));
}
```

### Error Subsystem: `ErrNum`

The `ErrNum` construct maps POSIX-style error codes (`errno`) to an evaluation function pointer. The error code is stored in the extended data region, and the strerror function is stored in the payload. The strerror's returned string should have global lifetime as a string literal.

```c
typedef const char *(*jeffe_strerror_fn)(int8_t err);

struct jeffe_value jeffe_value_errnum(int8_t errnum, jeffe_strerror_fn strerror_fn) {
    uint64_t tag = ((uint64_t)JEFFE_TYPETAG_ERRNUM) << 56;
    uint64_t err_byte = ((uint64_t)(uint8_t)errnum) << 48;
    uint64_t fn_payload = ((uint64_t)strerror_fn) & 0x0000FFFFFFFFFFFFULL;
    
    struct jeffe_value v;
    v.v = tag | err_byte | fn_payload;
    return v;
}

int8_t jeffe_errnum_errnum(struct jeffe_value val) {
    return (int8_t)((val.v >> 48) & 0xFF);
}

jeffe_strerror_fn jeffe_errnum_strerror_fn(struct jeffe_value val) {
    uint64_t fn_bits = canonicalize_ptr(val.v & 0x0000FFFFFFFFFFFFULL);
    return (jeffe_strerror_fn)fn_bits;
}

const char *jeffe_errnum_value(struct jeffe_value val) {
    jeffe_strerror_fn func = jeffe_errnum_strerror_fn(val);
    return func(jeffe_errnum_errnum(val));
}
```

### Objects

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
###### Implementation Details
> ⚠️this section is not part of the ABI contract

To better explain how Objects are created, this document refers to the following implementation-dependent struct.

```c
#include <stdatomic.h>

struct jeffe_obj_meta {
    jeffe_class_fn class_fn;
    void *userdata;
    uint64_t thread_id; // id of owning thread
    uint32_t strong_count; // strong count in the owning thread
    atomic_uint_least16_t weak_count; // weak count across all threads
    atomic_uint_least16_t atomic_strong; // strong count in other threads
};
```

### Object Implementation

```c
struct jeffe_value jeffe_value_obj(jeffe_class_fn class_fn, size_t nargs, const struct jeffe_value *args) {
    // 1. Allocate control structure
    struct jeffe_obj_meta *meta_ptr = malloc(sizeof(struct jeffe_obj_meta));
    meta_ptr->class_fn = class_fn;
    meta_ptr->userdata = NULL;
    meta_ptr->thread_id = 0;
    meta_ptr->strong_count = 1;
    atomic_init(&meta_ptr->weak_count, 0);
    atomic_init(&meta_ptr->atomic_strong, 0);

    // 2. Invoke constructor operation
    struct jeffe_value ctor_res = class_fn(
        &meta_ptr->userdata,
        JEFFE_OP_CTOR,
        nargs,
        args
    );

    // 3. Inspect instantiation success via error opcode check (Op::IsErr mapped to integer, e.g. some positive OP)
    // NOTE: assuming JEFFE_OP_IS_ERR exists.
    struct jeffe_value is_err_res = class_fn(
        &meta_ptr->userdata,
        /* JEFFE_OP_IS_ERR */ 2, // example opcode
        0,
        NULL
    );

    // Abort and unwind allocation if an error flag is confirmed
    // (psuedocode: if is_err_res implies error)
    if (jeffe_value_type(is_err_res) == JEFFE_TYPETAG_BOOL && /* is_true */ 0) {
        free(meta_ptr);
        return ctor_res;
    }

    // 4. Pack control block address into an Object-tagged Value container
    uint64_t tag = ((uint64_t)JEFFE_TYPETAG_OBJ) << 56;
    uint64_t payload = ((uint64_t)meta_ptr) & 0x0000FFFFFFFFFFFFULL;
    struct jeffe_value v;
    v.v = tag | payload;
    return v;
}

void jeffe_obj_drop(struct jeffe_value val) {
    struct jeffe_obj_meta *meta_ptr = (struct jeffe_obj_meta *)canonicalize_ptr(jeffe_value_payload(val));
    
    // Invoke the class destructor via dispatch handler
    meta_ptr->class_fn(
        &meta_ptr->userdata,
        JEFFE_OP_DTOR,
        0,
        NULL
    );

    // Deallocate control block memory
    free(meta_ptr);
}
```