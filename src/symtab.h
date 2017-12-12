/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for the TINY compiler     */
/* (allows only one symbol table)                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "globals.h"

/* SIZE is the size of the hash table */
#define SIZE 211

/* the list of line numbers of the source
* code in which a variable is referenced
*/
typedef struct LineListRec {
  int lineno;
  struct LineListRec * next;
} * LineList;

/* The record in the bucket lists for
* each variable, including name,
* assigned memory location, and
* the list of line numbers in which
* it appears in the source code
*/
typedef struct BucketListRec {
  char * name;
  TreeNode * treeNode;
  LineList lines;
  int memloc; /* memory location for variable */
  struct BucketListRec * next;
  ExpType type;
} * BucketList;

/* Scope List */
typedef struct ScopeListRec {
  char * name; // function name
  int nestedLevel;
  struct ScopeListRec *parent;
  BucketList hashTable[SIZE]; /* the hash table */
} * Scope;

// global scope to cover function definitions
Scope globalScope;

/* Scope List to output */
static Scope scopeList[SIZE];
static int sizeOfList = 0;

/* Stack to deal with scope */
static Scope scopeStack[SIZE];
static int topScope = 0;

Scope newScope(char * scopeName);
void popScope(void);
void pushScope(Scope scope);
void insertScopeToList(Scope scope);

/* Procedure st_insert inserts line numbers and
* memory locations into the symbol table
* loc = memory location is inserted only the
* first time, otherwise ignored
*/
void st_insert(char * scopeName, char * name, ExpType type, TreeNode * treeNode, int loc);

// return current scope
Scope currScope();
/*
 * Function st_lookup returns the BucketList or NULL if not found
 */
BucketList st_lookup(char * name);

Scope st_lookup_scope(char * scopeName);
BucketList st_lookup_all_scope(char * name);
void insertLines(char* name, int lineno);

/* Procedure printSymTab prints a formatted
 * listing of the symbol table contents
 * to the listing file
 */
void printSymTab(FILE * listing);

#endif
