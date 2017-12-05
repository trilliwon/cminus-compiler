/****************************************************/
/* File: cminus.y                                   */
/* The C-Minus Yacc/Bison specification file        */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/
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

%token IF ELSE INT RETURN VOID WHILE
%token ID NUM
%token  LT LE GT GE EQ NE SEMI
%token LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET
%left PLUS MINUS
%left TIMES OVER COMMA
%right ASSIGN
%token ERROR

%%

/* Grammar for C-Minus */

program     : decl_list
                 { savedTree = $1;}
            ;
decl_list   : decl_list decl
                 { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1; }
                     else $$ = $2;
                 }
            | decl  { $$ = $1; }
            ;
decl        : var_decl  { $$ = $1; }
            | fun_decl  { $$ = $1; }
            ;
id          : ID
                 { savedName = copyString(tokenString);
                   savedLineNo = lineno;
                 }
            ;
num         : NUM
                 { savedNumber = atoi(tokenString);
                   savedLineNo = lineno;
                 }
            ;
params      : param_list  { $$ = $1; }
            | VOID
            ;
param_list  : param_list COMMA param
                 { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $3;
                     $$ = $1; }
                     else $$ = $3;
                 }
            | param { $$ = $1; };
local_decls : local_decls var_decl
                 { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1; }
                     else $$ = $2;
                 }
            | /* empty */ { $$ = NULL; }
            ;
stmt_list   : stmt_list stmt
                 { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1; }
                     else $$ = $2;
                 }
            | /* empty */ { $$ = NULL; }
            ;
stmt        : exp_stmt { $$ = $1; }
            | comp_stmt { $$ = $1; }
            | sel_stmt { $$ = $1; }
            | while_stmt { $$ = $1; }
            | ret_stmt { $$ = $1; }
            ;
exp_stmt    : exp SEMI { $$ = $1; }
            | SEMI { $$ = NULL; }
            ;
args        : arg_list { $$ = $1; }
            | /* empty */ { $$ = NULL; }
            ;
arg_list    : arg_list COMMA exp
                 { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $3;
                     $$ = $1; }
                     else $$ = $3;
                 }
            | exp { $$ = $1; }
            ;
/************************************************/
/* VarK, FunK, ArrVarK, ArrParamK, NonArrParamK */
/************************************************/
var_decl    : type_spec id SEMI
                 { $$ = newDeclNode(VarK);
                   $$->child[0] = $1;
                   $$->lineno = lineno;
                   $$->attr.name = savedName;
                   $$->type = $1->type;
                 }
            | type_spec id LBRACKET num RBRACKET SEMI
                 { $$ = newDeclNode(ArrVarK);
                   $$->child[0] = $1; /* type */
                   $$->lineno = lineno;
                   $$->attr.arr.name = savedName;
                   $$->attr.arr.size = savedNumber;
                   $$->type = $1->type;
                 }
            ;
fun_decl    : type_spec id {
                   $$ = newDeclNode(FunK);
                   $$->lineno = lineno;
                   $$->attr.name = savedName;
                   $$->type = $1->type;
                 }
              LPAREN params RPAREN comp_stmt
                 {
                   $$ = $3;
                   $$->child[0] = $1;
                   $$->child[1] = $5;
                   $$->child[2] = $7;
                 }
            ;
param       : type_spec id
                 { $$ = newDeclNode(NonArrParamK);
                   $$->child[0] = $1;
                   $$->attr.name = savedName;
                   $$->type = $1->type;
                 }
            | type_spec id
              LBRACKET RBRACKET
                 { $$ = newDeclNode(ArrParamK);
                   $$->child[0] = $1;
                   $$->attr.name = savedName;
                   $$->type = $1->type;
                 }
            ;
/*********************************************
StmtNode
IfK, WhileK, CompoundK, ReturnK
*********************************************/
sel_stmt    : IF LPAREN exp RPAREN stmt
                 { $$ = newStmtNode(IfK);
                   $$->child[0] = $3;
                   $$->child[1] = $5;
                   $$->child[2] = NULL;
                 }
            | IF LPAREN exp RPAREN stmt ELSE stmt
                 { $$ = newStmtNode(IfK);
                   $$->child[0] = $3;
                   $$->child[1] = $5;
                   $$->child[2] = $7;
                 };

while_stmt   : WHILE LPAREN exp RPAREN stmt
                { $$ = newStmtNode(WhileK);
                  $$->child[0] = $3;
                  $$->child[1] = $5;
                };
comp_stmt   : LBRACE local_decls stmt_list RBRACE
                 { $$ = newStmtNode(CompoundK);
                   $$->child[0] = $2; /* local variable declarations */
                   $$->child[1] = $3; /* statements */
                 }
            ;
ret_stmt    : RETURN SEMI
                 { $$ = newStmtNode(ReturnK);
                   $$->child[0] = NULL;
                 }
            | RETURN exp SEMI
              {
              $$ = newStmtNode(ReturnK);
              $$->child[0] = $2;
              };

/*********************************************
ExpNode
 OpK, ConstK, AssignK, IdK, TypeK, ArrIdK, CallK
*********************************************/
simple_exp  : add_exp LE add_exp
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = LE;
                 }
            | add_exp LT add_exp
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = LT;
                 }
            | add_exp GT add_exp
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = GT;
                 }
            | add_exp GE add_exp
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = GE;
                 }
            | add_exp EQ add_exp
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = EQ;
                 }
            | add_exp NE add_exp
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = NE;
                 }
            | add_exp { $$ = $1; }
            ;
add_exp     : add_exp PLUS term
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = PLUS;
                 }
            | add_exp MINUS term
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = MINUS;
                 }
            | term { $$ = $1; }
            ;
term        : term TIMES factor
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = TIMES;
                 }
            | term OVER factor
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = OVER;
                 }
            | factor { $$ = $1; }
            ;
factor      : LPAREN exp RPAREN { $$ = $2; }
            | var { $$ = $1; }
            | call { $$ = $1; }
            | NUM
                 { $$ = newExpNode(ConstK);
                   $$->attr.val = atoi(tokenString);
                 }
            ;
exp         : var ASSIGN exp
                 { $$ = newStmtNode(AssignK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                 }
            | simple_exp { $$ = $1; }
            ;
type_spec   : INT
                 { $$ = newExpNode(TypeK);
                   $$->type = Integer;
                 }
            | VOID
                 { $$ = newExpNode(TypeK);
                   $$->type = Void;
                 }
            ;
var         : id
                 { $$ = newExpNode(IdK);
                   $$->attr.name = savedName;
                 }
            | id
                 { $$ = newExpNode(ArrIdK);
                   $$->attr.name = savedName;
                 }
                 LBRACKET exp RBRACKET
                 { $$->child[0] = $3;
                 }
            ;
call        : id {
                 $$ = newExpNode(CallK);
                 $$->attr.name = savedName;
              }
              LPAREN args RPAREN
                 { $$ = $2;
                   $$->child[0] = $4;
                 }
            ;
%%

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
