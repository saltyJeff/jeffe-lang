use crate::objmeta::ObjMeta;
use crate::sizeof_alloc::sizeof_alloc;
use crate::Op::{self, Ctor};
use crate::{ClassFn, StrerrorFn, TypeTag, Value};
use libc::{free, malloc};
use std::ffi::c_void;

pub(crate) const PAYLOAD_MASK: u64 = 0x0000_FFFF_FFFF_FFFF_u64;
pub(crate) fn compress_ptr(ptr: *mut c_void) -> u64 {
    (ptr as u64) & PAYLOAD_MASK
}
#[inline(always)]
pub(crate) fn uncompress_ptr(compressed_ptr: u64) -> *mut c_void {
    (((compressed_ptr << 16) as i64) >> 16) as *mut c_void
}

impl Value {
    pub fn new_nil() -> Self {
        Self((TypeTag::Nil as u64) << 56)
    }
    pub fn new_char(c: char) -> Self {
        Self((TypeTag::Char as u64) << 56 | c as u64)
    }
    pub fn new_bool(b: bool) -> Self {
        Self((TypeTag::Bool as u64) << 56 | b as u64)
    }
    pub fn new_i32(i: i32) -> Self {
        Self((TypeTag::I32 as u64) << 56 | ((i as u64) & 0xFFFF_FFFF_u64))
    }
    pub fn new_u32(u: u32) -> Self {
        Self((TypeTag::U32 as u64) << 56 | (u as u64))
    }
    pub fn new_f32(f: f32) -> Self {
        Self((TypeTag::F32 as u64) << 56 | (f.to_bits() as u64))
    }
    pub fn new_f64(f: f64) -> Self {
        Self((TypeTag::F64 as u64) << 56 | compress_ptr(Box::into_raw(Box::new(f)) as *mut c_void))
    }
    pub fn new_i64(il: i64) -> Self {
        Self(
            ((TypeTag::I64 as u64) << 56)
                | compress_ptr(Box::into_raw(Box::new(il)) as *mut c_void),
        )
    }
    pub fn new_u64(ul: u64) -> Self {
        Self(
            ((TypeTag::U64 as u64) << 56)
                | compress_ptr(Box::into_raw(Box::new(ul)) as *mut c_void),
        )
    }
    pub fn new_ptr(ptr: *mut c_void) -> Self {
        Self(((TypeTag::Ptr as u64) << 56) | compress_ptr(ptr))
    }
    pub fn new_cstruct(sz: usize) -> Self {
        unsafe { Self((TypeTag::CStruct as u64) << 56 | compress_ptr(malloc(sz))) }
    }
    pub fn new_errnum(err: i8, strerror_fn: StrerrorFn) -> Self {
        Self(
            ((TypeTag::ErrNum as u64) << 56)
                | ((err as u64) << 48)
                | compress_ptr(strerror_fn as usize as *mut c_void),
        )
    }
    pub fn new_obj(class_fn: ClassFn, args: &[Self]) -> Self {
        let mut meta = Box::new(ObjMeta::new(class_fn));
        let ctor_ret = meta.invoke_op(Ctor, args);
        // TODO: check for error
        let meta_ptr = Box::into_raw(meta) as *mut c_void;
        Self(((TypeTag::Obj as u64) << 56) | compress_ptr(meta_ptr))
    }
    pub fn type_tag(&self) -> TypeTag {
        let tag: TypeTag = unsafe { std::mem::transmute((self.0 >> 56) as u8) };
        tag
    }
}
impl Drop for Value {
    fn drop(&mut self) {
        use TypeTag::*;
        let lower_48 = self.0 & PAYLOAD_MASK;

        match self.type_tag() {
            F64 | I64 | U64 => unsafe {
                Box::from_raw(uncompress_ptr(lower_48));
            },
            CStruct => unsafe { free(uncompress_ptr(lower_48)) },
            Obj => unsafe {
                let meta_ptr = uncompress_ptr(lower_48) as *mut ObjMeta;
                (*meta_ptr).invoke_op(Op::Dtor, &[]);
                Box::from_raw(meta_ptr);
            },
            _ => {}
        }
    }
}
impl Clone for Value {
    fn clone(&self) -> Self {
        use TypeTag::*;
        let lower_48 = self.0 & PAYLOAD_MASK;

        match self.type_tag() {
            F64 => unsafe { Self::new_f64(*(uncompress_ptr(lower_48) as *mut f64)) },
            I64 => unsafe { Self::new_i64(*(uncompress_ptr(lower_48) as *mut i64)) },
            U64 => unsafe { Self::new_u64(*(uncompress_ptr(lower_48) as *mut u64)) },
            CStruct => unsafe {
                let src_ptr = uncompress_ptr(lower_48);
                let alloc_sz = sizeof_alloc(src_ptr);
                let ret = Self::new_cstruct(alloc_sz);
                let ret_ptr = uncompress_ptr(ret.0 & PAYLOAD_MASK);
                std::ptr::copy_nonoverlapping(src_ptr, ret_ptr, alloc_sz);
                ret
            },
            Obj => unsafe {
                let meta_ptr = uncompress_ptr(lower_48) as *mut ObjMeta;
                (*meta_ptr).invoke_op(Op::Copy, &[])
            },
            _ => Self(self.0),
        }
    }
}
