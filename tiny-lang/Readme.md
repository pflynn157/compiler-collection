## Riya Language

Welcome to Riya! The Riya language is a subset of the Orka language (with some changes). The purpose is to provide a more barebones frontend that can be used for forks, experiments, and teaching. So its so simplified, Riya also works as the reference implementation for any language that uses the overall Laado compiler collection. Besides having a simplified syntax, it also removes some of the semantics found in Orka.

### The Language

The language is very simple. It contains only these structures:
* Functions
* i8, i16, i32, i64 (signed and unsigned), char, string, and bool data types
* Constants
* Arrays (all dynamically allocated on the heap)
* Structures
* If/elif/else conditional statements
* While loops
* Built-in string support
* Defined yet minimal standard library

And that's all!

Here's a sample of the language:

```
import std.io;

struct S1 is
    x : i32 := 10;
    y : i32 := 20;
end

func main -> i32 is
    struct s : S1;

    var v1 : i32 := s.x;
    
    printf("X: %d\n", v1);
    printf("Y: %d\n", s.y);
    
    s.x := 25;
    printf("Changed X: %d\n", s.x);
    
    return 0;
end
```

### The Compiler

The compiler is the main implementation of the language. It uses the overall Laado library with LLVM to compile and run.


### The Interpreter

There is also a version of Riya that uses the Laado source-level AST interpreter as the backend, making Riya essentially an interpreted language as well as a compiled language. However, the two are not completely interchangable. The interpreted version has a few semantics and built-in functions that don't exist in the compiled language. This partly by design- this version of Riya is primarily meant for interpreter testing.

