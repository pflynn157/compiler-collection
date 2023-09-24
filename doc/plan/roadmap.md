## Roadmap

The following is our to-do list for Riya:


#### Codebase Changes

* (DONE) Redesigned test system
* (DONE) Add a dedicated lexical analysis test
* (DONE) Replace the C-based lexer with C++ lexer
* (DONE) Rebase AST- all functions in "ast.cpp"
* (DONE) Rebase AST- functions inherit from statements, blocks on the global scope
* (DONE) Rebase AST- merge all AST headers into one
* (DONE) Rebase AST- use struct instead of class/helper functions
* Create common functions between various compiler entry points
* Rename compiler output
* Redo license headers
* Remove PTASM


### LLIR
* (DONE) Add LLIR base to compiler
* Fix- equations with function calls may not load in the right order. Calls should always come first
* Conflict with return register
* Mod operation doesn't work
* Switch to smart/safe pointers


### Features

* Floating-point datatypes
* Separate declarations for functions and procedures (def and func)
* Pure functions for math (this would be used for sets)
* For-loops
* Range-loops
* Array range loops
* Do-while loops
* Infinite loops
* Vectors (arrays with size and data)
* Matrices (2D arrays with size and data)
* Tensors (3D arrays with size and data)
* Set datatype (node graph)
* Map function for arrays (generates a for-loop to fill with data)
* Threading/HPC
* For stdlib functions, scan AST and import functions as needed
* "With" keyword for file IO
* Default to main when no functions
* Switch "|" or bor
* Latex-based comments
* Switch parens in function calls to curly braces
* Variadiac arguments


### Standard Library

* (DONE) Create core object with "_start"
* (DONE) Create corelib with "out", "syscall", "malloc", and "free"
* (DONE) Redesign the print function
* Corelib with file IO
* Corelib with terminal IO (use print only)
* Corelib with math (math.h)

