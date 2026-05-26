use libc::c_void;

/// Get the actual usable size of an allocated memory block from the OS allocator.
/// Returns 0 if the pointer is null.
///
/// # Safety
/// `ptr` must be a null pointer or a valid pointer allocated by the system allocator
/// (e.g., via `libc::malloc`). Passing any other pointer is undefined behavior.
#[inline]
pub unsafe fn sizeof_alloc(ptr: *const c_void) -> usize {
    if ptr.is_null() {
        return 0;
    }

    os_sizeof_alloc(ptr as *mut c_void)
}

// Linux (glibc)
#[cfg(target_os = "linux")]
#[inline(always)]
unsafe fn os_sizeof_alloc(ptr: *mut c_void) -> usize {
    extern "C" {
        fn malloc_usable_size(ptr: *mut c_void) -> usize;
    }
    malloc_usable_size(ptr)
}
// macOS and iOS
#[cfg(any(target_os = "macos", target_os = "ios"))]
#[inline(always)]
unsafe fn os_sizeof_alloc(ptr: *mut c_void) -> usize {
    extern "C" {
        fn malloc_size(ptr: *const c_void) -> usize;
    }
    malloc_size(ptr)
}
// Windows (MSVC CRT)
#[cfg(target_os = "windows")]
#[inline(always)]
unsafe fn os_sizeof_alloc(ptr: *mut c_void) -> usize {
    extern "C" {
        fn _msize(ptr: *mut c_void) -> usize;
    }
    _msize(ptr)
}
