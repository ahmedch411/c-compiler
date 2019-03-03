#include "ast.h"
#include "codegen.h"
#define ASSEMBLY_FILE "program.s"


extern FILE *outfile;
extern AstNodePtr program;
AstNodePtr currentFun, mainFun;
char comment_msg[COMMENT_MAX];
char curr_instruction[INSTRUCTION_MAX_SIZE];
extern SymbolTableStackEntryPtr symbolStackTop;
int numVarMethod;
// char returnLabel[COMMENT_MAX];

ParamStackPtr top = NULL;

void push(int value)
{
   ParamStackPtr newNode;
   newNode = (ParamStackPtr) malloc(sizeof(ParamStack));
   newNode->data = value;
   if(top == NULL)
      newNode->next = NULL;
   else
      newNode->next = top;
   top = newNode;
   // printf("\nInsertion is Success!!!\n %d", top->data);
}

int pop()
{
   if(top == NULL)
   	;
      // printf("\nStack is Empty!!!\n");
   else{
      ParamStackPtr temp = top;
      // printf("\nDeleted element: %d", temp->data);
      top = temp->next;
      return temp->data;
   }
}


void reverse(AstNodePtr first)
{

	AstNodePtr curNode = first, nxtNode;
    first = NULL;

    //Until no more in list, insert current before first and advance.
    while (curNode != NULL) {
        // Need to save next node since we're changing the current.
        nxtNode = curNode->sibling;

        // Insert at start of new list.
        curNode->sibling = first;
        first = curNode;

        // Advance to next.
        curNode = nxtNode;
    }
}


void format_instruction_rr (char *op, char *dest, char *op1, char *op2) {
	snprintf(curr_instruction, INSTRUCTION_MAX_SIZE, "%s %s, %s, %s", op, dest, op1, op2);
	emit(curr_instruction);
}

void format_instruction_ri (char *op, char *dest, char *op1, int op2) {
	snprintf(curr_instruction, INSTRUCTION_MAX_SIZE, "%s %s, %s, %d", op, dest, op1, op2);
	emit(curr_instruction);
}

void format_instruction_lsi (char *op, char *dest, int op1) {
	snprintf(curr_instruction, INSTRUCTION_MAX_SIZE, "%s %s, %d", op, dest, op1);
	emit(curr_instruction);
}

void format_instruction_lsw (char *op, char *dest, int op1, char *op2) {
	snprintf(curr_instruction, INSTRUCTION_MAX_SIZE, "%s %s, %d(%s)", op, dest, op1, op2);
	emit(curr_instruction);
}

void format_instruction_lswr (char *op, char *dest, char* op1, char *op2) {
	snprintf(curr_instruction, INSTRUCTION_MAX_SIZE, "%s %s, %s(%s)", op, dest, op1, op2);
	emit(curr_instruction);
}

void format_instruction_beqz (char *op, char *op1, char* dest) {
	snprintf(curr_instruction, INSTRUCTION_MAX_SIZE, "%s %s, %s", op, op1, dest);
	emit(curr_instruction);
}

void format_instruction_jump (char *op, char* dest) {
	snprintf(curr_instruction, INSTRUCTION_MAX_SIZE, "%s %s", op, dest);
	emit(curr_instruction);
}

void format_comment(char *msg, char *name, int dimension){
	if (dimension < 0) {
		snprintf(comment_msg, COMMENT_MAX, "%s %s", msg, name);
	}
	else {
  		snprintf(comment_msg, COMMENT_MAX, "%s %s[%d]", msg, name, dimension);	
	}
	emit_comment(comment_msg);
}

void emit_binaryExp_Arg() {
	emit_comment("allocate space on stack for 1st operand");
	format_instruction_ri("subu", "$sp", "$sp", 4);
	format_instruction_lsw("sw", "$v0", 0, "$sp");
}

void emit_popStack_binaryExp_Arg() {
	emit_comment("pop 1st operand from stack");
	format_instruction_lsw("lw", "$v1", 0, "$sp");
	format_instruction_ri("addu", "$sp", "$sp", 4);
}

void emit_addInt_stack () {
	emit_comment("allocate space on stack for Int Parameter");
	format_instruction_ri("subu", "$sp", "$sp", 4);
	format_instruction_lsw("sw", "$v0", 0, "$sp");
}

