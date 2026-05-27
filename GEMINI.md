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

At the entry of functions taking `jeffe_value` parameters, proactively check if unpacking them immediately into `jeffe_value_holder` structs simplifies downstream checks, comparisons, and type switches.

Leverage standard C++17 mechanisms (like `if constexpr`, templates, and lambda/predicates) to collapse parallel multi-type switch structures and repeating dispatcher functions into DRY higher-order templates.