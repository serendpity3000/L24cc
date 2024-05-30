%{
	#include "node.h"
        #include <cstdio>
        #include <cstdlib>
	NBlock *programBlock; /* the top level root node of our final AST */
	NBlock *currentBlock;
	NBlock *savedBlock;

	extern int yylex();
	void yyerror(const char *s) { std::printf("Error: %s\n", s);std::exit(1); }
%}

/* Represents the many different ways we can access our data */
%union {
	Node *node;
	NBlock *block;
	NExpression *expr;
	NStatement *stmt;
	NIdentifier *ident;
	NVariableDeclaration *var_decl;
	std::vector<NIdentifier*> *ident_list;
	std::vector<NVariableDeclaration*> *varvec;
	std::vector<NExpression*> *exprvec;
	std::string *string;
	int token;
}

/* Define our terminal symbols (tokens). This should
   match our tokens.l lex file. We also define the node type
   they represent.
 */
%token <string> TIDENTIFIER TINTEGER TDOUBLE
%token <token> TCEQ TCNE TCLT TCLE TCGT TCGE TEQUAL
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TCOMMA TDOT TSEMIC
%token <token> TPLUS TMINUS TMUL TDIV TINCRE TDECRE TAND TOR
%token <token> TRETURN TEXTERN TMAIN 
%token <token> TIF TTHEN TELSE TEND
%token <token> TWHILE TFOR
%token <token> TSCAN

/* Define the type of node our nonterminal symbols represent.
   The types refer to the %union declaration above. Ex: when
   we call an ident (defined by union type ident) we are really
   calling an (NIdentifier*). It makes the compiler happy.
 */
%type <ident> ident
%type <expr> numeric expr  
%type <varvec> func_decl_args
%type <exprvec> call_args
%type <block> program stmts block stmt_list
%type <stmt> stmt var_decl func_decl extern_decl if_stmt while_stmt f_stmt for_stmt scan_stmt
%type <token> comparison
%type <ident_list> ident_list

/* Operator precedence for mathematical operators */

%right TEQUAL
%left TOR
%left TAND
%nonassoc TCEQ TCNE TCLT TCLE TCGT TCGE
%left TPLUS TMINUS
%left TMUL TDIV
%right TINCRE TDECRE


%start program

%%

program : TMAIN TLBRACE stmts TRBRACE { programBlock = $3; }
		;
		
stmts : stmt { $$ = new NBlock(); $$->statements.push_back($<stmt>1); currentBlock = $$;}
	  | stmts stmt { $1->statements.push_back($<stmt>2); currentBlock = $1;}
	  ;

stmt : var_decl TSEMIC | func_decl | extern_decl 
	 | expr TSEMIC{ $$ = new NExpressionStatement(*$1); }
	 | TRETURN expr TSEMIC{ $$ = new NReturnStatement(*$2); }
	 | if_stmt
	 | while_stmt
	 | for_stmt
	 | scan_stmt TSEMIC {$$ = $1;}
     ;

block : TLBRACE stmts TRBRACE { $$ = $2; }
	  | TLBRACE TRBRACE { $$ = new NBlock(); }
	  ;

var_decl : ident ident { $$ = new NVariableDeclaration(*$1, *$2); }
		 | ident ident TEQUAL expr { $$ = new NVariableDeclaration(*$1, *$2, $4); }
		 ;

extern_decl : TEXTERN ident ident TLPAREN func_decl_args TRPAREN
                { $$ = new NExternDeclaration(*$2, *$3, *$5); delete $5; }
            ;

func_decl : ident ident TLPAREN func_decl_args TRPAREN block 
			{ $$ = new NFunctionDeclaration(*$1, *$2, *$4, *$6); delete $4; }
		  ;
	
func_decl_args : /*blank*/  { $$ = new VariableList(); }
		  | var_decl { $$ = new VariableList(); $$->push_back($<var_decl>1); }
		  | func_decl_args TCOMMA var_decl { $1->push_back($<var_decl>3); }
		  ;

