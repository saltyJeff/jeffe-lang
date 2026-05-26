use jeffe_lang::*;
use std::sync::atomic::{AtomicU32, Ordering};

// Global counter for tracking dynamic mock allocations in objects
static ALLOC_COUNT: AtomicU32 = AtomicU32::new(0);

// ==========================================
// 1. IMMEDIATE PRIMITIVE TESTS
// ==========================================

#[test]
fn test_nil() {
    let nil_wrapper = Nil::new();
    let val: AnyValue = nil_wrapper.into();
    
    assert_eq!(val.get_tag(), TypeTag::Nil as u8);
    assert_eq!(val.get_payload(), 0);
    assert!(!val.is_error());

    let try_nil = Nil::try_from(val).unwrap();
    try_nil.val(); // Returns ()
}

#[test]
fn test_bool() {
    let true_val: AnyValue = Bool::new(true).into();
    let false_val: AnyValue = Bool::new(false).into();

    assert_eq!(true_val.get_tag(), TypeTag::Bool as u8);
    assert_eq!(true_val.get_payload(), 1);
    assert_eq!(false_val.get_tag(), TypeTag::Bool as u8);
    assert_eq!(false_val.get_payload(), 0);
    
    assert!(!true_val.is_error());
    assert!(!false_val.is_error());

    assert!(Bool::try_from(true_val).unwrap().val());
    assert!(!Bool::try_from(false_val).unwrap().val());
}

#[test]
fn test_char() {
    let c_val: AnyValue = Char::new('z').into();
    assert_eq!(c_val.get_tag(), TypeTag::Char as u8);
    assert_eq!(c_val.get_payload(), 'z' as u32 as u64);
    assert_eq!(Char::try_from(c_val).unwrap().val(), 'z');

    let emoji_val: AnyValue = Char::new('🦀').into();
    assert_eq!(Char::try_from(emoji_val).unwrap().val(), '🦀');
}

#[test]
fn test_i32_u32_f32() {
    // I32
    let i_val: AnyValue = I32::new(-12345).into();
    assert_eq!(i_val.get_tag(), TypeTag::I32 as u8);
    assert_eq!(I32::try_from(i_val).unwrap().val(), -12345);

    // U32
    let u_val: AnyValue = U32::new(987654).into();
    assert_eq!(u_val.get_tag(), TypeTag::U32 as u8);
    assert_eq!(U32::try_from(u_val).unwrap().val(), 987654);

    // F32
    let f_val: AnyValue = F32::new(2.718).into();
    assert_eq!(f_val.get_tag(), TypeTag::F32 as u8);
    let diff = F32::try_from(f_val).unwrap().val() - 2.718;
    assert!(diff.abs() < 1e-6);
}

#[test]
fn test_ptr() {
    let mut data: i32 = 1337;
    let raw_ptr = &mut data as *mut i32 as *mut libc::c_void;
    let ptr_val: AnyValue = Ptr::new(raw_ptr).into();

    assert_eq!(ptr_val.get_tag(), TypeTag::Ptr as u8);
    let extracted = Ptr::try_from(ptr_val).unwrap().val() as *mut i32;
    unsafe {
        assert_eq!(*extracted, 1337);
        *extracted = 9000;
        assert_eq!(data, 9000);
    }
}

// ==========================================
// 2. HEAP-ALLOCATED PRIMITIVES TESTS
// ==========================================

#[test]
fn test_heap_primitives_lifecycle() {
    // I64
    {
        let val1 = I64::new(-9876543210i64);
        let any_val: AnyValue = val1.into();
        assert_eq!(any_val.get_tag(), TypeTag::I64 as u8);
        assert_eq!(I64::try_from(any_val).unwrap().val(), -9876543210i64);
    } // drops here safely

    // U64
    {
        let val2 = U64::new(123456789012345u64);
        let any_val: AnyValue = val2.into();
        assert_eq!(any_val.get_tag(), TypeTag::U64 as u8);
        assert_eq!(U64::try_from(any_val).unwrap().val(), 123456789012345u64);
    } // drops here safely

    // F64
    {
        let val3 = F64::new(3.141592653589793);
        let any_val: AnyValue = val3.into();
        assert_eq!(any_val.get_tag(), TypeTag::F64 as u8);
        let diff = F64::try_from(any_val).unwrap().val() - 3.141592653589793;
        assert!(diff.abs() < 1e-15);
    } // drops here safely
}

#[test]
fn test_heap_primitive_deep_copy() {
    // Create a heap-allocated I64 value
    let original_i64 = I64::new(42424242);
    let original_any: AnyValue = original_i64.into();

    // Perform deep-copy clone() which allocates a brand new Box
    let cloned_any = original_any.clone();

    assert_eq!(cloned_any.get_tag(), TypeTag::I64 as u8);
    // Assert payloads (pointers) are DIFFERENT, meaning separate heap boxes
    assert_ne!(original_any.get_payload(), cloned_any.get_payload());

    // Extract wrappers
    let original_wrapper = I64::try_from(original_any).unwrap();
    let cloned_wrapper = I64::try_from(cloned_any).unwrap();

    // Check values are equal
    assert_eq!(original_wrapper.val(), 42424242);
    assert_eq!(cloned_wrapper.val(), 42424242);

    // Both wrappers will drop safely here without causing a double-free!
}

