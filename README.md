# Tigger
Tigger is a work in progress compiler for the Tiger language.

## Usage

Tigger requires [Lex and Yacc][lexyacc].

There are a few helpful commands:
- `make lextest`: build executable `lextest`, which you can run with
  `./lextest example.tiger` to see the tokens that get lexed.
- `make parsetest`: build executable `parsetest`. You can either run
  `./parsetest example.tiger` to check if `example.tiger` was parsed
  correctly, or you can run `./parsetest` and it will check every file
  in `testcases/`.

## Links
- [Project Starter Files][sfiles]

[sfiles]: https://www.cs.princeton.edu/~appel/modern/c/project.html
[lexyacc]: http://dinosaur.compilertools.net/#yacc
