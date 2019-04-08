# Tigger
Tigger is a work in progress compiler for the Tiger language. Track its
progress with [this issue][tracker].

## Usage

Tigger requires [Bison and Flex][bflex]. I recommend installing them with
[Homebrew][brew]. Once you've done that, you can try generating an AST from
a simple program:

	$ cd src
	$ make
	$ ./tigger ../testcases/test1.tig

## Links
- [Project Starter Files][sfiles]

[sfiles]: https://www.cs.princeton.edu/~appel/modern/c/project.html
[bflex]: http://dinosaur.compilertools.net/
[tracker]: https://github.com/tedbauer/tigger/issues/1
[brew]: https://brew.sh/
