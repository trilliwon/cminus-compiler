/****************************************************/
/* File: cminus.y                                   */
/* The C-Minus Yacc/Bison specification file        */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

/**************************/
/* C declarations         */
/**************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode *
static int savedNumber;
static char * savedName; /* for use in assignments */
static int savedLineNo;  /* ditto */
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yylex(void); // added 11/2/11 to ensure no conflict with lex

%}

/**************************/
/* Bison declarations     */
/**************************/

%token IF ELSE INT RETURN VOID WHILE
%token ID NUM
%token LT LE GT GE EQ NE SEMI
%token LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET
%left PLUS MINUS
%left TIMES OVER COMMA
%right ASSIGN
%token ERROR

/*

typedef enum { StmtK, ExpK, DeclK } NodeKind;
typedef enum { IfK, WhileK, AssignK, CompoundK, ReturnK } StmtKind;
typedef enum { OpK, ConstK, IdK, TypeK, ArrIdK, CallK, CalcK } ExpKind;
typedef enum { VarK, FunK, ArrVarK, ParamK, ArrParamK } DeclKind;

*/

%%

/**************************/
/* Grammar rules          */
/**************************/

program : declaration_list { savedTree = $1; }
        ;

declaration_list : declaration_list declaration
                   { YYSTYPE t = $1;
                     if (t != NULL) {
                       while (t->sibling != NULL) { t = t->sibling; }
                       t->sibling = $2;
                       $$ = $1;
                     } else {
                       $$ = $2;
                     }
                   }
                 | declaration { $$ = $1; }
                 ;

declaration : var_declaration { $$ = $1; }
            | fun_declaration { $$ = $1; }
            ;

id : ID
     { savedName = copyString(tokenString);
       savedLineNo = lineno;
     }
   ;

num : NUM
      { savedNumber = atoi(tokenString);
        savedLineNo = lineno;
      }
    ;

var_declaration : type_specifier id SEMI
                  { $$ = newDeclNode(VarK);
                    $$->child[0] = $1;
                    $$->lineno = lineno;
                    $$->attr.name = savedName;
                  }
                | type_specifier id LBRACKET num RBRACKET SEMI
                  { $$ = newDeclNode(ArrVarK);
                    $$->child[0] = $1;
                    $$->lineno = lineno;
                    $$->type = IntegerArray;
                    $$->attr.arr.name = savedName;
                    $$->attr.arr.size = savedNumber;
                  }
                ;

type_specifier : INT
                 { $$ = newExpNode(TypeK);
                   $$->type = Integer;
                 }
               | VOID
                 { $$ = newExpNode(TypeK);
                   $$->type = Void;
                 }
               ;

fun_declaration : type_specifier id {
                   $$ = newDeclNode(FunK);
                   $$->lineno = lineno;
                   $$->attr.name = savedName;
                 }
                 LPAREN params RPAREN compound_stmt
                 { $$ = $3;
                   $$->child[0] = $1;
                   $$->child[1] = $5;
                   $$->child[2] = $7;
                 }
                ;

params : param_list { $$ = $1; }
       | VOID
         { $$ = newDeclNode(ParamK);
           $$->type = Void;
         }
       ;

param_list : param_list COMMA param
             { YYSTYPE t = $1;
               if (t != NULL) {
                 while (t->sibling != NULL) { t = t->sibling; }
                 t->sibling = $3;
                 $$ = $1;
               } else {
                 $$ = $2;
               }
             }
           | param { $$ = $1; }
           ;

param : type_specifier id
        { $$ = newDeclNode(ParamK);
          $$->child[0] = $1;
          $$->attr.name = savedName;
        }
      | type_specifier id LBRACKET RBRACKET
        { $$ = newDeclNode(ArrParamK);
          $$->child[0] = $1;
          $$->attr.name = copyString(savedName);
        }
      ;

compound_stmt : LBRACE local_declarations statement_list RBRACE
                { $$ = newStmtNode(CompoundK);
                  $$->child[0] = $2;
                  $$->child[1] = $3;
                }
              ;

local_declarations : local_declarations var_declaration
                     { YYSTYPE t = $1;
                       if (t != NULL) {
                         while (t->sibling != NULL) { t = t->sibling; }
                         t->sibling = $2;
                         $$ = $1;
                       } else {
                         $$ = $2;
                       }
                     }
                   | { $$ = NULL; }
                   ;

statement_list : statement_list statement
                 { YYSTYPE t = $1;
                   if (t != NULL) {
                     while (t->sibling != NULL) { t = t->sibling; }
                    t->sibling = $2;
                    $$ = $1;
                   } else {
                     $$ = $2;
                   }
                 }
               | { $$ = NULL; }
               ;

