/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"
#include "util.h"

typedef int bool;
#define true 1
#define false 0

/*
 * To insert to symbol table build-in functions
 * int input()   // One integer value is input from the user.
 * void output() // Prints the value of arg.
 */
static void insertInputFunc(void) {
  TreeNode * fun_declaration = newDeclNode(FunK);
  fun_declaration->type = Integer;

  TreeNode * type_specifier = newExpNode(TypeK);
  type_specifier->attr.type = INT;

  TreeNode * compound_stmt = newStmtNode(CompoundK);
  compound_stmt->child[0] = NULL;
  compound_stmt->child[1] = NULL;

  fun_declaration->lineno = 0;
  fun_declaration->attr.name = "input";
  fun_declaration->child[0] = type_specifier;
  fun_declaration->child[1] = NULL;
  fun_declaration->child[2] = compound_stmt;

  /* Insert input function*/
  st_insert("global", "input", Integer, fun_declaration, 1);
}

/* To insert to symbol table build-in functions
 * int input()   // One integer value is input from the user.
 * void output() // Prints the value of arg.
 */
static void insertOutputFunc(void) {

  TreeNode * fun_declaration = newDeclNode(FunK);
  fun_declaration->type = Void;

  TreeNode * type_specifier = newDeclNode(FunK);
  type_specifier->attr.type = VOID;

  TreeNode * params = newDeclNode(ParamK);
  params->attr.name = "arg";
  params->child[0] = newExpNode(TypeK);
  params->child[0]->attr.type = INT;

  TreeNode * compound_stmt = newStmtNode(CompoundK);
  compound_stmt->child[0] = NULL;
  compound_stmt->child[1] = NULL;

  fun_declaration->lineno = 0;
  fun_declaration->attr.name = "output";
  fun_declaration->child[0] = type_specifier;
  fun_declaration->child[1] = params;
  fun_declaration->child[2] = compound_stmt;

  /* Insert output function*/
  st_insert("global", "output", Void, fun_declaration, 0);
}

/* Procedure traverse is a generic recursive
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc
 * in postorder to tree pointed to by t
 */
static void traverse(TreeNode * t, void (* preProc) (TreeNode *), void (* postProc) (TreeNode *)) {
  if (t != NULL) {
    preProc(t);
    for (int i=0; i < MAXCHILDREN; i++) {
      traverse(t->child[i], preProc, postProc);
    }
    postProc(t);
    traverse(t->sibling, preProc, postProc);
  }
}

/* nullProc is a do-nothing procedure to
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode * t) {

  if (t->nodekind == StmtK) {
    if (t->kind.stmt == CompoundK) {
      popScope();
    }
  }

  if (t==NULL) return;
  else return;
}

static void symbolError(TreeNode * t, char * message) {
  fprintf(listing,"Symbol Table error at line %d: %s\n", t->lineno, message);
  Error = TRUE;
  exit(-1);
}

// this is needed to check parameters
static bool isFirstCompoundK = false;
static int locationCounter = 0;

/* Procedure insertNode inserts
 * identifiers stored in t into
 * the symbol table
 */
