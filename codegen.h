void emit(const char *);
char *gen_new_label();
int gen_offsets(SymbolTablePtr s);
void code_gen_expr(AstNode *);
void code_gen_stmt(AstNode *);
void code_gen_method(AstNode *);
void code_gen_binary_expr(AstNode *);
void codegen();
void emit_method();
void emit_label(const char*);
void emit(const char*);
int emit_variables_local (SymbolTablePtr);
void emit_global_variables (SymbolTablePtr);
void format_comment(char*, char*, int);
void emit_comment(const char*);


typedef struct paramstack{
	int  data;
	struct paramstack *next;
}ParamStack;

typedef ParamStack *ParamStackPtr;
