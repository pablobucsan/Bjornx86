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

This builds bjornc2, compiles the standard library, and installs everything to `/usr/local/bin` and `/usr/local/lib/bjorn`.

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
