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

[ x ] compilled
[ x ] native
[ x ] turing complete
[   ] procedures
[   ] consts
[   ] optimizing assembly

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

