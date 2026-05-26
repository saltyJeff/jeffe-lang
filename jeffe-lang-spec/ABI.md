## Overview & Memory Topography

Every value in the language can be stored by an `jeffe_lang::Value`, which transparently wraps a `u64`.

The `u64` is divided as follows:
- top 8 bits: `TypeTag` enum
- next 8 bits: extended data
- bottom 48 bits: payload

The following rules determine how various values are stored:
- **Inline Primitives ($\le 48$ bits):** Values fitting within 48 bits are stored directly in the lower bits of the container. Unused bits in this field must be zeroed or sign-extended according to the underlying primitive's semantic type.
	- **Pointer Sign-Extension:** Virtual addresses in modern 64-bit architectures utilize a 48-bit address space, canonicalized by propagating bit 47 across the remaining upper 16 bits. When extracting pointers from an `Value`, implementations must manually sign-extend the 48-bit payload back to a full 64-bit address space before dereferencing
- **Heap-Allocated Primitives ($> 48$ bits):** Values exceeding 48 bits are allocated via the heap provider. The resulting 64-bit address is truncated to its lower 48 bits for storage.
## Type Tags
The `TypeTag` is defined as

```rust
#[repr(u8)]
pub enum TypeTag {
    Nil      = 0x00,
    Char     = 0x01,
    Bool     = 0x02,
    I32      = 0x03,
    U32      = 0x04,
    I64      = 0x05,
    U64      = 0x06,
    F32      = 0x07,
    F64      = 0x08,
    Ptr      = 0x09,
    CStruct  = 0x0A,
    ErrNum   = 0x0B,
    Obj      = 0x0C,
}
```

## Rust ABI Implementation

```rust
// helper
#[inline(always)]
pub fn canonicalize_ptr(payload: u64) -> u64
{ 
	(((payload << 16) as i64) >> 16) as u64 
}

#[derive(Copy, Clone, Debug, PartialEq, Eq)]
#[repr(transparent)]
pub struct Value(pub u64);

impl Value {
    #[inline(always)]
    pub fn get_tag(&self) -> u8 {
        (self.0 >> 56) as u8
    }

    #[inline(always)]
    pub fn get_payload(&self) -> u64 {
        self.0 & 0x0000_FFFF_FFFF_FFFF
    }
}
```

### Type Conversion Mechanics & Primitive Definitions

Data exchange between native types and `Value` relies on standard `From` and `TryFrom` trait implementations.
### Inline Primitive Example: `I32`

The following snippet demonstrates the layout for types matching or falling below the 48-bit immediate threshold. This exact boilerplates repeats for `U32`, `Bool`, `Char`, and `F32` types using their corresponding tags. For `Nil`, `new()` takes no arguments, and `val()` returns `()`.

```rust
pub struct I32(pub Value);

impl From<I32> for Value {
    #[inline]
    fn from(wrapper: I32) -> Self {
        wrapper.0
    }
}

impl TryFrom<Value> for I32 {
    type Error = &'static str;
    #[inline]
    fn try_from(value: Value) -> Result<Self, Self::Error> {
        if value.get_tag() == TypeTag::I32 as u8 {
            Ok(I32(value))
        } else {
            Err("ABI Type Mismatch")
        }
    }
}

impl I32 {
    pub fn new(val: i32) -> Self {
        let tag = (TypeTag::I32 as u64) << 56;
        let payload = (val as u64) & 0x0000_FFFF_FFFF_FFFF;
        I32(Value(tag | payload))
    }

    pub fn val(&self) -> i32 {
        self.0.get_payload() as i32
    }
}
```

### Heap-Allocated Primitive Example: `I64`

Types exceeding 48 bits must manage a heap allocation. This implementation structure must be repeated for `U64` and `F64`.


