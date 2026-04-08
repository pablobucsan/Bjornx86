# Björn

Björn is a statically typed, compiled systems programming language with a self-hosting toolchain built from scratch. The toolchain produces x86-64 Linux ELF executables with no dependency on LLVM, libc, or any external runtime.

## The Toolchain

```
source.bjo → bjornc2 → source.asm → bjornas2 → source.cub → bjornlk2 → a.elf
```

- **bjornc2** — single-pass compiler, written in C, emits x86-64 assembly
- **bjornas2** — two-pass assembler, written in Björn, produces `.cub` object files
- **bjornlk2** — linker, written in Björn, produces ELF executables
- **`.cub`** — custom binary object format designed for this toolchain

The assembler and linker are self-hosted — both are written in Björn and built by the toolchain itself.

## The Language

Björn is C-like with a focus on explicitness and control:

- Fixed-width integer types: `int8`, `int16`, `int32`, `int64`, `uint8`, `uint16`, `uint32`, `uint64`
- Explicit pointer types: `ptr<T>`
- Single inheritance with `class` and `object`
- Function overloading
- Manual memory management — no garbage collector
- No libc dependency — runtime is implemented via direct Linux syscalls

## Hello World

```bjorn
#use "bstdio.berry"

func void main(uint64 argc, ptr<str> argv)
{
    printf("Hello World!\n");
}
```

Build and run:

```bash
bjornc2 main.bjo
./a.elf
```

## Building

### Prerequisites

- GCC
- nasm (for bootstrapping bjornas and bjornlk from source)
- make

### Build the compiler

```bash
make build
```

### Build and deploy everything

```bash
sudo make deploy
```
This does the following:

- Installs `bjornc2` to `/usr/local/bin`
- Installs the standard library to `/usr/local/lib/bjorn`
- Sets up the BJORN_LIB_PATH environment variable in `~/.bashrc`

# Important note:
`make deploy` does not build or install the assembler (`bjornas`) or the linker (`bjornlk`). Those live in their own repositories. You can get the binaries there. Link at the end.

## How the full pipeline works
When you run:
`bjornc2 main.bjo`

`bjornc2` will automatically:

- Generate .asm files
- Call `bjornas` internally to produce `.cub` object files
- Call `bjornlk` internally to link everything into an ELF executable

So after you have installed `bjornc2` and have `bjornas` + `bjornlk` available in your PATH, you can build with a single command. You could also use the self-hosted and self-built versions, `bjornas2` and `bjornlk2`, then you have to specifiy the `-self` flag to the compiler. Make sure to run `bjornc2 -h` for a bit more information. It is adviced to also take a look at the Makefile script to understand the deploy process, specifically when it comes to the runtime libraries, as they are assemblerd with `bjornas`. You are free to mess around with that.

## Honest reality check

This project is first and foremost a learning exercise. I built it to deeply understand how the entire toolchain works — from source code to final binary — without relying on external frameworks.
Because of that goal:

- The toolchain is not production-ready.
- There are no optimisation passes (it targets correctness at roughly -O0 level).
- It only supports x86-64 Linux.
- Some parts are still young and may contain rough edges.
- The focus was on understanding and correctness, not speed or broad compatibility.

If you're looking for a polished, ready-to-use toolchain, this is probably not it yet.
If you're interested in how compilers, assemblers, and linkers actually work from first principles, then I hope this project is useful and I appreciate anyone taking the time to give it a try and bearing with my mistakes.

## Repository Structure

```
compiler/       — bjornc2 source (C)
bjorn-lib/
  src/          — Björn standard library source
  asm_extern/   — hand-written assembly (syscall wrappers etc.)
  cubs/         — compiled standard library objects
```

## Design

The toolchain is intentionally minimal and IR-free. The compiler performs a single pass from AST directly to x86-64 assembly with no intermediate representation. Register allocation uses a tree-scoped strategy that pins and unpins registers at AST node boundaries. The assembler uses a closed template system — roughly 50 mnemonics with typed operand templates and exact byte-size computation in the first pass.

The `.cub` object format is custom-designed for this toolchain, containing identifier, section, symbol, payload, and relocation blocks.

## Status

The toolchain is functional and self-hosting for the assembler and linker. It is under active development. Known limitations include no optimisation passes (the compiler targets correctness at `-O0` equivalent), and only `.text` and `.data` sections are supported.

## Related Repositories

- [BjornAssembler](https://github.com/pablobucsan/BjornAssembler) — bjornas2 source
- [BjornLinker](https://github.com/pablobucsan/BjornLinker) — bjornlk2 source
