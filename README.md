# cminus-compiler

1. Scanner
2. Parser
  - just using yacc
3. Semantic analyzer


# Execution

```
$ make
$ ./cminus gcd.cm
```

# TODO Semantic Analysis

- [x] main.c
  - Modify NO_ANALYZE, TraceParse, and TraceAnalyze to suit your assignment
- [ ] symtab.h symtab.c
  - Add scope and type to symbol table
  - Implement hash table
- [ ] analyze.c
  - Modify symbol table generation
    - [ ] buildSymtab(),
    - [ ] insertNode(),
    - [ ] traverse(),
    - [ ] ... , scope and type concept
  - Modify the checkNode() function to check the semantics of C-Minus
  - [x] Insert built-in function
    - Input(), output()



### [Original Code](http://www.cs.sjsu.edu/~louden/cmptext/)
