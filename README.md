pdf-sigil
=========

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

> This tool is a PoC on PKCS#1 digital signature verification in PDF with focus on minimizing external dependencies. Use only for educational purposes. Currently, there are other more advanced signature types in use that pdf-sigil does not support. Active development is stopped, PRs can be merged.

Word **sigil** in name stands for latin word *sigillum*, which means **seal** or **stamp**.

### Build

Performs build and puts the output into *build* directory. After these steps, there will be a **libpdfsigil** shared library with **selftest** executable and also **pdf-sigil** proof-of-concept executable. Also Doxygen documentation will be generated into *dir* directory.

```shell
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
