use crate::objmeta::ObjMeta;
use crate::sizeof_alloc::sizeof_alloc;
use crate::value::{uncompress_ptr, PAYLOAD_MASK};
use crate::{ClassFn, StrerrorFn, TypeTag, Value};
use std::ffi::c_void;

/// a tagged union alternative interface for jeffe_lang::Value
/// because it's rust, supports match clauses
/// will be tied to the underlying Value's lifetime
pub enum ValueUnion<'a> {
    Nil,
    Char(char),
    Bool(bool),
    Int32(i32),
    UInt32(u32),
    // Instead of copying data out or owning it,
    // these point directly to the data inside Value's allocations
    Int64(&'a i64),
    UInt64(&'a u64),
    Float32(f32),
    Float64(&'a f64),
    Pointer(*mut c_void),
    CStruct(&'a [u8]), // Representing a CStruct as a safe byte slice bound by lifetime 'a
    ErrNum(u8, StrerrorFn),
    Object(&'a ObjMeta, ClassFn), // Borrowing the actual ObjMeta struct directly
}
impl Value {
    pub fn as_union<'a>(&'a self) -> ValueUnion<'a> {
        use TypeTag::*;
        let lower_48 = self.0 & PAYLOAD_MASK;

        match self.type_tag() {
            Nil => ValueUnion::Nil,
            Char => ValueUnion::Char(unsafe { char::from_u32_unchecked(self.0 as u32) }),
            Bool => ValueUnion::Bool((self.0 & 1) != 0),
            I32 => ValueUnion::Int32(self.0 as i32),
            U32 => ValueUnion::UInt32(self.0 as u32),
            F32 => ValueUnion::Float32(f32::from_bits(self.0 as u32)),

            // Reconstruct the reference and bind it to the lifetime of &self
            F64 => unsafe { ValueUnion::Float64(&*(uncompress_ptr(lower_48) as *const f64)) },
            I64 => unsafe { ValueUnion::Int64(&*(uncompress_ptr(lower_48) as *const i64)) },
            U64 => unsafe { ValueUnion::UInt64(&*(uncompress_ptr(lower_48) as *const u64)) },

            Ptr => ValueUnion::Pointer(uncompress_ptr(lower_48)),

            CStruct => unsafe {
                let ptr = uncompress_ptr(lower_48);
                let alloc_sz = sizeof_alloc(ptr);
                // Turn raw malloc'd memory into a safe slice tied to 'a
                ValueUnion::CStruct(std::slice::from_raw_parts(ptr as *const u8, alloc_sz))
            },

            ErrNum => unsafe {
                let err = ((self.0 >> 48) & 0xFF) as u8;
                let fn_ptr = std::mem::transmute(uncompress_ptr(lower_48));
                ValueUnion::ErrNum(err, fn_ptr)
            },

            Obj => unsafe {
                let meta_ptr = uncompress_ptr(lower_48) as *const ObjMeta;
                let class_fn = (*meta_ptr).class_fn;
                // Safely hand out a reference to the ObjMeta bound by 'a
                ValueUnion::Object(&*meta_ptr, class_fn)
            },
        }
    }
}
