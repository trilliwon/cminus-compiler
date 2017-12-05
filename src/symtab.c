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
 * Scope List
 */
static Scope scopeList[SIZE];
static int numOfScope = 0;
static int location[SIZE];

Scope newScope(char * scopeName) {
  Scope newScope;
  newScope = (Scope) malloc(sizeof(struct ScopeListRec));
  newScope->name = scopeName;
  newScope->nestedLevel = numOfScope;
  newScope->parent = topInScopeList();
  scopeList[numOfScope++] = newScope;
  return newScope;
}

Scope topInScopeList(void) {
  return scopeList[numOfScope - 1];
}

void popScope(void) {
  numOfScope--;
}

void pushScope(Scope scope) {
  scopeList[numOfScope] = scope;
  location[numOfScope++] = 0;
}

int addLocation(void) {
  return location[numOfScope - 1]++;
}

/* Bucket Stack
BucketList st_bucket(char * name);
void st_insert(char * scope, char * name, ExpType type, int lineno, int loc);
int st_lookup (char * name)
*/
BucketList st_bucket(char * name) {
  int h = hash(name);
  Scope sc = topInScopeList();

  while(sc) {
    BucketList l = sc->hashTable[h];
    while ((l != NULL) && (strcmp(name,l->name) != 0)) l = l->next;
    if (l != NULL) return l;
    sc = sc->parent;
  }
  return NULL;
}

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert(char * scope, char * name, ExpType type, int lineno, int loc, TreeNode * treeNode) {
  int h = hash(name);
  Scope top = topInScopeList();
  BucketList l = top->hashTable[h];

  /* Move to last node finding the same name until last BucketList */
  while ((l != NULL) && (strcmp(name, l->name) != 0)) l = l->next;

  /* variable not yet in BucketList */
  if (l == NULL) {
    l = (BucketList) malloc(sizeof(struct BucketListRec));
    l->name = name;
    l->type = type;
    l->treeNode = treeNode;
    l->lines = (LineList) malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->memloc = loc;
    l->lines->next = NULL;
    l->next = top->hashTable[h];
    top->hashTable[h] = l;
  } else {
    /* already exist in the BucketList */
    LineList t = l->lines;
    while (t->next != NULL) t = t->next;
    t->next = (LineList) malloc(sizeof(struct LineListRec));
    t->next->lineno = lineno;
    t->next->next = NULL;
  }
} /* st_insert */

/* Function st_lookup returns the memory
 * location of a variable or -1 if not found
 */
int st_lookup (char * name) {
  BucketList l = st_bucket(name);
  if (l != NULL) return l->memloc;
  return -1;
}

int st_lookup_top (char * name) {
  int h = hash(name);
  Scope sc = topInScopeList();

  while(sc) {
    BucketList l = sc->hashTable[h];
    while ((l != NULL) && (strcmp(name,l->name) != 0)) l = l->next;
    if (l != NULL) return l->memloc;
    break;
  }
  return -1;
}

int st_add_lineno(char * name, int lineno) {
  BucketList l = st_bucket(name);
  LineList ll = l->lines;

  while (ll->next != NULL) ll = ll->next;
  ll->next = (LineList) malloc(sizeof(struct LineListRec));
  ll->next->lineno = lineno;
  ll->next->next = NULL;
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
            fprintf(listing, "Void     ");
            break;
          case Integer:
            fprintf(listing, "Integer  ");
            break;
          default:
            break;
        }

        //TODO Location, Scope, Line Numbers

        while (t != NULL) {
          fprintf(listing,"%4d ", t->lineno);
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

  fprintf(listing, "\n------------------\n");
  fprintf(listing, "|  Symbol table  |");
  fprintf(listing, "\n------------------\n\n");

  for (int i = 0; i<numOfScope; ++i) {

    Scope scope = scopeList[i];
    BucketList * hashTable = scope->hashTable;
    fprintf(listing, "--------------------------------------------------------------------------\n");
    fprintf(listing, "Name       Type             Data Type   Location   Scope   Line Numbers   \n");
    fprintf(listing, "---------  ---------------  ----------  ---------  ------  ---------------\n");
    printSymbolTableRows(hashTable, listing);
    fprintf(listing, "--------------------------------------------------------------------------\n");

    fputc('\n', listing);
  }
} /* printSymTab */