statement : expression_stmt { $$ = $1; }
          | compound_stmt { $$ = $1; }
          | selection_stmt { $$ = $1; }
          | iteration_stmt { $$ = $1; }
          | return_stmt { $$ = $1; }
          ;

expression_stmt : expression SEMI { $$ = $1; }
                | SEMI { $$ = NULL; }
                ;

selection_stmt : IF LPAREN expression RPAREN statement
                 { $$ = newStmtNode(IfK);
                   $$->child[0] = $3;
                   $$->child[1] = $5;
                 }
               | IF LPAREN expression RPAREN statement ELSE statement
                 { $$ = newStmtNode(IfK);
                   $$->child[0] = $3;
                   $$->child[1] = $5;
                   $$->child[2] = $7;
                 }
               ;

iteration_stmt : WHILE LPAREN expression RPAREN statement
                 { $$ = newStmtNode(WhileK);
                   $$->child[0] = $3;
                   $$->child[1] = $5;
                 }
               ;

return_stmt : RETURN SEMI
              { $$ = newStmtNode(ReturnK);
                $$->type = Void;
              }
            | RETURN expression SEMI
              { $$ = newStmtNode(ReturnK);
                $$->child[0] = $2;
              }
            ;

expression : var ASSIGN expression
             { $$ = newStmtNode(AssignK);
               $$->child[0] = $1;
               $$->child[1] = $3;
             }
           | simple_expression { $$ = $1; }
           ;

var : id
      { $$ = newExpNode(IdK);
        $$->attr.name = savedName;
      }
    | id {
        $$ = newExpNode(ArrIdK);
        $$->attr.name = savedName;
      } LBRACKET expression RBRACKET
      {
        $$ = $2;
        $$->child[0] = $4;
      }
    ;

simple_expression : additive_expression relop additive_expression
                    { $$ = newExpNode(CalcK);
                      $$->child[0] = $1;
                      $$->child[1] = $2;
                      $$->child[2] = $3;
                    }
                  | additive_expression { $$ = $1; }
                  ;

relop : LT
        { $$ = newExpNode(OpK);
          $$->attr.op = LT;
        }
      | LE
        { $$ = newExpNode(OpK);
          $$->attr.op = LE;
        }
      | GT
        { $$ = newExpNode(OpK);
          $$->attr.op = GT;
        }
      | GE
        { $$ = newExpNode(OpK);
          $$->attr.op = GE;
        }
      | EQ
        { $$ = newExpNode(OpK);
          $$->attr.op = EQ;
        }
      | NE
        { $$ = newExpNode(OpK);
          $$->attr.op = NE;
        }
      ;

additive_expression : additive_expression addop term
                      { $$ = newExpNode(CalcK);
                        $$->child[0] = $1;
                        $$->child[1] = $2;
                        $$->child[2] = $3;
                      }
                    | term { $$ = $1; }
                    ;

addop : PLUS
        { $$ = newExpNode(OpK);
          $$->attr.op = PLUS;
        }
      | MINUS
        { $$ = newExpNode(OpK);
          $$->attr.op = MINUS;
        }
      ;

term : term mulop factor
       { $$ = newExpNode(CalcK);
         $$->child[0] = $1;
         $$->child[1] = $2;
         $$->child[2] = $3;
       }
     | factor { $$ = $1; }
     ;

mulop : TIMES
        { $$ = newExpNode(OpK);
          $$->attr.op = TIMES;
        }
      | OVER
        { $$ = newExpNode(OpK);
          $$->attr.op = OVER;
        }
      ;

factor : LPAREN expression RPAREN { $$ = $2; }
       | var { $$ = $1; }
       | call { $$ = $1; }
       | num
         { $$ = newExpNode(ConstK);
           $$->type = Integer;
           $$->attr.val = atoi(tokenString);
         };
       ;

call : id {
         $$ = newExpNode(CallK);
         $$->attr.name = savedName;
         } LPAREN args RPAREN {
           $$ = $2;
           $$->child[0] = $4;
       }
     ;

args : arg_list { $$ = $1; }
     | { $$ = NULL; }
     ;

arg_list : arg_list COMMA expression
           { YYSTYPE t = $1;
             if (t != NULL) {
               while (t->sibling != NULL) { t = t->sibling; }
               t->sibling = $3;
               $$ = $1;
             } else {
               $$ = $3;
             }
           }
         | expression { $$ = $1; }
         ;

%%

/**************************/
/* Additional C code      */
/**************************/

int yyerror(char * message) {
  fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  printToken(yychar,tokenString);
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the TINY scanner
 */
static int yylex(void) {
  return getToken();
}

TreeNode * parse(void) {
  yyparse();
  return savedTree;
}
