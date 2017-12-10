# cminus-compiler

1. Scanner
  - Implementation Lexer
  - Using Lexer
2. Parser
  - just using yacc
3. Semantic analyzer
  - Implementation of Symbol Table
  - Implementation of Type Checker


# TODO Semantic Analysis
>
- main.c
  - [DONE] Modify NO_ANALYZE, TraceParse, and TraceAnalyze to suit your assignment
- `symtab.h` `symtab.c`
  - [DONE] Add scope and type to symbol table
  - [DONE] Implement hash table
- analyze.c
  - [DONE] Insert built-in function
    - Input(), output()
  - [DOING] Modify symbol table generation
    - [DONE]  `buildSymtab()`
    - [DOING] `insertNode()`
    - [TODO]  `traverse()`
    - [TODO]  scope and type concept
  - [TODO] Modify the checkNode() function to check the semantics of C-Minus

### [Original Code](http://www.cs.sjsu.edu/~louden/cmptext/)


# Execution

```
$ make
$ ./cminus gcd.cm
```