int emit_variables_local (SymbolTablePtr scope)
{
    int i;
	int numVariables = 0;
	for(i=0; i<MAXHASHSIZE;i++) {	
	   ElementPtr symelement = scope->hashTable[i];
		while(symelement) {
           	switch(symelement->stype->kind) {
			   	case INT :
			   		format_comment("make space for", symelement->id, -1);
			   		symelement->offset = -8 - (numVariables*4) - (numVarMethod*4);
			   	   	// printf(" offset for %s: %d\n", symelement->id , symelement->offset);
			   	   	numVariables++;
			   	break;
			   	case ARRAY :
			   		format_comment("make space for", symelement->id, symelement->stype->dimension);
			   		// format_instruction_ri("subu", "$sp", "$sp", 4*symelement->stype->dimension);
			   		// emit(curr_instruction);
			   		symelement->offset = -8 - (numVariables*4) - (numVarMethod*4);
			   		numVariables = numVariables + symelement->stype->dimension;
			   		// printf(" offset for %s: %d\n", symelement->id , symelement->offset);
			   	break;
			   	case VOID:
			   	break;
			   	case FUNCTION :
			   	break;
		   	}
			symelement = symelement->next; 
		}		
	}
	return numVariables;
}

void emit_global_variables (SymbolTablePtr scope)
{
    int i;
	int numVariables = 0;
	for(i=0; i<MAXHASHSIZE;i++) {	
	   ElementPtr symelement = scope->hashTable[i];
		while(symelement) {
           	switch(symelement->stype->kind) {
			   	case INT :
			   		if (strcmp(symelement->id, "input") == 0) {
						break;
					}
			   		format_comment("make space for", symelement->id, -1);
			   		// emit("addu $gp, $gp, 4");
			   		symelement->offset = (numVariables*4);
			   	   	// printf(" offset for %s: %d\n", symelement->id , symelement->offset);
			   	   	numVariables++;
			   	break;
			   	case ARRAY :
			   		format_comment("make space for", symelement->id, symelement->stype->dimension);
			   		// format_instruction_ri("addu", "$gp", "$gp", 4*symelement->stype->dimension);
			   		// emit(curr_instruction);
			   		symelement->offset = (numVariables*4);
			   		numVariables = numVariables + symelement->stype->dimension;
			   		// printf(" offset for %s: %d\n", symelement->id , symelement->offset);
			   	break;
			   	case VOID:
			   	break;
			   	case FUNCTION :
			   	break;
		   	}
			symelement = symelement->next; 
		}		
	}
	format_instruction_ri("addu", "$gp", "$gp", 4*numVariables);
}

