The following builtins are implemented and provided by the runtime, and are built on top of the underlying constructs in [[ABI]]
## errors
The following builtin error codes are defined inside `enum jeffe_builtin_errno` to represent structural and runtime failures:
- `JEFFE_ERRNO_UNDEFINED`: Represents undefined operations, such as invalid mathematical operations on incompatible types.
- `JEFFE_ERRNO_NOTIMPL`: Indicates that a specific operator or class function has not been implemented.
- `JEFFE_ERRNO_ARGTYPE`: Triggered when an incorrect number of parameters or invalid type arguments are passed to a class method or factory constructor.
- `JEFFE_ERRNO_OUTOFBOUNDS`: Represents indexing and element access violations that fall outside the bounds of a collection sequence.

## string
## list

## tuple
Tuples are immutable sequence collections represented by a custom object created via:
```c
struct jeffe_value jeffe_tuple(size_t argn, const struct jeffe_value *argv);
```

Tuples implement the following standard operators:
- **`JEFFE_OP_CTOR`**: Allocates the immutable sequence elements from the arguments array `argv`.
- **`JEFFE_OP_DTOR`**: Destroys all stored child references exactly once, then deallocates the tuple's state.
- **`JEFFE_OP_GET`**: Expects a single argument `key` representing the 0-indexed offset. Returns the element at the index, or triggers `JEFFE_ERRNO_OUTOFBOUNDS` if the index exceeds limits.
- **`JEFFE_OP_LEN`**: Returns the count of elements in the tuple as a `uint64_t` value.
- **`JEFFE_OP_ITER`**: Sequentially iterates through the tuple.
  - If `iter_obj` is `nil`, returns a custom tuple containing `(value_at_index_0, next_iterator_state)`.
  - If `iter_obj` is a valid iterator state indicating the next index, returns `(value_at_index, next_iterator_state)`.
  - If it is the last element, returns `(last_value, nil)`.

## dict
## set