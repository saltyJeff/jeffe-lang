# jeffe-lang

## Building and Testing

Since command execution within sandboxed agents is restricted on Windows environments, please execute the build and verification steps locally in your terminal:

### 1. Execute the Test Suite
Verify that all ABI features, boxing mechanics, deep copying, memory management, and dynamic objects execute correctly:
```powershell
cargo test --test lib_tests
```

### 2. Check Code Style and Quality
Format files according to standard Rust style guides and check for compiler lints:
```powershell
cargo fmt -- --check
cargo clippy -- -D warnings
```
