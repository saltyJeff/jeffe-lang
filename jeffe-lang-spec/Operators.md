All the listed operators lie inside `enum jeffe_op`, which is always positive and passed into `jeffe_class_fn` (see [[ABI#Objects]])

Classes do not have to implement all of the following, and may return `jeffe_value_errnum(NOTIMPL, builtin_strerror)` instead.

Users should set their compiler's settings so that their `classfn` handles each different operator as a fast `switch`-type instruction.
## Core
- `ctor()`: initializes the `userdata`
	- returns either an error or `nil`
- `dtor()`: destroys the `userdata`
	- returns `nil`
- `copy(other: struct jeffe_value)`: performs `a = other`
	- returns the copy.
- `get(key: struct jeffe_value)`: performs `a[key]`
	- returns the value for a given key
- `set(key: struct jeffe_value, val: struct jeffe_value)`: performs `a[key] = val`
	- returns `nil`
## Meta 
- `is_err()`: returns a Boolean indicating whether the value should be interpreted as an error. Used to enable users to define complex errors. See [[ABI#Objecs]]
	- returns `bool`
- `classname()`: returns a `const char *` indicating the name of the class.
## Mathematical
### Arithmetic
- `add(other: struct jeffe_value)`: performs `a + other`
	- returns the result
- //sub
- mul
- div
- modd (two ds because mod is a seserved word)
### Reverse Arithmetic
- radd
- rsub
- rmul
- rdiv
- rmodd
### Logical/Bitwise
- log_not
- shl
- shr
- bit_and
- bit_or
- bit_not
**NOTE:** `log_and` and `log_or` are implemented in the runtime to allow for shortcircuiting
### Compare
* `cmp(other: struct jeffe_value)`: performs `a <=> other`
	* returns negative value for less, zero for same, positive for more
## Iteration
- `len()`: get the length of the collection
	- returns the length (`uint64_t`)
- `iter(iter_obj: struct jeffe_value)`: iterates
	- if `iter_obj` is `nil`, return the first `(value, iter_obj)` tuple. See [[Builtins#tuple]]
	- if at the last iteration, return `(value, nil)`