```rust
pub struct I64(pub Value);

impl From<I64> for Value {
    #[inline]
    fn from(wrapper: I64) -> Self {
        wrapper.0
    }
}

impl TryFrom<Value> for I64 {
    type Error = &'static str;

    #[inline]
    fn try_from(value: Value) -> Result<Self, Self::Error> {
        if value.get_tag() == TypeTag::I64 as u8 {
            Ok(I64(value))
        } else {
            Err("ABI Type Mismatch")
        }
    }
}

impl I64 {
    pub fn new(val: i64) -> Self {
        let boxed = Box::into_raw(Box::new(val)) as u64;
        let tag = (TypeTag::I64 as u64) << 56;
        let payload = boxed & 0x0000_FFFF_FFFF_FFFF;
        I64(Value(tag | payload))
    }

    pub fn val(&self) -> i64 {
        let ptr_bits = canonicalize_ptr(self.0.get_payload());
        unsafe { *(ptr_bits as *const i64) }
    }
}

impl Drop for I64 {
    fn drop(&mut self) {
        let ptr_bits = canonicalize_ptr(self.0.get_payload());
        unsafe {
            let _ = Box::from_raw(ptr_bits as *mut i64);
        }
    }
}
```

### Native Memory Management: `CStruct`

`CStruct` encapsulates unstructured heap blocks managed via standard system allocators (`malloc`/`free`). Unlike basic raw pointers, `CStruct` retains ownership of its target allocation and frees it automatically when the instance drops out of scope.

```rust
pub struct CStruct(pub Value);

impl From<CStruct> for Value {
    #[inline]
    fn from(wrapper: CStruct) -> Self {
        wrapper.0
    }
}

impl TryFrom<Value> for CStruct {
    type Error = &'static str;

    #[inline]
    fn try_from(value: Value) -> Result<Self, Self::Error> {
        if value.get_tag() == TypeTag::CStruct as u8 {
            Ok(CStruct(value))
        } else {
            Err("ABI Type Mismatch")
        }
    }
}

impl CStruct {
    pub fn new(struct_sz: usize) -> Self {
        unsafe {
            let raw_ptr = libc::malloc(struct_sz);
            assert!(!raw_ptr.is_null(), "Allocation failure");
            
            let tag = (TypeTag::CStruct as u64) << 56;
            let payload = (raw_ptr as u64) & 0x0000_FFFF_FFFF_FFFF;
            CStruct(Value(tag | payload))
        }
    }

    pub fn val(&self) -> *mut libc::c_void {
        canonicalize_ptr(self.0.get_payload()) as *mut libc::c_void
    }
}

impl Drop for CStruct {
    fn drop(&mut self) {
        unsafe {
            libc::free(self.val());
        }
    }
}
```

### Error Subsystem: `ErrNum`

The `ErrNum` construct maps POSIX-style error codes (`errno`) to an evaluation function pointer. The error code is stored in the extended data region, and the strerror function is stored in the payload. The strerror's returned string should have global lifetime as a string literal.

```rust
pub type StrerrorFn = extern "C" fn(err: i8) -> *const libc::c_char;

pub struct ErrNum(pub Value);

impl From<ErrNum> for Value {
    #[inline]
    fn from(wrapper: ErrNum) -> Self {
        wrapper.0
    }
}

impl TryFrom<Value> for ErrNum {
    type Error = &'static str;

    #[inline]
    fn try_from(value: Value) -> Result<Self, Self::Error> {
        if value.get_tag() == TypeTag::ErrNum as u8 {
            Ok(ErrNum(value))
        } else {
            Err("ABI Type Mismatch")
        }
    }
}

impl ErrNum {
    pub fn new(errnum: i8, strerror_fn: StrerrorFn) -> Self {
        let tag = (TypeTag::ErrNum as u64) << 56;
        let err_byte = ((errnum as u8) as u64) << 48;
        let fn_payload = (strerror_fn as u64) & 0x0000_FFFF_FFFF_FFFF;
        
        ErrNum(Value(tag | err_byte | fn_payload))
    }

    pub fn errnum(&self) -> i8 {
        ((self.0.0 >> 48) & 0xFF) as i8
    }

    pub fn strerror_fn(&self) -> StrerrorFn {
        let fn_bits = canonicalize_ptr(self.0.0 & 0x0000_FFFF_FFFF_FFFF);
        unsafe { std::mem::transmute(fn_bits) }
    }

    pub fn value(&self) -> *const libc::c_char {
        let func = self.strerror_fn();
        func(self.errnum())
    }
}
```