static void insertNode(TreeNode * t) {
  switch (t->nodekind) {
    /*
     * Statement Key
     */
    case StmtK: {
      switch (t->kind.stmt) {
        case CompoundK: {
          if (!isFirstCompoundK) {
            Scope scope = newScope(currScope()->name);
            scope->parent = currScope();
            pushScope(scope);
          }
          isFirstCompoundK = false;
          break;
        }
        default:
          break;
      }
      break;
    }

    /*
    *
    * OpK, ConstK, IdK, TypeK, ArrIdK, CallK, CalcK } ExpKind;
    */
    case ExpK: {
      switch (t->kind.exp){
        case IdK:
        case ArrIdK:
        case CallK: {
          // check undeclation
          if (st_lookup_all_scope(t->attr.name) == NULL){
            fprintf(listing,"Symbol Table error  %s\n", t->attr.name);
            symbolError(t, "Undefined Symbol");
          } else {
            insertLines(t->attr.name, t->lineno);
          }
        }
        default:
          break;
      }
      break;
    }

    /*
     * Declaration Key
     */
    case DeclK: {
      switch (t->kind.decl) {
        case FunK: {
          // initialize location counter
          locationCounter = 0;

          /* Look up scope list to check scope existence */
          if (st_lookup_scope(t->attr.name) != NULL) {
            symbolError(t, "Redefinition of function");
            break;
          }

          if (strcmp(currScope()->name, "global") == 0) {
            st_insert(currScope()->name, t->attr.name, t->child[0]->type, t, locationCounter++);
          }

          Scope scope = newScope(t->attr.name);
          scope->parent = currScope();
          pushScope(scope);
          isFirstCompoundK = true;
          break;
        }

        case VarK: {

          /* Look up to check variable existence */
          if (st_lookup(t->attr.name) != NULL) {
            symbolError(t, "Redefinition of variable");
            break;
          }

          if (t->child[0]->type == Void) {
            symbolError(t, "Variable should not be void type.");
            break;
          }

          st_insert(currScope()->name, t->attr.name, t->child[0]->type, t, locationCounter++);
          break;
        }

        case ArrVarK: {

          if (t->child[0]->type == Void) {
            symbolError(t, "Redefinition of Array variable");
            break;
          }

          /*  Look up to check array variable existence  */
          if (st_lookup(t->attr.arr.name) != NULL) {
            symbolError(t, "Array Variable has already declared.");
            break;
          }

          st_insert(currScope()->name, t->attr.arr.name, t->child[0]->type, t, locationCounter++);
          break;
        }

        case ArrParamK: {

          if (t->child[0]->type == Void) {
            symbolError(t, "Array Parameter should not be void type.");
            break;
          }

          /*  Look up to check array parameter existence  */
          if (st_lookup(t->attr.name) != NULL) {
            symbolError(t, "Redefinition of Array Parameter");
            break;
          }

          st_insert(currScope()->name, t->attr.name, t->child[0]->type, t, locationCounter++);
          break;
        }

        case ParamK: {

          if (t->attr.name != NULL) {
            /*  Look up to check parameter existence  */
            if (st_lookup(t->attr.name) != NULL) {
              symbolError(t, "Redefinition of Parameter");
              break;
            }

            st_insert(currScope()->name, t->attr.name, t->child[0]->type, t, locationCounter++);
          }
          break;
        }
        default:
          break;
      }
      break;
    }
    default:
      break;
   }
 }

static void typeError(TreeNode * t, char * message) {
  fprintf(listing,"Type error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
  exit(-1);
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t) {
  switch (t->nodekind) {
    case StmtK:
      switch (t->kind.stmt) {
        case AssignK: {
          // Verify the type match of two operands when assigning.
          if (t->child[0]->attr.arr.type == IntegerArray) {
            typeError(t->child[0], "Assignment to Integer Array Variable");
          }

          if (t->child[0]->attr.type == Void) {
            typeError(t->child[0], "Assignment to Void Variable");
          }
          break;
        }
        case ReturnK: {
          const TreeNode * funcDecl;
          const ExpType funcType = funcDecl->type;
          const TreeNode * expr = t->child[0];

          if (funcType == Void && (expr != NULL && expr->type != Void)) {
             typeError(t,"expected no return value");
           } else if (funcType == Integer && (expr == NULL || expr->type == Void)) {
             typeError(t,"expected return value");
           }
        }
        default:
          break;
       }
       break;
     case ExpK:
       switch (t->kind.exp) {
         case OpK: {
           break;
         }
         case ConstK: {
           break;
         }
         case IdK: {
           break;
         }
         case ArrIdK: {
           break;
         }
         case CallK: {
           break;
         }
         default:
           break;
       }
       break;
     default:
       break;
   }
 }



/* Procedure typeCheck performs type checking
* by a postorder syntax tree traversal
*/
void typeCheck(TreeNode * syntaxTree) {
  traverse(syntaxTree, nullProc, checkNode);
}

/* Function buildSymtab constructs the symbol
* table by preorder traversal of the syntax tree
*/
void buildSymtab(TreeNode * syntaxTree) {

  // insert global scope
  globalScope = newScope("global");
  pushScope(globalScope);
  insertInputFunc();
  insertOutputFunc();
  traverse(syntaxTree, insertNode, nullProc);
  popScope();
  if (TraceAnalyze) {
    printSymTab(listing);
  }
}
