Hack Assembler from Nand2Tetris
===============================

A C implementation of the Hack Assembler, adhering to the specified API outlined
 in Project 6 of the [NandToTetris](https://www.nand2tetris.org/) course.


Development Approach
--------------------

The code is extensively commented.  The focus is on explaining the design 
choices serving to adapt to the API requirements.

The application incorporates some rudimentary error handling.  I acknowledge
there's lots of room for improvement in this aspect. 


### Building

Unit testing leveraged by the [minunit](https://github.com/siu/minunit)
framework.  The script `tests/compare.sh` uses the `diff` command to compare the
solution's output Hack files against correctly compiled binaries.  A makefile
streamlines the development process:

+ `make tests`: Run and build unit tests.
+ `make compare`: Run the comparison script.
+ `make install`: Compiles and install the binary into `~/.local/bin`
+ `make uninstall`: Removes the compiled binary from `~/.local/bin`

### Usage

```sh
$ hackassembler <input.asm>
```


A Note on Compatibility
-----------------------

Some modules rely on the POSIX-specific functions `strdup()` and `getline()`. 
Consequently, building on non-POSIX systems, such as Windows, may require 
porting the code by defining these functions.