int add_params_to_stack(AstNodePtr node) {
	ElementPtr elem = symLookup(node->fname);
	AstNodePtr formalVarNode = elem->snode->children[0];
	AstNodePtr curNode = node->children[0], nxtNode;
	int numArguments;
	ParamStackPtr currentOffset;
	//reverse the current Param List
	node->children[0] = NULL;
    //Until no more in list, insert current before first and advance.
    while (curNode != NULL) {
        // Need to save next node since we're changing the current.
        nxtNode = curNode->sibling;

        // Insert at start of new list.
        curNode->sibling = node->children[0];
        node->children[0] = curNode;

        // Advance to next.
        curNode = nxtNode;
    }
    int i;
    int currIndex =0, prevIndex =0;
	AstNodePtr currParam = node->children[0];
	while(currParam != NULL) {
		numArguments = 0;
		// printf("%s: %d %s %d\n", currParam->nSymbolPtr->id, currParam->nSymbolPtr->offset, typeNameConv(currParam->nSymbolPtr->stype), currParam->nSymbolPtr->stype->dimension);
		if (currParam->eKind == CONST_EXP) {
			// printf("Constant Value detected: %d\n", currParam->nValue);
			code_gen_expr(currParam);
			emit_addInt_stack();
			numArguments++;
		}
		else {
			if (currParam->nSymbolPtr != NULL) {
				if (strcmp(typeNameConv(currParam->nSymbolPtr->stype), "ARRAY") == 0) {

					if (currParam->children[0] != NULL) {
						// printf("Array expression detected: %s with size : %d\n", currParam->nSymbolPtr->id, currParam->nSymbolPtr->stype->dimension);
						code_gen_expr(currParam);
						emit_addInt_stack();
						numArguments++;
					}
					else {
						format_instruction_ri("addu", "$v0", "$fp", currParam->nSymbolPtr->offset);
						emit_addInt_stack();
						numArguments++;
						// for (i=0; i<currParam->nSymbolPtr->stype->dimension; i++) {
						// 	AstNodePtr currArrayNode = new_ExprNode(ARRAY_EXP);
						// 	currArrayNode->nSymbolPtr = currParam->n0ymbolPtr;
						// 	currArrayNode->children[0] = new_ExprNode(CONST_EXP);
						// 	currArrayNode->children[0]->nValue = i;
						// 	code_gen_expr(currArrayNode);
						// 	emit_addInt_stack();
						// 	// printf("Array index: %s[%d] at offset: %d \n", currArrayNode->nSymbolPtr->id, i, currIndex);
						// 	numArguments++;
						// }	
					}	
				}
				else {
					// printf("Variable detected: %s\n", currParam->nSymbolPtr->id);
					code_gen_expr(currParam);
					emit_addInt_stack();
					numArguments++;
				}
			}
			else {
				// printf("ADD expression detected\n");
				code_gen_expr(currParam);
				emit_addInt_stack();
				numArguments++;
			}
		}
		// printf("offset value is :%d for %s\n", currIndex, formalVarNode->nSymbolPtr->id);
		// prevIndex = currIndex;
		// formalVarNode->nSymbolPtr->offset = currIndex;
		// printf("%s: %d %s %d\n", formalVarNode->nSymbolPtr->id, formalVarNode->nSymbolPtr->offset, typeNameConv(formalVarNode->nSymbolPtr->stype), formalVarNode->nSymbolPtr->stype->dimension);
		currParam = currParam->sibling;
		// printf("Number of Arguments:%d\n", numArguments);
		if (top == NULL) {
			push(numArguments * 4);
		}
		else {
			currentOffset = top;
			while (currentOffset != NULL) {
				currentOffset->data = currentOffset->data + (numArguments*4);
				currentOffset = currentOffset->next;
			}
			push(numArguments*4);
		}
	}
	// currentOffset = top;
	// while(currentOffset != NULL) {
	// 	printf("Offset is : %d\n", currentOffset->data);
	// 	currentOffset = currentOffset->next;
	// }

	while (formalVarNode != NULL) {
		formalVarNode->nSymbolPtr->offset = pop();
		// printf("offset value is :%d for %s\n",formalVarNode->nSymbolPtr->offset, formalVarNode->nSymbolPtr->id);
		formalVarNode = formalVarNode->sibling;
	}
	return numArguments;
}

void add_array_element_stack(int i) {
	format_instruction_lsi("lsi", "$v0", i);
	emit_addInt_stack();
}

void local_array_expr (int offset) {
	format_instruction_ri("mul", "$t2", "$v0", 4);
	format_instruction_lsi ("li", "$t0", offset);
	format_instruction_rr("addu", "$t1", "$fp", "$t0");
	format_instruction_rr("subu", "$t1", "$t1", "$t2");
}

void global_array_expr (int offset) {
	format_instruction_ri("mul", "$t2", "$v0", 4);
	format_instruction_lsi ("li", "$t0", offset);
	format_instruction_rr("addu", "$t1", "$gp", "$t0");
	format_instruction_rr("addu", "$t1", "$t1", "$t2");
}

void argument_array_expr(int offset) {
	format_instruction_ri("mul", "$t2", "$v0", 4);
	format_instruction_lsi ("li", "$t0", offset);
	format_instruction_rr("addu", "$t1", "$fp", "$t0");
	format_instruction_rr("subu", "$t1", "$t1", "$t2");
}
// 
// void emit_method(AstNodePtr node) {
void emit_method() {
	// int numVar;
	// emit_label(node->nSymbolPtr->id);
	emit("subu $sp, $sp, 8");
	emit("sw $fp, 4($sp)");
	emit("sw $ra, 0($sp)");
	emit("addiu $fp, $sp, 4");
	// if(node->children[1] != NULL) {
	// 	numVar = emit_variables_local(node->children[1]->nSymbolTabPtr);
	// }
	//---format_instruction_ri("subu", "$sp", "$sp", 4*numVar);
	// return numVar;
}

void emit_label(const char *label){
	fprintf(outfile, "%s:\n", label); 
}