### Objects

`jeffe-lang` uses a single function scheme for declaring new classes. The signature is:
```rust
pub type ClassFn = unsafe extern "C" fn(
    userdata: *mut *mut libc::c_void, 
    op: i32, 
    nargs: usize, 
    args: *const Value
) -> Value;
```

Negative values of `op` are reserved for user extensions. Positive values are part of the ABI specification as part of enum `jeffe_lang::Op`:
```rust
#[repr(i32)]
pub enum Op
{
	Ctor,
	Dtor
}
```

*For a full list of operators, see [[Operators]]*
#### Implementation Details
> ⚠️this section is not part of the ABI contract

To better explain how Objects are created, this document refers to the following implementation-dependent struct.

```rust
#[repr(C)]
struct ObjMeta {
    class_fn: ClassFn,
    userdata: *mut libc::c_void,
    thread_id: u64, // id of owning thread
    strong_count: u32, // strong count in the owning thread
    weak_count: std::sync::atomic::AtomicU16, // weak count across all threads
    atomic_strong: std::sync::atomic::AtomicU16, // strong count in other threads
}
```

### Object Implementation

```rust
pub struct Object(pub Value);

impl From<Object> for Value {
    #[inline]
    fn from(wrapper: Object) -> Self {
        wrapper.0
    }
}

impl TryFrom<Value> for Object {
    type Error = &'static str;

    #[inline]
    fn try_from(value: Value) -> Result<Self, Self::Error> {
        if value.get_tag() == TypeTag::Obj as u8 {
            Ok(Object(value))
        } else {
            Err("ABI Type Mismatch")
        }
    }
}

impl Object {
    pub fn new(class_fn: ClassFn, args: &mut [Value]) -> Result<Self, Value> {
        // 1. Allocate control structure
        let meta_ptr = Box::into_raw(Box::new(ObjMeta {
            class_fn,
            userdata: std::ptr::null_mut(),
            thread_id: 0, 
            strong_count: 1,
            weak_count: std::sync::atomic::AtomicU16::new(0),
            atomic_strong: std::sync::atomic::AtomicU16::new(0),
        }));

        unsafe {
            // 2. Invoke constructor operation
            let ctor_res = class_fn(
                &mut (*meta_ptr).userdata,
                Op::Ctor as i32,
                args.len(),
                args.as_ptr(),
            );

            // 3. Inspect instantiation success via error opcode check
            let is_err_res = class_fn(
                &mut (*meta_ptr).userdata,
                Op::IsErr as i32,
                0,
                std::ptr::null_mut(),
            );

            // Abort and unwind allocation if an error flag is confirmed
            if is_err_res.is_error()
                let _ = Box::from_raw(meta_ptr);
                return Err(ctor_res);
            }

            // 4. Pack control block address into an Object-tagged Value container
            let tag = (TypeTag::Obj as u64) << 56;
            let payload = (meta_ptr as u64) & 0x0000_FFFF_FFFF_FFFF;
            Ok(Object(Value(tag | payload)))
        }
    }

    #[inline]
    pub fn meta(&self) -> *mut libc::c_void {
        canonicalize_ptr(self.0.get_payload()) as *mut libc::c_void
    }

    #[inline]
    pub fn classfn(&self) -> ClassFn {
        unsafe { (*(self.meta() as *mut ObjMeta)).class_fn }
    }

    #[inline]
    pub fn userdata(&self) -> *mut libc::c_void {
        unsafe { (*(self.meta() as *mut ObjMeta)).userdata }
    }
}

impl Drop for Object {
    fn drop(&mut self) {
        unsafe {
            let meta_ptr = self.meta() as *mut ObjMeta;
            
            // Invoke the class destructor via dispatch handler
            ((*meta_ptr).class_fn)(
                &mut (*meta_ptr).userdata,
                Op::Dtor as i32,
                0,
                std::ptr::null_mut(),
            );

            // Deallocate control block memory
            let _ = Box::from_raw(meta_ptr);
        }
    }
}