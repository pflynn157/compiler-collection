## Tiny Lang

Welcome to Tiny Lang! Tiny Lang is a fork of my Orka compiler/programming language project. Unlike my other forks, however, this is not meant to be some new and improved version. This is a simplified version that adheres to a spec that I wrote for a very simple programming language. In other words, it's an example implementation.

This project was born out of another one. I started a project to test implementing a compiler in another programming language, and I got stuck not really knowing what I was implementing. Most of my previous languages are broken in some way (to date, Orka is my most complete and functional language). However, Orka and many of the other languages for that matter are too big for quick implementations. I thought about C, but C is a real language designed for serious, real-world work, and it has its share of quirks I didn't want to deal with. Finally, I'm lazy, and I didn't want to have to create some example implementation from scratch.

The solution: Create a downsided version of Orka that contains the most essential programming elements. This downsized language is meant to be usable for most real-world tasks, but simple enough to where a compiler or interpreter can be implemented quickly.

### Examples

The examples directory contains a few simple yet practical examples of the language in action:
* Command line arguments
* cat (clone of UNIX cat)
* echo (clone of UNIX echo)
* fs_test (example of reading/writing files)
* Math Practice
* Tic-Tac-Toe

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

The compiler is completely written in C++ and uses only the standard library and LLVM. The lexer, parser, and AST (the frontend) is written from scratch. The backend uses LLVM for simplicity.


### Dependencies

In order to build, you need a C++ compiler, LLVM (most versions should), and minilex.

Minilex is a recent addition to this project. It's my own project, so you can find here on my Github profile. Minilex is a simple lexical analyzer generator. All that's needed is a config file (see src/lex), and a compatible lexical analyzer is generated in the build directory. The source for minilex was originally based on the Tiny Lang lexical analyzer, so needless to say, it works perfectly! :)

For the curious, adding minilex was to make it even easier to fork.

### License

This program is licensed under the BSD-3 License.

