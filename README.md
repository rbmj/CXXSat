CXXSat
======

This project is a clang plugin that transforms C code into a boolean
satisfiability problem and hands the problem off to a SAT solver for
solving.

Right now the plugin relies on the minisat library and clang version
3.5.  It can be compiled with any C++14 compliant compiler; however,
it requires clang 3.5.0's headers and libraries to be present on the
system due to API incompatibilities between clang versions.

The build system uses cmake, and build/install is a simple matter of
cmake/make/make install.

Limitations
-----------

Currently, the plugin does not handle C++ objects/methods.  Also, the
function currently may not call any functions or use any loops.  That
capability is planned for a future release.

Only primitive integer types (bool, char, short, int, long, etc.) may
be used in the function.  No structs/arrays may be used.  Typedefs are
supported, so long as they are to a primitive type.

Currently there is no way to specify constraints on the input.

For casting rules, the plugin assumes that the `int` datatype is 32
bits wide, but the actual sizes of variables are given by the
compiler.  It is planned for the plugin to query the compiler for this
size instead in a future release.

Usage
-----

Invocation:

    $ cxxsat -function=foo -value=0xdeadbeef [OPTIONS] FILES [COMPILER_SPEC]

Where:

 - The 'function' argument specifies the function to generate constraints
   for.  Currently this function should not be overloaded if the input is
   in C++.
 - The 'value' argument specifies what the function's return value should
   be for some values of the argument
 - The `COMPILER_SPEC` is either:
    - Empty, in which case the tool will search for `compile_commands.json`
      in the source directory of the first provided input file, and then
      all parent directories of the same.  This file is the result of
      using the -DCMAKE_EXPORT_COMPILE_COMANDS=ON option when building a
      source tree.
    - `-p=DIRECTORY`, where DIRECTORY is the path to search for the
      `compile_commands.json` file.
    - `-- [FLAGS]`, where `[FLAGS]` are the flags to pass to the compiler.

Current options supported by the plugin:

 - `-dump` specifies that instead of solving the problem, the plugin should
   dump a representation of the problem in the DIMACS file format to
   standard output.


Examples
--------

An example problem:

foo.c:

    #include <stdint.h>
    int32_t add(int32_t x, int32_t y) {
        return x + y;
    }

Invocation:

    $ cxxsat -function=add -value=0x1337 foo.c --
    x: 4600
    y: 319


Paper
-----

This project started as a final project for SI485 at USNA for the fall
AY 2015 semester.  A survey paper on recent SAT techniques and some
(only mildly scientific) benchmarks are located in the paper/ directory.
The paper can be built (it has it's own Makefile) with gnuplot and a
Tex distirbution with LaTeX/BibTeX/dvipdf and python.


Clang Tips
----------
To dump the AST:

    clang -fsyntax-only -Xclang -ast-dump