void emit_comment(const char *comment) {
	fprintf(outfile, "# %s\n", comment);
}
void emit(const char *instr){
	fprintf(outfile, "\t%s\n", instr);
}

//0 for loop start
//1 for loop end
//2 for conditional start
//3 for conditional end
char *gen_new_label(int type, char *fname){
	static int loop_start = 0;
	static int loop_end = 0;
	static int conditional_start = 0;
	static int conditional_end = 0;
	char *returnLabel = (char *) malloc(sizeof(char) * COMMENT_MAX);
	switch(type) {
		case 0:
			snprintf(returnLabel, COMMENT_MAX, "loop%d_start", loop_start);
			loop_start++;
		break;
		case 1:
			snprintf(returnLabel, COMMENT_MAX, "loop%d_end", loop_end);
			loop_end++;
		break;
		case 2:
			snprintf(returnLabel, COMMENT_MAX, "ELSE%d_start", conditional_start);
			conditional_start++;
		break;
		case 3:
			snprintf(returnLabel, COMMENT_MAX, "IFELSE%d_end", conditional_end);
			conditional_end++;
		break;
		case 4:
			snprintf(returnLabel, COMMENT_MAX, "%s_exit", fname);
		break;
		default:
		// printf("invalid number in gen_new_label\n");
		exit(0);
		break;
	}
	return returnLabel;
}