// ==========================================
// 3. SYSTEM MEMORY ALLOCATOR (CStruct)
// ==========================================

#[test]
fn test_cstruct_lifecycle() {
    {
        let struct_size = 32;
        let cstruct = CStruct::new(struct_size);
        let any_val: AnyValue = cstruct.into();

        assert_eq!(any_val.get_tag(), TypeTag::CStruct as u8);
        let raw_ptr = CStruct::try_from(any_val).unwrap().val() as *mut u8;
        assert!(!raw_ptr.is_null());

        unsafe {
            for i in 0..struct_size {
                *raw_ptr.add(i) = i as u8;
            }
            for i in 0..struct_size {
                assert_eq!(*raw_ptr.add(i), i as u8);
            }
        }
    } // drops here and calls libc::free safely
}

// ==========================================
// 4. POSIX ERROR CODES (ErrNum)
// ==========================================

unsafe extern "C" fn mock_strerror(err: i8) -> *const libc::c_char {
    match err {
        7 => b"Permission denied\0".as_ptr() as *const libc::c_char,
        38 => b"Functionality not implemented\0".as_ptr() as *const libc::c_char,
        _ => b"Generic error\0".as_ptr() as *const libc::c_char,
    }
}

#[test]
fn test_errnum_lifecycle() {
    let err_val: AnyValue = ErrNum::new(7, mock_strerror).into();

    assert_eq!(err_val.get_tag(), TypeTag::ErrNum as u8);
    
    // Check errnum is placed in bits 48-55 (extended data)
    let wrapper = ErrNum::try_from(err_val.clone()).unwrap();
    assert_eq!(wrapper.errnum(), 7);

    // Call value() to execute mock_strerror and get string
    let c_str_ptr = wrapper.value();
    let c_str = unsafe { std::ffi::CStr::from_ptr(c_str_ptr) };
    assert_eq!(c_str.to_str().unwrap(), "Permission denied");

    // AnyValue representing an ErrNum is an error
    assert!(err_val.is_error());
}

// ==========================================
// 5. DYNAMIC OBJECTS & OPERATOR TESTS
// ==========================================

// Structured userdata for mock object
struct MockUserdata {
    value: i32,
    should_fail_is_err: bool,
    alloc_id: u32,
}

// Mock dynamic object class function
unsafe extern "C" fn mock_class_func(
    userdata: *mut *mut libc::c_void,
    op: i32,
    nargs: usize,
    args: *mut AnyValue,
) -> AnyValue {
    let op_enum = std::mem::transmute::<i32, Op>(op);

    match op_enum {
        Op::Ctor => {
            // Read value from args if present, default to 42
            let initial_value = if nargs > 0 {
                let any_arg = &*args;
                if any_arg.get_tag() == TypeTag::I32 as u8 {
                    I32::try_from(any_arg.clone()).unwrap().val()
                } else {
                    42
                }
            } else {
                42
            };

            // Allocate userdata on heap
            let alloc_id = ALLOC_COUNT.fetch_add(1, Ordering::SeqCst) + 1;
            let ud = Box::into_raw(Box::new(MockUserdata {
                value: initial_value,
                should_fail_is_err: false,
                alloc_id,
            })) as *mut libc::c_void;
            *userdata = ud;
            
            Nil::new().into()
        }
        Op::Dtor => {
            if !userdata.is_null() && !(*userdata).is_null() {
                let _ = Box::from_raw(*userdata as *mut MockUserdata);
                *userdata = std::ptr::null_mut();
            }
            Nil::new().into()
        }
        Op::IsErr => {
            let ud = *userdata as *mut MockUserdata;
            if !ud.is_null() && (*ud).should_fail_is_err {
                Bool::new(true).into()
            } else {
                Bool::new(false).into()
            }
        }
        Op::Copy => {
            let src_ud = *userdata as *const MockUserdata;
            let dest_alloc_id = ALLOC_COUNT.fetch_add(1, Ordering::SeqCst) + 1;
            let dest_ud = Box::into_raw(Box::new(MockUserdata {
                value: (*src_ud).value,
                should_fail_is_err: (*src_ud).should_fail_is_err,
                alloc_id: dest_alloc_id,
            }));

            // Create meta structure for new object clone
            let new_meta = Box::into_raw(Box::new(ObjMeta {
                class_func: mock_class_func,
                userdata: dest_ud as *mut libc::c_void,
                thread_id: 0,
                strong_count: 1,
                weak_count: std::sync::atomic::AtomicU16::new(0),
                atomic_strong: std::sync::atomic::AtomicU16::new(0),
            }));

            let tag = (TypeTag::Obj as u64) << 56;
            let payload = (new_meta as u64) & 0x0000_FFFF_FFFF_FFFF;
            AnyValue(tag | payload)
        }
        Op::Add => {
            let self_ud = *userdata as *const MockUserdata;
            let other_any = &*args;
            
            let other_value = if other_any.get_tag() == TypeTag::Obj as u8 {
                let other_meta = canonicalize_ptr(other_any.get_payload()) as *const ObjMeta;
                let other_ud = (*other_meta).userdata as *const MockUserdata;
                (*other_ud).value
            } else if other_any.get_tag() == TypeTag::I32 as u8 {
                I32::try_from(other_any.clone()).unwrap().val()
            } else {
                0
            };

            I32::new((*self_ud).value + other_value).into()
        }
        _ => ErrNum::new(NOTIMPL, builtin_strerror).into(),
    }
}

