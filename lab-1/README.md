---
title: "Lab 1 -- ARM Assembly 1"
author: [Dr Dimi Racordon]
date: "1.9.2025"
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

The objective of this laboratory is to familiarize yourself with the basics of the ARM assembly language for the purpose of communicating with an operating system.
Specifically, we'll learn how to:

- use general purpose registers to perform simple operations; and
- perform system calls.

The estimated duration of this lab is **4 periods**.
The result of Task 5 must be submitted on [isc.hevs.ch/learn](https://isc.hevs.ch/learn) no later than Sunday 28th September at 23:59 (CEST).

# Part 1 -- Setup

## Task 1 -- Configuring your container

Assembly languages typically target a family of related CPUs.
As we will use 64-bit ARM assembly, you will have to setup a specific architecture (i.e., [AArch64](https://developer.arm.com/documentation/102374/0102)) to complete this lab.
Fortunately, we can use containers to emulate such an architecture on top of most modern systems.

1. Install [Docker Desktop](https://www.docker.com/) and [Visual Studio Code (VS Code)](https://code.visualstudio.com) on your machine.
2. In VS Code, install the [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) extension.
3. Download (or clone using git) the following repository: [https://github.com/ISC-HEI/202.1-os](https://github.com/ISC-HEI/202.1-os).
4. Open the `lab-1` folder of this repository with VS Code and click on "Reopen in Container" in the bottom-right dialogue. If the dialog does not show, you can press `Ctrl+Shift+P` to open the command palette and type "Reopen in Container".

VS Code will reopen and start a Docker container configured to run Linux on a 64-bit ARM architecture (emulated if necessary).
Once the container is ready, open a terminal in VS Code and run the following command to confirm that your system is up and running:

```bash
uname -m
```

The command should print `aarch64` on your terminal.

> It should be possible to complete this lab if you are running Linux on a 64-bit ARM architecture natively.
> Using a container is nonetheless recommended to ensure that your environment is properly configured with all the tools that will be necessary for this lab and its successors.

## Task 2 -- Compiling a simple program

Open the file `trivial.s`, which should contain the following text:

```asm
.global _start
_start:
  mov x0, #42
  mov w8, #93
  svc #0
```

This program simply exits with status 42.
Compile it with the following command:

```bash
./compile.sh trivial.s
```

This command will produce an executable binary that you can run as follows:

```bash
./trivial
echo $?
```

The first line runs your program and the second displays its return status, which should be equal to `42`.

> The expression `$?` denotes the return status of the last executed command.

# Part 2 -- Arithmetic and branches

AArch64 provides 31 general purpose 64-bit registers, named `x0` through `x30`.
Those can also be used as 32-bit registers using `w0` through `w30` to refer to the 32 least significant bits of the corresponding 64-bit register.

The name of the register used in an instruction determines the size of the calculation.
In the following, for example, both instructions operate on the same register but the first performs a 32-bit integer addition whereas the second performs a 64-bit integer subtraction.

```asm
add w0, w0, #2
sub x0, x0, #4
```

The `cmp` instruction compares two integers and stores the result in a special register.
This result can be used to conditionally branch to a particular location using `b{c}`, where `c` is a condition code (e.g., `eq` for equal, `lt` for less than, etc.)

```asm
  cmp w0, #2
  blt tail
  lsl w0, w0, #1
tail:
  add w0, w0, #1
```

Here, for example, the `blt` instruction causes execution to skip the `lsl` instruction if and only if the contents of `w0` is less than `2`.
Otherwise, the `lsl` instruction is executed.
In either case, the program eventually performs the addition.
Note that `b` without a condition flag performs an unconditional branch.

The `bl` (branch and link) instruction is used to perform a function call.
Unlike a regular branch, `bl` copies the address of the next instruction into the link register (i.e., `lr`) which can then restored using the `ret` instruction.

```asm
  mov w0, #2
  bl  increment
  mov w8, #93
  svc #0
.incr:
  add x0, x0, #1
  ret
```

Here, for example, `incr` can be used as a function that increments the contents of `x0`.
The `bl` instruction moves control flow to the first instruction of this function and the `ret` instruction moves it to the instruction right after `bl` (i.e., the assignment of `w8`).

Note that the contents of general-purpose registers may change before and after a `bl`.
It is the responsibility of the caller to save the contents of these registers if necessary.

## Task 3 -- Fibonacci

The Fibonacci sequence is a series of natural numbers in which in each element is the sum of the two elements that precede it.
More formally:

$$
f_n = \begin{cases}
  n&\text{if } n \le 1 \\
  f_{n-1} + f_{n-2}&\text{otherwise}
\end{cases}
$$

Open the file named `fibonacci.s`, and then:

1. Add a routine that writes to `x0` the value of $f_n$ assuming $n$ is initially stored in `x0` and $f_n < 2^{64}$. Remember to end your routine with a `ret` instruction to give control flow back to the caller.
2. Modify the code under `_start` so that it calls your routine to write the value of $f_{12}$ to `x0` instead of storing `42`. Use a `bl` (branch and link) instruction to perform the call.

> You can use exit statuses as a rudimental "print debug" technique to observe the contents of your registers.
> However, note that exit statuses are truncated to 8 bits.
> For example, exiting with status `257` from your program will have the same observable effect as exiting with status `1`.

## Task 4 -- Printing numbers

Open the file `str_to_nat.s` and observe the function `_print_nat`.
This function prints the value of `x0` to the standard output, assuming it contains an unsigned integer less than $1000$.
Modify this function so that:

1. it can print any 32-bit unsigned integer; and
2. it exits the program with status `1` if `x0` contains a larger number.

> The instruction `strb w5, [x3, x1]` writes the least significant byte of `w5` in the buffer referred to by `x3` offset by the value of `x1`.
> In Scala, this instruction is roughly equivalent to the assignment `x3.update(x1, w5.toByte)`, assuming `x1` and `x5` are integers, and `x3` is an array of 8 bytes.

## Task 5 -- Printing PIDs

Create a file `pid.s` and write a program that obtains its process ID (PID) from the operating system and prints it to the standard output, re-using your `_print_nat` function to do so.

> The system call `getpid` returns the process ID of the calling program.

Assuming your program is compiled to an executable named `pid`, you can confirm that your program is behaving correctly using the command below.
If your program is correct, the same PID should be displayed twice.

```bash
./pid & echo $!
```

Once you're done, submit your program on [isc.hevs.ch/learn](https://isc.hevs.ch/learn).
Your lab will be graded on the basis of the correctness and clarity of your implementation.