void code_gen_expr(AstNode *node){
	AstNodePtr params;
	ElementPtr elem;
	int numArguments;
	char *label_exit;
	switch(node->eKind)
	{
		case CONST_EXP:
			emit_comment("constant expression");
			format_instruction_lsi("li", "$v0", node->nValue);
			break;
		case VAR_EXP:
			emit_comment("variable expression");
			//global variable or not
			if (node->nSymbolPtr->scope == 0) {
				format_instruction_lsw("lw", "$v0", node->nSymbolPtr->offset, "$gp");
			}
			else if (node->nSymbolPtr->scope == 1) {
				format_instruction_lsw("lw", "$v0", node->nSymbolPtr->offset, "$fp");
			}
			else {
				format_instruction_lsw("lw", "$v0", node->nSymbolPtr->offset, "$fp");
			}
			// printf("scope for %s is %d\n", node->nSymbolPtr->id, node->nSymbolPtr->scope);	
			break;
		case ADD_EXP:
			emit_comment("add expression");
			code_gen_expr(node->children[0]);
			emit_binaryExp_Arg();
			code_gen_expr(node->children[1]);
			emit_popStack_binaryExp_Arg();
			format_instruction_rr("addu", "$v0", "$v0", "$v1");
			break;
		case SUB_EXP:
			emit_comment("sub expression");
			code_gen_expr(node->children[0]);
			emit_binaryExp_Arg();
			code_gen_expr(node->children[1]);
			emit_popStack_binaryExp_Arg();
			format_instruction_rr("subu", "$v0", "$v1", "$v0");
			break;
		case MULT_EXP:
			emit_comment("mult expression");
			code_gen_expr(node->children[0]);
			emit_binaryExp_Arg();
			code_gen_expr(node->children[1]);
			emit_popStack_binaryExp_Arg();
			format_instruction_rr("mul", "$v0", "$v0", "$v1");
			break;
		case DIV_EXP:
			emit_comment("div expression");
			code_gen_expr(node->children[0]);
			emit_binaryExp_Arg();
			code_gen_expr(node->children[1]);
			emit_popStack_binaryExp_Arg();
			format_instruction_rr("divu", "$v0", "$v1", "$v0");
			break;
		case EQ_EXP:
			emit_comment("EQ expression");
			code_gen_expr(node->children[0]);
			emit_binaryExp_Arg();
			code_gen_expr(node->children[1]);
			emit_popStack_binaryExp_Arg();
			format_instruction_rr("seq", "$v0", "$v0", "$v1");
			break;
		case NE_EXP:
			emit_comment("NE expression");
			code_gen_expr(node->children[0]);
			emit_binaryExp_Arg();
			code_gen_expr(node->children[1]);
			emit_popStack_binaryExp_Arg();
			format_instruction_rr("sne", "$v0", "$v0", "$v1");
			break;
		case GT_EXP:
			emit_comment("GT expression");
			code_gen_expr(node->children[0]);
			emit_binaryExp_Arg();
			code_gen_expr(node->children[1]);
			emit_popStack_binaryExp_Arg();
			format_instruction_rr("sgt", "$v0", "$v1", "$v0");
			break;
		case GE_EXP:
			emit_comment("GE expression");
			code_gen_expr(node->children[0]);
			emit_binaryExp_Arg();
			code_gen_expr(node->children[1]);
			emit_popStack_binaryExp_Arg();
			format_instruction_rr("sge", "$v0", "$v1", "$v0");
			break;
		case LT_EXP:
			emit_comment("LT expression");
			code_gen_expr(node->children[0]);
			emit_binaryExp_Arg();
			code_gen_expr(node->children[1]);
			emit_popStack_binaryExp_Arg();
			format_instruction_rr("slt", "$v0", "$v1", "$v0");
			break;
		case LE_EXP:
			emit_comment("LE expression");
			code_gen_expr(node->children[0]);
			emit_binaryExp_Arg();
			code_gen_expr(node->children[1]);
			emit_popStack_binaryExp_Arg();
			format_instruction_rr("sle", "$v0", "$v1", "$v0");
			break;
		case ASSI_EXP:
			emit_comment("ASSI expression");
			//LHS is global variable
			if (node->children[0]->nSymbolPtr->scope == 0) {
				//an array expression on LHS
				if (node->children[0]->eKind == ARRAY_EXP) {
					if (node->children[0]->children[0]->eKind == CONST_EXP) {
						emit_comment("array has integer as index");
						code_gen_expr(node->children[1]);
						format_instruction_lsw("sw", "$v0", node->children[0]->nSymbolPtr->offset + (node->children[0]->children[0]->nValue * 4), "$gp");
					}
					//array index is expression
					else {	
						emit_comment("array has expression as index");
						code_gen_expr(node->children[0]->children[0]);
						global_array_expr(node->children[0]->nSymbolPtr->offset);
						emit("move $t4, $t1");
						code_gen_expr(node->children[1]);
						format_instruction_lsw("sw", "$v0", 0, "$t4");
					}
				}
				else {
					code_gen_expr(node->children[1]);
					format_instruction_lsw("sw", "$v0", node->children[0]->nSymbolPtr->offset, "$gp");
				}
			}
			else if (node->children[0]->nSymbolPtr->scope == 1) {
				// printf("FORMALVAR %s detected on LHS\n", node->children[0]->nSymbolPtr->id);
				//an array expression on LHS
				if (node->children[0]->eKind == ARRAY_EXP) {
					//array index is integer literal
					if (node->children[0]->children[0]->eKind == CONST_EXP) {
						emit_comment("array has integer as index");
						code_gen_expr(node->children[1]);
						emit("move $v1, $v0");
						//get the address of the array passed by reference in $v0
						format_instruction_lsw("lw", "$v0", node->children[0]->nSymbolPtr->offset, "$fp");
						format_instruction_lsw("sw", "$v1", -(node->children[0]->children[0]->nValue * 4), "$v0");
					}
					//array index is expression
					else {
						emit_comment("array has expression as index");
						code_gen_expr(node->children[0]->children[0]);
						//index value in $t1
						emit("move $t1, $v0");
						//compute and store the RHS in $v1
						code_gen_expr(node->children[1]);
						emit("move $v1, $v0");
						//get the address of the array passed by reference in $v0
						format_instruction_lsw("lw", "$v0", node->children[0]->nSymbolPtr->offset, "$fp");
						//multiply index with 4 to get byte offset
						format_instruction_ri("mul", "$t2", "$t1", 4);
						//subtract the byte offset from base address
						format_instruction_rr("subu", "$t1", "$v0", "$t2");
						//store the value to the computed address in $t1
						format_instruction_lsw("sw", "$v1", 0, "$t1");
					}
				}
				else {
					code_gen_expr(node->children[1]);
					format_instruction_lsw("sw", "$v0", node->children[0]->nSymbolPtr->offset, "$fp");
				}
			}
			else {
				//an array expression on LHS
				if (node->children[0]->eKind == ARRAY_EXP) {
					emit_comment("array expression on LHS");
					//array index is an integer literal
					if (node->children[0]->children[0]->eKind == CONST_EXP) {
						emit_comment("array has integer as index");
						code_gen_expr(node->children[1]);
						format_instruction_lsw("sw", "$v0", node->children[0]->nSymbolPtr->offset - (node->children[0]->children[0]->nValue * 4), "$fp");
					}
					//array index is expression
					else {	
						emit_comment("array has expression as index");
						code_gen_expr(node->children[0]->children[0]);
						local_array_expr(node->children[0]->nSymbolPtr->offset);
						emit("move $t4, $t1");
						code_gen_expr(node->children[1]);
						format_instruction_lsw("sw", "$v0", 0, "$t4");
					}
				}
				else {
					code_gen_expr(node->children[1]);
					format_instruction_lsw("sw", "$v0", node->children[0]->nSymbolPtr->offset, "$fp");
				}
			}
			break;
		case CALL_EXP:
			emit_comment("CALL expression");
			if (strcmp(node->fname, "output") == 0) {
				emit_comment("output");
				code_gen_expr(node->children[0]);
				emit("move $a0, $v0");
				emit("li $v0, 1");
				emit("syscall");
			}
			else if (strcmp(node->fname, "input") == 0) {
				emit_comment("input");
				emit("li $v0, 5");
				emit("syscall");
			}
			else {
				emit_comment("function call to user defined");
				numArguments = add_params_to_stack(node);
				format_instruction_jump("jal", node->fname);
				format_instruction_ri("addu", "$sp", "$sp", numArguments*4);
				// label_exit = gen_new_label(4, node->fname);
				// format_instruction_jump("j", label_exit);
			}
			break;
		case ARRAY_EXP:
			emit_comment("ARRAY expression");
			//check if array is in global scope
			if (node->nSymbolPtr->scope == 0) {
				// printf("%s is in global scope\n", node->nSymbolPtr->id);
				//array index is an integer literal
				if (node->children[0]->eKind == CONST_EXP) {
					emit_comment("global array has integer as index");
					format_instruction_lsw("lw", "$v0", node->nSymbolPtr->offset + (node->children[0]->nValue * 4), "$gp");
				}
				//array index is expression
				else {	
					emit_comment("global array has expression as index");
					code_gen_expr(node->children[0]);
					global_array_expr(node->nSymbolPtr->offset);
					format_instruction_lsw("lw", "$v0", 0, "$t1");
				}
			}
			//FORMALVAR
			else if (node->nSymbolPtr->scope == 1) {
				// printf("%s is FORMALVAR with index : %d and offset %d\n", node->nSymbolPtr->id, node->children[0]->nValue, node->nSymbolPtr->offset);
				if (node->children[0]->eKind == CONST_EXP) {
					// printf("%s is FORMALVAR with index : %d and offset %d\n", node->nSymbolPtr->id, node->children[0]->nValue, node->nSymbolPtr->offset);
					emit_comment("argument array has integer as index");
					format_instruction_lsw("lw", "$v0", node->nSymbolPtr->offset, "$fp");
					format_instruction_lsw("lw", "$v1", -(node->children[0]->nValue * 4), "$v0");
					emit("move $v0, $v1");
				}
				else {	
					// printf("%s is FORMALVAR with offset %d\n", node->nSymbolPtr->id, node->nSymbolPtr->offset);
					emit_comment("argument array has expression as index");
					code_gen_expr(node->children[0]);
					format_instruction_lsw("lw", "$v1", node->nSymbolPtr->offset, "$fp");
					format_instruction_ri("mul", "$t2", "$v0", 4);
					emit("subu $v1, $v1, $t2");
					format_instruction_lsw("lw", "$t2", 0, "$v1");
					emit("move $v0, $t2");
				}
			}
			//local scope array expression
			else {
				// printf("%s is in local scope\n", node->nSymbolPtr->id);
				//array index is an integer literal
				if (node->children[0]->eKind == CONST_EXP) {
					emit_comment("array has integer as index");
					format_instruction_lsw("lw", "$v0", node->nSymbolPtr->offset - (node->children[0]->nValue * 4), "$fp");
				}
				//array index is expression
				else {	
					emit_comment("array has expression as index");
					code_gen_expr(node->children[0]);
					local_array_expr(node->nSymbolPtr->offset);
					format_instruction_lsw("lw", "$v0", 0, "$t1");
				}
			}
			break;
	}	
}

