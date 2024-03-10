# YulaLang

## YulaLang its a concatinative programming language

## author - yula, telegramm - @imaginativemurder

## its programming language than writen in C++

## About language

```txt
YulaLang its a stack-based language.
YulaLang uses reversed-polish notation.
its mean 1 + 1 its 1 1 +
print("hello") its "hello" puts
operations pushes values with his types
on the stack, and other operations pops values.

YulaLang compiling source into IR
(Intermidian Representation) or bytecode.
After that IR typechecks and compilled down to
assembly (NASM x84_64 32-bit).
```

# goals

- [x] Compiled to a native instruction set (only x86_64 for now)
- [x] [Turing-complete]
- [x] Statically typed (the type checking is inspired by [WASM validation](https://binji.github.io/posts/webassembly-type-checking/))
- [ ] impliment procedures
- [ ] implement constants
- [ ] optimization on assembly generation

# Exampes

Hellow world
```yula
include "std"

"Hello, World!\n" puts
0
```

Simple loop

```yula
include "std"

0 while dup < 10 do
	dup puti // puti - put int
	1 +
end
```

