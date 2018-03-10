pdf-sigil
=========

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Word **sigil** in name stands for latin word *sigillum*, which means **seal** or **stamp**.

### Build

Performs build with output into "build" directory. After those steps, there will be a **static and shared library** and also **selftest executable**.

```shell
cmake -E make_directory build
cmake -E chdir build cmake ..
cmake -E chdir build make
```

The selftest is run automatically during the make, but if you want to re-run it, use one of the following commands:

```shell
cmake -E chdir build make run_tests # producing default output level
cmake -E chdir build make run_tests_verbose # verbose output level
cmake -E chdir build make run_tests_quiet # without output
```
