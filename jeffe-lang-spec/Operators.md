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
### Type Promotion & Upcasting Rules
When evaluating mixed-type operations (such as primitive arithmetic or comparison operations between operands of different types), `jeffe-lang` performs automatic type upcasting based on the following hierarchy of precedence:

$$\text{F64} > \text{F32} > \text{U64} > \text{I64} > \text{U32} / \text{Char} > \text{I32}$$

Both operands are promoted to the highest precedence type between them before the operation is executed.

### Arithmetic
- `add(other: struct jeffe_value)`: performs `a + other`
	- returns the result of the addition
- `sub(other: struct jeffe_value)`: performs `a - other`
	- returns the result of the subtraction
- `mul(other: struct jeffe_value)`: performs `a * other`
	- returns the result of the multiplication
- `div(other: struct jeffe_value)`: performs `a / other`
	- returns the result of the division
- `mod(other: struct jeffe_value)`: performs `a % other`
	- returns the result of the modulo

### Reverse Arithmetic
Used as a fallback for objects when the left-hand side operand does not support the arithmetic operation directly.
- `radd(other: struct jeffe_value)`: performs `other + a`
- `rsub(other: struct jeffe_value)`: performs `other - a`
- `rmul(other: struct jeffe_value)`: performs `other * a`
- `rdiv(other: struct jeffe_value)`: performs `other / a`
- `rmod(other: struct jeffe_value)`: performs `other % a`

### Logical/Bitwise
- `log_not()`: performs logical negation `!a`
- `shl(other: struct jeffe_value)`: performs bitwise shift left `a << other`
- `shr(other: struct jeffe_value)`: performs bitwise shift right `a >> other`
- `bit_and(other: struct jeffe_value)`: performs bitwise AND `a & other`
- `bit_or(other: struct jeffe_value)`: performs bitwise OR `a | other`
- `bit_not()`: performs bitwise NOT `~a`
**NOTE:** `log_and` and `log_or` are implemented directly in the runtime compiler layer to allow for shortcircuiting.

## Compare
* `cmp(other: struct jeffe_value)`: performs `a <=> other`
	* The runtime comparative C API `struct jeffe_value jeffe_cmp(struct jeffe_value a, struct jeffe_value b)` invokes the metadata comparison sequence and returns a boxed `jeffe_value`:
		- A `jeffe_value` of type `i32` with value `-1` if `a < b`, `1` if `a > b`, or `0` if `a == b` for orderable types.
		- If the types are non-orderable (e.g. `ptr`, `cstruct`, `errnum`, or default `obj`), returns `0` (type `i32`) if they are equal, or a `JEFFE_ERRNO_NOTORDERED` `errnum` value if they are not equal.
	* Custom object implementations that intercept `JEFFE_OP_CMP` via `jeffe_class_fn` return a boxed comparative `jeffe_value` containing numeric components.

## Iteration
- `len()`: get the length of the collection
	- returns the length (`uint64_t`)
- `iter(iter_obj: struct jeffe_value)`: iterates
	- if `iter_obj` is `nil`, return the first `(value, iter_obj)` tuple. See [[Builtins#tuple]]
	- if at the last iteration, return `(value, nil)`