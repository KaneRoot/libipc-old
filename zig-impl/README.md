# Why rewrite in Zig

I did a library called `libipc` then I wanted to rewrite it in Zig.

The problems with the C-based version:

- libipc cannot easily be ported on other plateforms
  * cannot compile under musl
  * cannot compile for any OS
  * different libc = different error codes for functions
- cannot change the code easily without risking to break stuff
- behavior isn't the same accross OSs
  * glibc-based and musl-based Linux have different behavior
  * Linux and OpenBSD have different behavior
- hard to debug
  * OpenBSD has a buggy valgrind

For all these reasons, this library took a lot of time to dev (~ 2 months), and I'm still not entirely confident in its reliability.
Tests are required to gain confidence.
The mere compilation isn't a proof of anything.
And the code size (2kloc) for something that simple can be seen as a mess, despite being rather fair.

What Zig brings to the table:

- OS and libc-agnostic, all quirks are handled in the std lib by the Zig dev team
- same source code for all OSs
- can compile **from and to** any OS and libc
- errors already are verified in the std lib
- compiling `foo() catch |e| switch(e) {}` shows all possible errors
- std lib provides
  * basic structures and functions, no need to add deps or write functions to have simple lists and such
  * memory allocators & checks, no need for valgrind and other memory checking tools
  * basic network structures (address & stuff)
- structures can be printed
- code and tests in the same file
- (in a near future) async functions, to handle networking operations in a very elegant manner

All these features provide a **massive gain of time**.

And in general, just having the same standard library for all OSs is great just not to have to rely on different implementations and even locations of code and header files.