void code_gen_stmt(AstNodePtr node){
	AstNodePtr stmt;
	char *label_start, *label_end;
	int numVar;
	switch(node->sKind)
	{
		case IF_THEN_ELSE_STMT:
			emit_comment("conditional statment");
			//we have only if condition
			if (node->children[2] == NULL){
				label_end = gen_new_label(3, NULL);
				code_gen_expr(node->children[0]);
				format_instruction_beqz("beqz", "$v0", label_end);
				code_gen_stmt(node->children[1]);
				emit_label(label_end);
			}
			//we have if else condition
			else {
				label_end = gen_new_label(3, NULL);
				label_start = gen_new_label(2, NULL);
				code_gen_expr(node->children[0]);
				format_instruction_beqz("beqz", "$v0", label_end);
				code_gen_stmt(node->children[1]);
				format_instruction_jump("j", label_start);
				emit_label(label_end);
				code_gen_stmt(node->children[2]);
				emit_label(label_start);
			}
			break;
		case WHILE_STMT:
			emit_comment("while statment");
			//label for while loop
			label_start = gen_new_label(0, NULL);
			label_end = gen_new_label(1, NULL);
			// printf("%s, %s\n", label_start, label_end);
			emit_label(label_start);
			code_gen_expr(node->children[0]);
			format_instruction_beqz("beqz", "$v0", label_end);
			code_gen_stmt(node->children[1]);
			format_instruction_jump("j", label_start);
			emit_label(label_end);
			break;
		case RETURN_STMT:
			emit_comment("return statment");
			//check for null return statement
			if (node->children[0] != NULL) {
				//emit code for expression in return statement and store in $v0
				code_gen_expr(node->children[0]);	
			}
			label_end = gen_new_label(4, currentFun->nSymbolPtr->id);
			format_instruction_jump ("j", label_end);
			break;
		case COMPOUND_STMT:
			emit_comment("compound statement");
			numVar = emit_variables_local(node->nSymbolTabPtr);
			format_instruction_ri("subu", "$sp", "$sp", (4*numVar) + (numVarMethod * 4));
			numVarMethod = numVarMethod + numVar;
			// printf("local variables are:%d\n", numVar);
			stmt = node->children[0];
			while (stmt != NULL)
			{
				code_gen_stmt(stmt);
				stmt = stmt->sibling;
			}
			format_instruction_ri("addu", "$sp", "$sp", 4*numVar);
			break;
		case EXPRESSION_STMT:
			emit_comment("expression statement");
			if (node-> children[0] != NULL)
			  code_gen_expr(node->children[0]);		
			break;
		default:
			break;
	}
}

