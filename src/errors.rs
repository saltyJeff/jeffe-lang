pub type StrErrorFn = unsafe extern "C" fn(err: i8) -> *const libc::c_char;

/// `NOTIMPL`: error code indicating functional implementation is missing.
pub const NOTIMPL: i8 = 38; // Maps to standard POSIX ENOSYS

/// Built-in strerror resolver function.
/// Maps error codes (such as `NOTIMPL`) to static C-compatible string literals.
pub unsafe extern "C" fn builtin_strerror(err: i8) -> *const libc::c_char {
    if err == NOTIMPL {
        b"Functionality not implemented\0".as_ptr() as *const libc::c_char
    } else {
        b"Unknown error\0".as_ptr() as *const libc::c_char
    }
}
