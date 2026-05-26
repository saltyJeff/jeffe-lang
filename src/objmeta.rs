use crate::{ClassFn, Op, Value};
use static_assertions::assert_eq_size;
use std::ffi::c_void;
use std::ptr::null_mut;
use std::sync::atomic::AtomicU16;
use std::thread;
use std::thread::ThreadId;

pub(crate) struct ObjMeta {
    pub(crate) class_fn: ClassFn,
    pub(crate) userdata: *mut c_void,
    pub(crate) thread_id: ThreadId,
    pub(crate) strong_count: u32,
    pub(crate) weak_count: std::sync::atomic::AtomicU16,
    pub(crate) atomic_strong_count: std::sync::atomic::AtomicU16,
}
impl ObjMeta {
    pub fn new(class_fn: ClassFn) -> Self {
        Self {
            class_fn,
            userdata: null_mut(),
            thread_id: thread::current().id(),
            strong_count: 1,
            atomic_strong_count: AtomicU16::new(0),
            weak_count: AtomicU16::new(0),
        }
    }
    pub fn invoke(&mut self, op: i32, args: &[Value]) -> Value {
        unsafe { (self.class_fn)(&mut self.userdata, op, args.len(), args.as_ptr()) }
    }
    pub fn invoke_op(&mut self, op: Op, args: &[Value]) -> Value {
        unsafe { (self.class_fn)(&mut self.userdata, op as i32, args.len(), args.as_ptr()) }
    }
}

assert_eq_size!(ObjMeta, [u8; 32]);
