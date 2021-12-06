StaticMemoryAnalyzer
===============

StaticMemoryAnalyzer is a lightweight tool that aim to generate
in a simple way information about compile-time memory consumption.The memories that are focussed are RAM, Cache, Stack.
This tool does not take dynamic memory allocation into account. Thus it can be a useful tool to utilize within a continuous integration workflow.

Supported Compilers
-------------------

The code is meant to work on the following compilers:

- GCC 7.4.0 (or later) ( with explicit C++17)
- Visual C++ 16 2019


## License
See the `LICENSE` file for details. StaticMemoryAnalyzer is licensed under the
MIT license.