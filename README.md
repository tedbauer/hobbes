# Tigger
Implementation and exercise solutions for Appel's Modern Compiler Implementation in C.

## Usage

Tigger requires [Lex and Yacc][lexyacc].

There are a few helpful commands:
- `make lextest`: build executable `lextest`, which you can run with
  `./lextest example.tiger` to see the tokens that get lexed.
- `make parsetest`: build executable `parsetest`. You can either run
  `./parsetest example.tiger` to check if `example.tiger` was parsed
  correctly, or you can run `./parsetest` and it will check every file
  in `testcases/`.

## Status

### Compiler implementation
- [X] Lexical Analysis
    + [ ] Add support for `\^c`, `\ddd`, and `\f___f\`
- [X] Parsing
- [ ] Abstract Syntax
- [ ] Semantic Analysis
- [ ] Activation Records
- [ ] Translation to IR
- [ ] Basic Blocks and Traces
- [ ] Instruction Selection
- [ ] Liveness Analysis
- [ ] Register Allocation
- [ ] Garbage Collection
- [ ] Object-Oriented Stuff
- [ ] Functional Stuff
- [ ] Polymorphic Types
- [ ] Dataflow Analysis
- [ ] Loop Optimizations
- [ ] Static Single-Assignment Form
- [ ] Pipelining and Scheduling
- [ ] Memory Hierarchy Stuff

### Exercises
Todo

## Links
- [Project Starter Files][sfiles]

[sfiles]: https://www.cs.princeton.edu/~appel/modern/c/project.html
[lexyacc]: http://dinosaur.compilertools.net/#yacc