#[test]
fn test_object_lifecycle_and_dispatch() {
    let mut args = [I32::new(100).into()];
    let obj_res = Object::new(mock_class_func, &mut args);
    assert!(obj_res.is_ok());

    let obj = obj_res.unwrap();
    let obj_any: AnyValue = obj.into();
    
    assert_eq!(obj_any.get_tag(), TypeTag::Obj as u8);
    assert!(!obj_any.is_error());

    unsafe {
        let meta_ptr = canonicalize_ptr(obj_any.get_payload()) as *mut ObjMeta;
        let ud = (*meta_ptr).userdata as *mut MockUserdata;
        assert_eq!((*ud).value, 100);
    }

    let add_arg = I32::new(50).into();
    let res = obj_any.operator(Op::Add, &add_arg, &Nil::new().into());
    assert_eq!(res.get_tag(), TypeTag::I32 as u8);
    assert_eq!(I32::try_from(res).unwrap().val(), 150);
}

#[test]
fn test_object_error_unwinding() {
    unsafe extern "C" fn failing_class_func(
        userdata: *mut *mut libc::c_void,
        op: i32,
        _nargs: usize,
        _args: *mut AnyValue,
    ) -> AnyValue {
        let op_enum = std::mem::transmute::<i32, Op>(op);
        match op_enum {
            Op::Ctor => {
                let ud = Box::into_raw(Box::new(MockUserdata {
                    value: 0,
                    should_fail_is_err: true,
                    alloc_id: 999,
                })) as *mut libc::c_void;
                *userdata = ud;
                Nil::new().into()
            }
            Op::Dtor => {
                if !userdata.is_null() && !(*userdata).is_null() {
                    let _ = Box::from_raw(*userdata as *mut MockUserdata);
                    *userdata = std::ptr::null_mut();
                }
                Nil::new().into()
            }
            Op::IsErr => Bool::new(true).into(),
            _ => ErrNum::new(NOTIMPL, builtin_strerror).into(),
        }
    }

    let mut args = [];
    let result = Object::new(failing_class_func, &mut args);
    assert!(result.is_err());
}

#[test]
fn test_object_deep_copy() {
    let mut args = [I32::new(250).into()];
    let obj = Object::new(mock_class_func, &mut args).unwrap();
    let obj_any: AnyValue = obj.into();

    // Clone the object using clone() which triggers Op::Copy
    let cloned_any = obj_any.clone();

    assert_eq!(cloned_any.get_tag(), TypeTag::Obj as u8);
    assert_ne!(obj_any.get_payload(), cloned_any.get_payload());

    unsafe {
        let original_meta = canonicalize_ptr(obj_any.get_payload()) as *mut ObjMeta;
        let cloned_meta = canonicalize_ptr(cloned_any.get_payload()) as *mut ObjMeta;

        assert_ne!(original_meta, cloned_meta);
        let orig_ud = (*original_meta).userdata as *const MockUserdata;
        let clone_ud = (*cloned_meta).userdata as *const MockUserdata;
        assert_ne!(orig_ud, clone_ud);

        assert_eq!((*orig_ud).value, 250);
        assert_eq!((*clone_ud).value, 250);
    }
}

#[test]
fn test_match_anyvalue() {
    // Test Nil match
    let val_nil: AnyValue = Nil::new().into();
    match val_nil.into_match() {
        MatchAnyValue::Nil(_) => {}
        _ => panic!("Expected Nil"),
    }

    // Test Bool match
    let val_bool: AnyValue = Bool::new(true).into();
    match val_bool.into_match() {
        MatchAnyValue::Bool(b) => assert!(b.val()),
        _ => panic!("Expected Bool"),
    }

    // Test I32 match
    let val_i32: AnyValue = I32::new(42).into();
    match val_i32.into_match() {
        MatchAnyValue::I32(i) => assert_eq!(i.val(), 42),
        _ => panic!("Expected I32"),
    }

    // Test I64 match (heap-allocated)
    let val_i64: AnyValue = I64::new(-9999).into();
    match val_i64.into_match() {
        MatchAnyValue::I64(i) => assert_eq!(i.val(), -9999),
        _ => panic!("Expected I64"),
    }
}

