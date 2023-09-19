## Roadmap

The following is our to-do list for Riya:


#### Codebase Changes

* Move the output tests into their own directory
* Add a dedicated lexical analysis test
* Replace the C-based lexer with C++ lexer
* (DONE) Rebase AST- all functions in "ast.cpp"
* (DONE) Rebase AST- functions inherit from statements, blocks on the global scope
* (DONE) Rebase AST- merge all AST headers into one
* (DONE) Rebase AST- use struct instead of class/helper functions


### Features

* Floating-point datatypes
* Separate declarations for functions and procedures (def and func)
* For-loops
* Range-loops
* Array range loops
* Do-while loops
* Infinite loops
* Vectors (arrays with size and data)
* Matrices (2D arrays with size and data)
* Tensors (3D arrays with size and data)
* Map function for arrays (generates a for-loop to fill with data)
* Threading/HPC


### Standard Library

* Create core object with "_start"
* Create corelib with "out", "syscall", "malloc", and "free"
* Corelib with file IO
* Corelib with terminal IO

