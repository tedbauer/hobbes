# Tigger
Tigger is a work in progress compiler for the Tiger language (from [Appel's
_Modern Compiler Implementation in C_][tiger]). Track the progess of Tigger
with [this issue][tracker].

## Usage

Tigger requires [Bison and Flex][bflex]. I recommend installing them with
[Homebrew][brew]. Once you've done that, you can try generating and
typechecking the AST of a simple program:

	$ cd src
	$ make
	$ ./tigger ../testcases/test1.tig

## Links
- [Project Starter Files][sfiles]

[sfiles]: https://www.cs.princeton.edu/~appel/modern/c/project.html
[bflex]: http://dinosaur.compilertools.net/
[tracker]: https://github.com/tedbauer/tigger/issues/1
[brew]: https://brew.sh/
[tiger]: https://www.cs.princeton.edu/~appel/modern/
