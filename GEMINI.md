# Code structure
This is a C project.

- Source goes in `src/`
- Public includes go in `inc/`. Private includes go in `src/`
- Tests go in `test/`
- Specification documents go in `jeffe-lang-spec/`

# Instructions
You should keep the README.md, source, and tests updated and consistent. 

If there is conflict in the spec docs, or the spec docs are unclear, do not edit them. Ask me for permission and give me suggestions first.

Public includes should contain javadoc/doxygen style doc comments.
They should also be C headers, not C++ ones.

When making changes, first test the changes, and if they pass, make the change. If they fail, tell me and I'll fix them.

Be sure to run `clang-format` and `clang-tidy`

When committing your changes, each commit should include:
- Subject: 
    - Type of change (feat, fix, docs, style, refactor, test, perf, ci, build, chore), followed by "(ai)".
    - Description of change
    - E.g. `feat(ai): add new feature`
- Body:
    - Concise restatement of the prompt, professional for documentary purposes.
    - Acceptance criteria for the prompt.

When asked to create tests, make them modular and simple. Do not create mega kitchen sink tests.
Similarly, do not make mega kitchen sink files.

Use DRY principles but not to excess with super macros. Write as an experienced systems programmer writing maintainable code, not a vibe-coding web dev. Do not leave excessively detailed comments/reasoning traces. Except that terse declaration documentation comments are acceptable. Long stepped comments are likely better served by breaking the function up into smaller ones.

Use C++ style casts over C-style casts

Google antigravity IDE is now bugged and cant run cmake. Tell me and I can run the code.

Before performing manual masking (`PAYLOAD_MASK`), raw bitwise shifts, or pointer decompression (`ptr_uncompress`), always prefer using the standard `jeffe_value_holder` via `jeffe_value_held_data` to access typetags and union fields. Only fall back to manual bit manipulation if absolute raw performance in a hot assembly loop warrants it.

At the entry of functions taking `jeffe_value` parameters, proactively check if unpacking them immediately into `jeffe_value_holder` structs simplifies downstream checks, comparisons, and type switches. Avoid the redundant idiom of calling `jeffe_value_typetag()` followed by retrieving `jeffe_value_held_data()` (or vice-versa), since the `jeffe_value_holder` struct already contains the `typetag` field.

Leverage standard C++17 mechanisms (like `if constexpr`, templates, and lambda/predicates) to collapse parallel multi-type switch structures and repeating dispatcher functions into DRY higher-order templates.

# Creating classes
If asked to design or implement a class, ensure the following constraints and architectural guidelines are met:
1. **Public API Declarations:** Declare the corresponding `class_fn` (e.g. `jeffe_tuple_class_fn`) and the factory constructor helper (e.g. `jeffe_tuple`) in the public header [jeffe_lang.h](inc/jeffe_lang.h) with Doxygen/Javadoc-style documentation.
2. **Argument Safety Guards:** Inside the `class_fn`, proactively verify the argument count `argn` for each operation. For single-parameter operations (such as `JEFFE_OP_GET` or `JEFFE_OP_ITER`), strictly assert that `argn == 1`, returning `JEFFE_ERRNO_ARGTYPE` if the condition is not met.
3. **C++ Style Casts:** Cast `userdata` pointers using modern C++ static casts (e.g. `static_cast<class_data*>(*userdata)`) instead of legacy C-style casts.
4. **Memory Hygiene & Reference Semantics:**
   * **Parameter Lifetime Independence:** Class functions must have absolutely no dependency on the lifetime of the arguments passed in `argv`. The values in `argv` are caller-owned and transient.
   * **Retaining Values:** Any value the class needs to retain or store in its `userdata` past the lifetime of the current function call MUST be explicitly duplicated using `jeffe_copy()`.
   * **Proper Destruction:** Every retained value must eventually be destroyed exactly once via `jeffe_destroy()` (typically during the destructor operation `JEFFE_OP_DTOR` when freeing the class's allocated data structure).
   * **Value vs. Reference Types:**
     * Primitives and heap primitives (`i64`, `u64`, `f64`, `cstruct`) are deep-copied during assignment, requiring independent destruction of both original and copied handles.
     * Custom objects use shallow-copied shared references without reference counting. Therefore, ensure that custom objects inside collections are destroyed **exactly once** to prevent double-free segment faults.
   * **Local Leak Prevention:** In operations like `JEFFE_OP_ITER` or any operation returning a new collection/object, make sure to destroy any temporary or locally copied references created inside the class function before returning to prevent memory leaks.