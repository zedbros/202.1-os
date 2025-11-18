---
title: "Lab 4 -- Schedulers"
author: [Dr Dimi Racordon]
date: "18.11.2025"
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

The objective of this laboratory is to understand how operating systems schedule tasks.

The estimated duration of this lab is **4 periods**.
The result of your work must be submitted on [isc.hevs.ch/learn](https://isc.hevs.ch/learn) no later than Sunday 23rd November at 23:59 (CEST).

# Setup

You will need a modern C++ compiler to complete this lab.
You can either install clang or gcc on your own system or use the instructions below to run a C++ compiler using Docker.

1. Install [Docker Desktop](https://www.docker.com/) and [Visual Studio Code (VS Code)](https://code.visualstudio.com) on your machine.
2. In VS Code, install the [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) extension.
3. Download (or clone using git) the following repository: [https://github.com/ISC-HEI/202.1-os](https://github.com/ISC-HEI/202.1-os).
4. Open the `lab-4` folder of this repository with VS Code and click on "Reopen in Container" in the bottom-right dialogue. If the dialog does not show, you can press `Ctrl+Shift+P` to open the command palette and type "Reopen in Container".
   VS Code will reopen and start a Docker container configured to run Linux on a 64-bit ARM architecture (emulated if necessary).

> Make sure to open the lab folder **directly** to let VS Code detect that a Docker container has been configured.

Whether or not you are using Docker, you can check if your system is ready by compiling the existing code with the following command:

```bash
make
```

# Schedulers

Open the file `include/Scheduler.hh`.
This header implements some data structures to emulate the behavior of various schedulers for a virtual machine.

The interface of a scheduler is described by the abstract class `Scheduler`, which defines a virtual method `Scheduler::step`, whose specification is described in the comments.
The template parameter `core_count` describes the number of CPU cores that the virtual machine emulates.

The goal of this laboratory is to implement schedulers satisfying this interface.
An example is given with the struct `FCFS` in line 63, which implements a first come first serve strategy.

## Task 1 — Round-robin

Open the file `include/Scheduler.hh` and locate the declaration of the `RoundRobin` struct in line 114.
This struct is meant to implement a round-robin scheduler.

Your task is to implement the struct's constructor and the method `RoundRobin::step`.

## Task 2 — Priority scheduling

Open the file `include/Scheduler.hh` and locate the declaration of the `Priority` struct in line 135.
This struct is meant to implement a round-robin scheduler.

Your task is to implement the struct's constructor and the method `Priority::step`.
The priority of a task is described by the field `Task::priority`, declared in line 20.

## Testing

To test your implementation you can uncomment some or all of the tests in `test/test-all.cc`.
The test suite can be run with the following command:

```bash
make test
```

Don't hesitate to write additional tests.
Your submission will be tested on the tests in `test/test-all.cc` and additional "hidden" test cases.

## Submission

Once you're done, submit your modified `Scheduler.hh` file on [isc.hevs.ch/learn](https://isc.hevs.ch/learn).
Your lab will be graded on the basis of the correctness and clarity of your implementation.
