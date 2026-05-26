# Code structure
This is a Rust project.

- Source goes in `src/`
- Tests go in `test/`
- Specification documents go in `jeffe-lang-spec/`

# Instructions
You should keep the README.md, source, and tests updated and consistent. 

If there is conflict in the spec docs, or the spec docs are unclear, do not edit them. Ask me for permission and give me suggestions first.

When making changes, first test the changes, and if they pass, make the change. If they fail, tell me and I'll fix them.

Be sure to run cargo fmt and cargo clippy to format the code and check for clippy lints. 

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

Use DRY principles but not to excess with super macros. Write as an experienced systems programmer writing maintainable code, not a vibe-coding web dev. Do not leave excessively detailed comments/reasoning traces. Except that documentation comments are acceptable.
Write rusty-code.

Do not install new cargo packages without asking my permission.