ident : TIDENTIFIER { $$ = new NIdentifier(*$1); delete $1; }
	  ;

numeric : TINTEGER { $$ = new NInteger(atol($1->c_str())); delete $1; }
		| TDOUBLE { $$ = new NDouble(atof($1->c_str())); delete $1; }
		;
	
expr : ident TEQUAL expr { $$ = new NAssignment(*$<ident>1, *$3); }
	 | ident TLPAREN call_args TRPAREN { $$ = new NMethodCall(*$1, *$3); delete $3; }
	 | ident { $<ident>$ = $1; }
	 | numeric
     | expr TMUL expr { $$ = new NBinaryOperator(*$1, TMUL, *$3); }
     | expr TDIV expr { $$ = new NBinaryOperator(*$1, TDIV, *$3); }
     | expr TPLUS expr { $$ = new NBinaryOperator(*$1, TPLUS, *$3); }
     | expr TMINUS expr { $$ = new NBinaryOperator(*$1, TMINUS, *$3); }
	 | expr TAND expr { $$ = new NBinaryOperator(*$1, TAND, *$3); }
     | expr TOR expr { $$ = new NBinaryOperator(*$1, TOR, *$3); }
 	 | expr comparison expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
     | TLPAREN expr TRPAREN { $$ = $2; }
	 | expr TINCRE { $$ = new NUnaryOperator(*$1, TINCRE); }
	 | TINCRE expr {$$ = new NUnaryOperator(*$2, TINCRE); }
     | expr TDECRE { $$ = new NUnaryOperator(*$1, TDECRE); }
	 | TDECRE expr { $$ = new NUnaryOperator(*$2, TDECRE); }
	 ;
	
call_args : /*blank*/  { $$ = new ExpressionList(); }
		  | expr { $$ = new ExpressionList(); $$->push_back($1); }
		  | call_args TCOMMA expr  { $1->push_back($3); }
		  ;

stmt_list : stmt 
		{ 
			$$ = new NBlock();
			$$->statements.push_back($<stmt>1);
			$$->locals = currentBlock->locals;
			currentBlock = $$;
		}
          | stmt_list stmt { $1->statements.push_back($<stmt>2); }
          ;
if_stmt : TIF TLPAREN expr TRPAREN TTHEN TLBRACE stmt_list TRBRACE TEND
        { $$ = new NIfStatement(*$3, *$7);}
        | TIF TLPAREN expr TRPAREN TTHEN TLBRACE stmt_list TRBRACE TELSE TLBRACE stmt_list TRBRACE TEND
        { $$ = new NIfStatement(*$3, *$7, *$11); }
        ;

stmt_list : stmt 
			{ 
				$$ = new NBlock();
				$$->statements.push_back($<stmt>1);
				$$->locals = currentBlock->locals;
				currentBlock = $$;
			}
            | stmt_list stmt { $1->statements.push_back($<stmt>2); }
            ;

while_stmt : TWHILE TLPAREN expr TRPAREN TLBRACE stmt_list TRBRACE
           { $$ = new NWhileStatement(*$3, *$6); }
           ;

for_stmt : TFOR TLPAREN f_stmt expr TSEMIC expr TRPAREN TLBRACE stmt_list TRBRACE
         { $$ = new NForStatement($3, $4, new NExpressionStatement(*$6), *$9); }
         ;

f_stmt : var_decl TSEMIC {$$ = $1;}
       | expr TSEMIC {$$ = new NExpressionStatement(*$1);}
       | TSEMIC {$$ = nullptr;}
       ;

scan_stmt : TSCAN TLPAREN ident_list TRPAREN { $$ = new NScanStatement(*$3); delete $3; }

ident_list : ident { $$ = new std::vector<NIdentifier*>(); $$->push_back($1); }
           | ident_list TCOMMA ident { $1->push_back($3); $$ = $1; }

comparison : TCEQ | TCNE | TCLT | TCLE | TCGT | TCGE;

%%