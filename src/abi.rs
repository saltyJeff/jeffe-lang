use std::ffi::{c_char, c_void};

/// Public ABI interface types here

#[repr(transparent)]
pub struct Value(pub u64);

#[repr(u8)]
pub enum TypeTag
{
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

pub type StrerrorFn = extern "C" fn(err: i8) -> *const c_char;
pub type ClassFn = unsafe extern "C" fn(
    userdata: *mut *mut c_void,
    op: i32,
    nargs: usize,
    args: *const Value
) -> Value;
