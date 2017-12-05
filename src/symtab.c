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
static int hash ( char * key ) {
  int temp = 0;
  int i = 0;
  while (key[i] != '\0') {
    temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}

static Scope scopeList[SIZE];
static int nScope = 0;

static int location[SIZE];

/* Scope stack declaration
sc_top();
sc_pop();
sc_push();
*/
static Scope scopeStack[SIZE];
static int nScopeStack = 0;

Scope sc_top(void) {
  return scopeStack[nScopeStack - 1];
}

// TODO
void sc_pop(void) {
  nScopeStack--;
}

void sc_push( Scope scope ) {
  scopeStack[nScopeStack] = scope;
  location[nScopeStack++] = 0;
}

int addLocation(void) {
  return location[nScopeStack - 1]++;
}

Scope sc_create(char * scope) {
  Scope newScope;
  newScope = (Scope) malloc(sizeof(struct ScopeListRec));
  newScope->name = scope;
  newScope->nestedLevel = nScopeStack;
  newScope->parent = sc_top();
  scopeList[nScope++] = newScope;
  return newScope;
}

/* Bucket Stack
BucketList st_bucket(char * name);
void st_insert(char * name, int lineno, int loc, TreeNode * treeNode);
int st_lookup (char * name)
*/
BucketList st_bucket(char * name) {
  int h = hash(name);
  Scope sc = sc_top();

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
void st_insert(char * name, int lineno, int loc, TreeNode * treeNode) {
  int h = hash(name);
  Scope top = sc_top();
  BucketList l =  top->hashTable[h];

  while ((l != NULL) && (strcmp(name, l->name) != 0)) l = l->next;

  /* variable not yet in table */
  if (l == NULL) {
    l = (BucketList) malloc(sizeof(struct BucketListRec));
    l->name = name;
    l->treeNode = treeNode;
    l->lines = (LineList) malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->memloc = loc;
    l->lines->next = NULL;
    l->next = top->hashTable[h];
    top->hashTable[h] = l; }
  else {
    // ERROR!
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
  Scope sc = sc_top();

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

  for (int i = 0; i<nScope; ++i) {

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
