---
title: "Lab 2 -- ARM Assembly 2"
author: [Dr Dimi Racordon]
date: "22.9.2025"
version: "1.0.0"

module: "202"
ue: "202.1"
course: "Operating Systems"

# LaTex specific
fontsize: 10pt
caption-justification: centering
---

<style>
r { color: Red }
y { color: Yellow }
FIXME { color: Yellow }
TODO {color: Blue}
</style>

# Objective

The objective of this laboratory is to familiarize yourself with the basics of low-level memory management using the ARM assembly language.
Specifically, we'll learn how to:

- manipulate the stack pointer;
- allocate and deallocate heap memory.

The estimated duration of this lab is **4 periods**.
The result of Task 5 must be submitted on [isc.hevs.ch/learn](https://isc.hevs.ch/learn) no later than Sunday 12th October at 23:59 (CEST).

# Setup

Assembly languages typically target a family of related CPUs.
As we will use 64-bit ARM assembly, you will have to setup a specific architecture (i.e., [AArch64](https://developer.arm.com/documentation/102374/0102)) to complete this lab.
Fortunately, we can use containers to emulate such an architecture on top of most modern systems.

1. Install [Docker Desktop](https://www.docker.com/) and [Visual Studio Code (VS Code)](https://code.visualstudio.com) on your machine.
2. In VS Code, install the [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) extension.
3. Download (or clone using git) the following repository: [https://github.com/ISC-HEI/202.1-os](https://github.com/ISC-HEI/202.1-os).
4. Open the `lab-2` folder of this repository with VS Code and click on "Reopen in Container" in the bottom-right dialogue. If the dialog does not show, you can press `Ctrl+Shift+P` to open the command palette and type "Reopen in Container".

> Make sure to open the lab folder **directly** to let VS Code detect that a Docker container has been configured.

VS Code will reopen and start a Docker container configured to run Linux on a 64-bit ARM architecture (emulated if necessary).
Once the container is ready, open a terminal in VS Code and run the following command to confirm that your system is up and running:

```bash
uname -m
```

The command should print `aarch64` on your terminal.

> It should be possible to complete this lab if you are running Linux on a 64-bit ARM architecture natively.
> Using a container is nonetheless recommended to ensure that your environment is properly configured with all the tools that will be necessary for this lab and its successors.

# Manipulating the Call Stack

In Linux (and in most other systems), each process is given access to a fixed pool of memory that is managed like a LIFO (last-in-first-out) stack.
This pool of memory is often called the "call stack" or simply the "stack".
Its main is to store the value of local variables that either do not fit into registers or must persist after a function call.
The command-line arguments of the process are also stored on the stack.

## Allocating Memory

The stack grows downward: the bottom of the stack has a higher address than its top.
The special register `sp` stores a pointer to the top of the stack.
Allocating memory on the stack amounts to decrementing the value in `sp` whereas deallocating memory amounts to incrementing that value.
The architecture's specification requires that `sp` be aligned at a 16-byte boundary.
In other words, **`sp` must always be a multiple of 16**.

The following program uses the stack to swap the contents of two registers:

```asm
.text
.global _start
_start:
  sub  sp, sp, #16            // push 16 bytes to the stack
  mov  x0, #93
  mov  x8, #42
  stp  x0, x8, [sp]           // store x0 and x8 on the stack
  ldp  x8, x0, [sp]           // load the contents of the stack to x8 and x0
  add  sp, sp, #16            // pop 16 bytes from the stack
  svc  #0
```

## Command-line Arguments

The operating pushes the command-line arguments of a process on the stack at the beginning of its execution.
The top of the stack (i.e, the value pointed to by `sp`) contains the number of command-line arguments.
Pointers to these arguments are stored next.

For example, if one executes `ls -a somewhere`, the stack will be laid out so that:

- `sp` points to 3;
- `sp+0x08` points to a null-terminated string `/bin/ls`;
- `sp+0x10` points to a null-terminated string `-a`; and
- `sp+0x18` points to a null-terminated string `somewhere`.

> Readers familiar with C/C++ may note that this particular layout is reminiscent of the traditional parameter list declared on the `main` function of a program: `int main(int argc, char** argv)`.
> That is no coincidence! `argc` will denote the number of arguments and `argv` will be an array of pointers to the values of these arguments.

The following excerpt shows how to get a pointer to the value of first command line argument, which always denote the name of the executable:

```asm
_start:
  add  x0, sp, #8
  ldr  x1, [x0]               // x0 = argv
  ldr  x2, [x1]               // x1 = argv[0]
```

## Calling Functions

The [Procedure Call Standard](https://developer.arm.com/documentation/102374/0103/Procedure-Call-Standard) specifies the registers whose values should not be corrupt (i.e., overridden) when a function is called.
Specifically, registers `x19` to `x28` are said to be *callee-saved*, meaning that any function using these registers should restore their original value before returning.
Further, registers `x29` and `x30` denote have special significance:

- `x29` denotes the *frame pointer*, which identifies the start of the current call frame; and
- `x30` denotes the *link register*, which identifies where control-flow should return after a function call.

> Forgetting to restore the link register may prevent the `ret` instruction from behaving properly!

It is customary at the beginning of a function to store `x29` and `x30` on the stack and restore these registers before the function returns.
The following excerpt illustrates:

```asm
_factorial:
  stp  x29, x30, [sp, #-32]!  // store fp and lr on the stack
  mov  x29, sp                // assign the frame pointer
  str  x28, [sp, #16]         // store x28 on the stack
  mov  x28, x0                // write x0 to a callee-save register
  subs x0, x0, #1             // x0 -= 1
  ble  _factorial.one         // if x0 <= 1 { goto _factorial.one }
  bl   _factorial             // x0 := factorial(x0 - 1)
  mul  x0, x0, x28            // x0 *= x28
  b    _factorial.ret         // goto _factorial.ret
_factorial.one:
  mov x0, #1                  // x0 = 1
_factorial.ret:
  ldr  x28, [sp, #16]         // restore x28
  ldp  x29, x30, [sp], #32    // restore fp and lr
  ret                         // return
```

## Addressing Modes

There are several ways to specify the address from/ro which an instruction should read/write.
Some of them are shown in the table below, where `R` stands for either an **extended** general-purpose register (e.g., `x5`) or the stack pointer (i.e., `sp`).
More documentation can be found on [this page](https://developer.arm.com/documentation/102374/0103/Loads-and-stores---addressing).

| Name | Syntax | Effect |
| - | - | - |
| Register | `[R]` | accesses `R` |
| Register with offset | `[R, #i]` | accesses `R + i` |
| Register with pre-increment | `[R, #i]!`  | computes `R += i` and accesses `R` |
| Register with post-increment | `[R], #i`  | accesses `R` and computes `R += i` |

## Task 1 -- Fold

`foldright` is an algorithm that is used to "fold" the elements of a collection into a single value, using some combinator.
In Scala, for example, the expression `ArraySeq(1, 2, 3).foldRight(0)(_ + _)` computes `6`.
In this example, the collection is an array of 3 elements and the combinator is the integer addition.
The algorithm is called `foldright` because elements are visited right to left.
Written imperatively, the above sequence is equivalent to the following block:

```scala
var accumulator = 0
for x <- ArraySeq(1, 2, 3).reverse do
  accumulator + x
accumulator
```

Open the file `fold.s` and locate the label `_foldright`, which marks the star of a function.
Your task is to implement the definition of this function that satisfies its specification.
You are free to opt for an iterative or recursive approach.

To test your function, you can compile the program and give it a list of numbers.
A successful implementation will write their sum on the console.
A usage example is shown below.

```bash
./compile.sh fold.s
./fold sum 1 2 3
```

Do not hesitate to use other functions to help you develop and debug your implementation.
In particular, you can use `_printa` to print an address and `_printn` to print a number.

## Task 2 -- Insert

Notice that `_foldright` is a higher order function.
Its fifth parameter, which represent the combinator, is expected to the the address of a function accepting its parameters in registers `x0` and `x1`.

- If you type `./fold sum ...`, the program uses `_add` as the combinator, which simply adds the value of each element to the accumulator.
- If you type `./fold sort ...`, the program uses `_insert` as the combinator, which inserts elements into a binary search tree.

Internally, `_insert` calls `_tree_insert` to process each element.
Your task is to implement the latter.

In the same file as the one in which you have implemented `_foldright`, locate the label `_tree_insert` and write an implementation that satisfies the functions's specification.

Your implementation can use `_aalloc` to allocate new tree nodes on the heap.
Observe that this function expects two arguments.
The first is the number of bytes to allocate and the second is the alignment of this allocation, expressed as the exponent of a power of two.
For example, the following call requests the allocation of 24 bytes (the size of a tree node) aligned at 16 (i.e., $2^4$).

```asm
_somewhere:
  mov  x0, #24
  mov  x1, #4
  bl   _aalloc
```

To test your function, you can compile the program and give it a list of numbers.
A successful implementation will write them in increasing order.
A usage example is shown below.

```bash
./compile.sh fold.s
./fold sort 1 5 3 7 2
```

## Submission

Once you're done, submit your while program, which should the implementations of `_foldright` and `_tree_insert`, on [isc.hevs.ch/learn](https://isc.hevs.ch/learn).
Your lab will be graded on the basis of the correctness and clarity of your implementation.
