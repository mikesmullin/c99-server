# C99 Server

This is an example/base/template application showing how its possible to code a portable application using containers.

## My Stack

- IDE: VSCode (`code`) on Windows 11 w/ `clangd` language server
- Compiler: `clang` (build happens inside Container)
- Compile Target: Podman Desktop > Podman Machine (WSL2) > Podman Container, running Alpine Linux
- Debugger: `lldb` client, connected to remote `lldb-server` (in Container)

## My Code Style

Learn my particular approach to coding portable code in C99.

- see: [docs/my-code-style.md](docs/my-code-style.md)

## Install / Prerequisites

- Install LLVM v20.1.8 from [llvm/llvm-project releases](https://github.com/llvm/llvm-project/releases/)
  - (bug workaround) Install specifically Python 3.10.10 [Windows embeddable package (64-bit)](https://www.python.org/downloads/release/python-31010/) by extracting it to `C:\Program Files (x86)\LLVM\bin`.
- VSCode Extensions
  - Required
    - ms-vscode.cpptools
    - llvm-vs-code-extensions.vscode-clangd
    - llvm-vs-code-extensions.lldb-dap
  - Recommended
    - GitHub.copilot
    - GitHub.copilot-chat
    - bierner.markdown-mermaid
    - spmeesseman.vscode-taskexplorer
    - ms-vscode-remote.remote-wsl

## Build & Run (w/ Debug)

Use the VSCode `launch.json` launch configuration:

- `Debug main`: main program
- `Debug test`: currently open unit test (ie. `test/**/*.c`)

### LLDB Troubleshooting Tips

The main trick here is we are cross-compiling from (any OS) to Linux using Clang.  
Likewise, we are cross-debugging to Linux using LLDB.

- LLDB client and server versions should match

- You can automate the CLI by sending a chain of commands:
  ```sh
  lldb -o "platform select remote-linux" -o "platform connect connect://localhost:2345" 
  ```

- Connecting the client manually:
  ```
  # (probably unnecessary) disable ASLR in WSL2 podman machine
  settings set target.disable-aslr false

  target create build/main
  settings set target.source-map /app z:\tmp
  breakpoint set -f src/main.c -l 14
  breakpoint clear -f src/main.c -l 14
  process launch
  ```

- If you suspect the compiler is not emitting debug symbols correctly:
  ```sh
  readelf -S build/main
  readelf --debug-dump=info build/main | grep DW_AT_name
  nm -a build/main
  ```