void code_gen_method(AstNode *node){
	// int numVar = emit_method(node);();
	emit_label(node->nSymbolPtr->id);//----------
	emit_method();
	// emit_variables_local();
	AstNodePtr stmt = node->children[1];
	char *label_exit;
	numVarMethod = 0;
	while (stmt != NULL)
	{
		code_gen_stmt(stmt);
		stmt = stmt->sibling;
	}
	label_exit = gen_new_label(4, node->nSymbolPtr->id);
	emit_label(label_exit);
	format_instruction_lsw("lw", "$ra", -4, "$fp");
	format_instruction_lsw("lw", "$fp", 0, "$fp");
	format_instruction_ri("addu", "$sp", "$sp", 8);
	emit("jr $ra");
}

void codegen(){
	outfile = fopen(ASSEMBLY_FILE, "w+");
	emit(".data");
	emit_global_variables(symbolStackTop->symbolTablePtr);
	emit(".text");
	emit(".align 2");
	emit(".globl main");
	emit("\n");
	// emit(".global main");
	// emit_label("_main");
	// emit("jal main");
	currentFun = program;
	mainFun = program;
	while (strcmp(mainFun->nSymbolPtr->id, "main") != 0) {
		mainFun = mainFun->sibling;
	}
	code_gen_method(mainFun);
	emit("\n");
	while (currentFun != NULL)
	{
		//skip input, main and output methods from code generation
		if (strcmp(currentFun->nSymbolPtr->id, "input") == 0 || strcmp(currentFun->nSymbolPtr->id, "output") == 0 || strcmp(currentFun->nSymbolPtr->id, "main") == 0) {
			currentFun = currentFun->sibling;
			continue;
		}
		code_gen_method(currentFun);
		emit("\n");
		currentFun = currentFun->sibling;
	}

	fclose(outfile);
}	
