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

/* counter for variable memory locations */
static char * funcName;

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
  st_insert("global", "input", fun_declaration, 0, 0);
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
  st_insert("global", "output", fun_declaration, 0, 0);
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
  if (t==NULL) return;
  else return;
}

static void symbolError(TreeNode * t, char * message) {
  fprintf(listing,"Symbol error at line %d: %s\n", t->lineno, message);
  Error = TRUE;
}

/* Procedure insertNode inserts
 * identifiers stored in t into
 * the symbol table
 * { StmtK, ExpK, DeclK }
 */
static void insertNode(TreeNode * t) {
  switch (t->nodekind) {
    case StmtK: {
      if (t->kind.stmt == CompoundK) {
        Scope scope = newScope(funcName);
        pushScope(scope);
        fprintf(listing, "insertNode() was called at %s: %d. info %s, line: %d\n", __FILE__, __LINE__, scope->name, t->lineno);
        t->attr.scope = currScope();
      }
      break;
    }
    case ExpK: {
      // TODO
      break;
    }
    case DeclK: {
      switch (t->kind.decl) {
        case FunK:
          {
            funcName = t->attr.name;
            Scope scope = newScope(funcName);
            fprintf(listing, "DeclK was called at %s: %d.  line: %d\n", __FILE__, __LINE__, t->lineno);
            switch (t->child[0]->attr.type) {
              case INT:
                t->type = Integer;
                break;
              case VOID:
              default:
                t->type = Void;
                break;
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
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t) {
  switch (t->nodekind) {
    case StmtK:
      switch (t->kind.stmt) {
        case AssignK:
         // if (t->child[0]->type == IntegerArray)
         //  /* no value can be assigned to array variable */
         //  typeError(t->child[0],"assignment to array variable");
         // else if (t->child[1]->type == Void)
         //   /* r-value cannot have void type */
         //   typeError(t->child[0],"assignment of void value");
         // else t->type = t->child[0]->type;
         break;
        case CompoundK:
          popScope();
          break;
        case WhileK:
         if (t->child[0]->type == Void)
         /* while test should be void function call */
           typeError(t->child[0],"while test has void value");
         break;
        case ReturnK:
          {
            const TreeNode * funcDecl;
            const ExpType funcType = funcDecl->type;
            const TreeNode * expr = t->child[0];

            if (funcType == Void && (expr != NULL && expr->type != Void)) {
               typeError(t,"expected no return value");
               //ValueReturned = TRUE;
             } else if (funcType == Integer && (expr == NULL || expr->type == Void)) {
               typeError(t,"expected return value");
             }
           }
           break;
         default:
           break;
       }
       break;
     case ExpK:
       switch (t->kind.exp) {
         case OpK:
           {
             ExpType leftType, rightType;
             TokenType op;

             leftType = t->child[0]->type;
             rightType = t->child[1]->type;
             op = t->attr.op;

             /*if (leftType == Void || rightType == Void)
               typeError(t, "two operands should have non-void type");
             else if (leftType == IntegerArray && rightType == IntegerArray)
               typeError(t, "not both of operands can be array");
             else if (op == MINUS && leftType == Integer && rightType == IntegerArray)
               typeError(t, "invalid operands to binary expression");
             else if ((op == TIMES || op == OVER) && (leftType == IntegerArray || rightType == IntegerArray))
               typeError(t, "invalid operands to binary expression");
             else t->type = Integer;*/
           }
           break;
         case ConstK:
          t->type = Integer;
          break;
         case IdK:
          break;
         case ArrIdK:
           {
             const char *symbolName = t->attr.name;
             const BucketList bucket;
             TreeNode *symbolDecl = NULL;

             if (bucket == NULL) break;

             if (t->kind.exp == ArrIdK) {
               if (symbolDecl->kind.decl != ArrVarK && symbolDecl->kind.decl != ArrParamK) {
                 typeError(t, "expected array symbol");
               }
               else if (t->child[0]->type != Integer) {
                 typeError(t,"index expression should have integer type");
               }
               else t->type = Integer;
             } else {
               t->type = symbolDecl->type;
             }
           }
           break;
         case CallK:
           {
             const char *callingFuncName = t->attr.name;
             const TreeNode * funcDecl;
             TreeNode *arg;
             TreeNode *param;

             if (funcDecl == NULL) break;

             arg = t->child[0];
             param = funcDecl->child[1];

             if (funcDecl->kind.decl != FunK) {
               typeError(t,"expected function symbol");
               break;
             }

             while (arg != NULL) {
               if (param == NULL) typeError(arg, "the number of parameters is wrong");
               /* the number of arguments does not match to that of parameters */
               /*else if (arg->type == IntegerArray &&
                   param->type != IntegerArray)
                 typeError(arg,"expected non-array value");
               else if (arg->type == Integer &&
                   param->type == IntegerArray)
                 typeError(arg,"expected array value");*/
               else if (arg->type == Void)
                 typeError(arg, "void value cannot be passed as an argument");
               else {  // no problem!
                 arg = arg->sibling;
                 param = param->sibling;
                 continue;
               }
               /* any problem */
               break;
             }

             if (arg == NULL && param != NULL)
             /* the number of arguments does not match to
                that of parameters */
               typeError(t->child[0],"the number of parameters is wrong");
               t->type = funcDecl->type;
           }
           break;
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
  // traverse(syntaxTree, nullProc, checkNode);
  // popScope();
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
  fprintf(listing, "buildSymtab() was called at %s: %d. info %s\n", __FILE__, __LINE__, globalScope->name);
  if (TraceAnalyze) {
    printSymTab(listing);
  }
}
