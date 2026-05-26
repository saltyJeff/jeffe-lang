All the listed operators lie inside a `enum jeffe_lang::Op`, which is always positive and passed into `classfns` (see [[ABI#Objects]])

Classes do not have to implement all of the following, and may return `ErrNum(NOTIMPL, builtin_strerror)` instead.

Users should set their compiler's settings so that their `classfn` handles each different operator as a fast `switch`-type instruction.
## Core
- `ctor()`: initializes the  `userdata`
	- returns either an error or `Nil`
- `dtor()`: destroys the `userdata`
	- returns `Nil`
- `copy(other: &Value)`: performs `a = other`
	- returns the copy.
- `get(key: &Value)`: performs `a[key]`
	- returns the value for a given key
- `set(key: &Value, val: &Value)`: performs `a[key] = val`
	- returns `Nil`
## Meta 
-  `is_err()`: returns a Boolean indicating whether the value should be interpreted as an error. Used to enable users to define complex errors. See [[ABI#Object Implementation]]
	- returns `Bool`
- `name()`: returns a `const char *` indicating the name of the class.
## Arithmetic
- `add(other: &Value)`: performs `a + other`
	- returns the result
- //sub
- mul
- div
- modd (two ds because mod is a seserved word)
## Reverse Arithmetic
- radd
- rsub
- rmul
- rdiv
- rmodd
## Logical/Bitwise
- log_not
- shl
- shr
- bit_and
- bit_or
- bit_not
**NOTE:** `log_and` and `log_or` are implemented in the runtime to allow for shortcircuiting
## Compare
* `cmp(other: Value&)`: performs `a <=> other`
	* returns negative value for less, zero for same, positive for more
## Iteration
- `len()`: get the length of the collection
	- returns the length (`u64`)
- `iter(iter_obj: Value)`: iterates
	- if `iter_obj` is `Nil`, return the first `(value, iter_obj)` tuple. See [[Builtins#tuple]]
	- if at the last iteration, return `(value, nil)`