/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4

/* the hash function */
static int hash(char * key) {
  int temp = 0;
  int i = 0;
  while (key[i] != '\0') {
    temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}

/*
 * Create new Scope
 *
 *
 */
Scope newScope(char * scopeName) {
  Scope newScope = (Scope) malloc(sizeof(struct ScopeListRec));
  newScope->name = scopeName;
  return newScope;
}

void popScope(void) {
  // TODO free memory
  topScope--;
}

void pushScope(Scope scope) {
  topScope++;
  scopeList[topScope] = scope;
}

Scope currScope() {
  return scopeList[topScope];
}

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( char * scopeName,
                char * name,
                TreeNode * treeNode,
                int lineno,
                int loc ) {

  int h = hash(name);


  BucketList l;
  Scope currScope;
  for (int i=0; i<=topScope; i++) {
    currScope = scopeList[topScope];
    l = currScope->hashTable[h];;

    /** try to find bucket */
    while ((l != NULL) && (strcmp(name, l->name) != 0)) l = l->next;
  }

  /* variable not yet in BucketList */
  if (l == NULL) {
    l = (BucketList) malloc(sizeof(struct BucketListRec));
    l->name = name;
    l->treeNode = treeNode;
    l->lines = (LineList) malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->memloc = loc;
    l->lines->next = NULL;
    l->next = currScope->hashTable[h];
    currScope->hashTable[h] = l;
  } else {
    /* already exist in the BucketList */
    LineList t = l->lines;
    while (t->next != NULL) t = t->next;
    t->next = (LineList) malloc(sizeof(struct LineListRec));
    t->next->lineno = lineno;
    t->next->next = NULL;
  }
} /* st_insert */

/* Function st_lookup returns Bucket
 * location of a variable or NULL if not found
 */
BucketList st_lookup(char * scopeName, char * name) {

  Scope scope = NULL;

  for (int i=0; i<=topScope; i++) {
    if (strcmp(scopeList[i]->name, scopeName) == 0) {
      scope = scopeList[i];
      break;
    }
  }

  if (scope == NULL) return NULL;

  int h = hash(name);
  BucketList bucket = scope->hashTable[h];

  while ((bucket != NULL) && (strcmp(name, bucket->name) != 0)) bucket = bucket->next;
  return bucket;
}

void printSymbolTableRows(BucketList *hashTable, FILE *listing) {

  for (int i=0; i<SIZE; ++i) {

    if (hashTable[i] != NULL) {
      BucketList l = hashTable[i];
      TreeNode *node = l->treeNode;

      while (l != NULL) {

        LineList t = l->lines;
        fprintf(listing,"%-10s ", l->name);

        switch (node->nodekind) {
          case DeclK:
            switch (node->kind.decl) {
              case FunK:
                fprintf(listing, "Function         ");
                break;
              case VarK:
                fprintf(listing, "Variable         ");
                break;
              case ArrVarK:
                fprintf(listing, "Array Variable   ");
                break;
              case ParamK:
                fprintf(listing, "Parameter        ");
                break;
              case ArrParamK:
                fprintf(listing, "Array Parameter  ");
                break;
              default:
                break;
              }
            break;
        default:
          break;
        }

        switch (node->type) {
          case Void:
            fprintf(listing, "Void        ");
            break;
          case Integer:
            fprintf(listing, "Integer     ");
            break;
          default:
            break;
        }

        // print memory location
        fprintf(listing, "%d", l->memloc);

        // print line numbers
        while (t != NULL) {
          fprintf(listing, "%11d ", t->lineno);
          t = t->next;
        }

        fprintf(listing, "\n");
        l = l->next;
      }
    }
  }
}

/* Procedure printSymTab prints a formatted
 * listing of the symbol table contents
 * to the listing file
 */
void printSymTab(FILE * listing) {
  fprintf(listing, "printSymTab\n");

  fprintf(listing, "\n------------------\n");
  fprintf(listing, "|  Symbol table  |");
  fprintf(listing, "\n------------------\n\n");

  for (int i = 0; i<topScope; ++i) {

    Scope scope = scopeList[i];
    BucketList * hashTable = scope->hashTable;
    fprintf(listing, "Scope : %s\n", scope->name);
    fprintf(listing, "------------------------------------------------------------------\n");
    fprintf(listing, "Name       Type             Data Type   Location   Line Numbers   \n");
    fprintf(listing, "---------  ---------------  ----------  ---------  ---------------\n");
    printSymbolTableRows(hashTable, listing);
    fprintf(listing, "------------------------------------------------------------------\n");

    fputc('\n', listing);
  }
} /* printSymTab */

/* Bucket Stack
 * BucketList st_bucket(char * name);
 * void st_insert(char * scope, char * name, ExpType type, int lineno, int loc);
 * int st_lookup (char * name)
 */
BucketList st_bucket(char * name) {
  int h = hash(name);
  Scope currScope = scopeList[topScope];

  while(currScope) {
    BucketList l;
    while ((l != NULL) && (strcmp(name,l->name) != 0)) l = l->next;
    if (l != NULL) return l;
    currScope = currScope->parent;
  }
  return NULL;
}
