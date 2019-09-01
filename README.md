# Hobbes
Hobbes is a work in progress compiler for the Tiger language (from [Appel's
_Modern Compiler Implementation in C_][tiger]). Track the progess of Hobbes
with [this issue][tracker].

## Usage

Hobbes requires [Bison and Flex][bflex]. I recommend installing them with
[Homebrew][brew]. Once you've done that, you can try generating and
typechecking the AST of a simple program:

	$ cd src
	$ make
	$ ./hobbes ../testcases/test1.tig

## Links
- [Project Starter Files][sfiles]
- [Language spec from Tufts][spec]

[sfiles]: https://www.cs.princeton.edu/~appel/modern/c/project.html
[bflex]: http://dinosaur.compilertools.net/
[tracker]: https://github.com/tedbauer/hobbes/issues/1
[brew]: https://brew.sh/
[tiger]: https://www.cs.princeton.edu/~appel/modern/
[spec]: http://www.cs.tufts.edu/comp/181/Tiger.pdf
