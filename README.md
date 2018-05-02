pdf-sigil
=========

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Word **sigil** in name stands for latin word *sigillum*, which means **seal** or **stamp**.

### Build

Performs build with output into "build" directory. After those steps, there will be a **static and shared library** and also **selftest executable**.

```shell
mkdir build
cd build
cmake ..
make
```

The selftest is run automatically during the make, but if you want to re-run it, use one of the following commands (from the build directory):

```shell
make run_tests # producing default output level
make run_tests_verbose # verbose output level
make run_tests_quiet # without output
```
