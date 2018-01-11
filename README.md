# cminus-compiler

1. Scanner
    - Implementation Lexer
    - Using Lexer
2. Parser
    - just using yacc
3. Semantic analyzer
    - Implementation of Symbol Table
    - Implementation of Type Checker


# Semantic Analysis

- main.c
  - [x] Modify NO_ANALYZE, TraceParse, and TraceAnalyze to suit your assignment
- `symtab.h` `symtab.c`
  - [x] Add scope and type to symbol table
  - [x] Implement hash table
- analyze.c
  - [x] Insert built-in function
    - Input(), output()
  - [x] Modify symbol table generation
  - [ ] Modify the checkNode() function to check the semantics of C-Minus

### [Original Code](http://www.cs.sjsu.edu/~louden/cmptext/)


# Execution

```
$ make
$ ./compiler gcd.cm
```
