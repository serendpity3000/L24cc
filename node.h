#include <iostream>
#include <vector>
#include <map>
#include <llvm/IR/Value.h>
#include <llvm/IR/Instructions.h>

class CodeGenContext;
class NStatement;
class NExpression;
class NVariableDeclaration;

typedef std::vector<NStatement*> StatementList;
typedef std::vector<NExpression*> ExpressionList;
typedef std::vector<NVariableDeclaration*> VariableList;

class Node {
public:
	virtual ~Node() {}
	virtual llvm::Value* codeGen(CodeGenContext& context) { return NULL; }
};

class NExpression : public Node {
};

class NStatement : public Node {
};

class NInteger : public NExpression {
public:
	long long value;
	NInteger(long long value) : value(value) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NDouble : public NExpression {
public:
	double value;
	NDouble(double value) : value(value) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NIdentifier : public NExpression {
public:
	std::string name;
	NIdentifier(const std::string& name) : name(name) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NMethodCall : public NExpression {
public:
	const NIdentifier& id;
	ExpressionList arguments;
	NMethodCall(const NIdentifier& id, ExpressionList& arguments) :
		id(id), arguments(arguments) { }
	NMethodCall(const NIdentifier& id) : id(id) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NUnaryOperator : public NExpression {
public:
    NExpression& expr;
    int op;

    NUnaryOperator(NExpression& expr, int op) : expr(expr), op(op) {}

    virtual llvm::Value* codeGen(CodeGenContext& context);
};


class NBinaryOperator : public NExpression {
public:
	int op;
	NExpression& lhs;
	NExpression& rhs;
	NBinaryOperator(NExpression& lhs, int op, NExpression& rhs) :
		lhs(lhs), rhs(rhs), op(op) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NAssignment : public NExpression {
public:
	NIdentifier& lhs;
	NExpression& rhs;
	NAssignment(NIdentifier& lhs, NExpression& rhs) : 
		lhs(lhs), rhs(rhs) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NBlock : public NExpression {
public:
    std::vector<NStatement*> statements;
    std::map<std::string, llvm::AllocaInst*> locals;

    NBlock() = default;
    NBlock(const NBlock& parentBlock) : locals(parentBlock.locals) { }

    virtual llvm::Value* codeGen(CodeGenContext& context);
};


class NExpressionStatement : public NStatement {
public:
	NExpression& expression;
	NExpressionStatement(NExpression& expression) : 
		expression(expression) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NReturnStatement : public NStatement {
public:
	NExpression& expression;
	NReturnStatement(NExpression& expression) : 
		expression(expression) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NVariableDeclaration : public NStatement {
public:
	const NIdentifier& type;
	NIdentifier& id;
	NExpression *assignmentExpr;
	NVariableDeclaration(const NIdentifier& type, NIdentifier& id) :
		type(type), id(id) { assignmentExpr = NULL; }
	NVariableDeclaration(const NIdentifier& type, NIdentifier& id, NExpression *assignmentExpr) :
		type(type), id(id), assignmentExpr(assignmentExpr) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NExternDeclaration : public NStatement {
public:
    const NIdentifier& type;
    const NIdentifier& id;
    VariableList arguments;
    NExternDeclaration(const NIdentifier& type, const NIdentifier& id,
            const VariableList& arguments) :
        type(type), id(id), arguments(arguments) {}
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NFunctionDeclaration : public NStatement {
public:
	const NIdentifier& type;
	const NIdentifier& id;
	VariableList arguments;
	NBlock& block;
	NFunctionDeclaration(const NIdentifier& type, const NIdentifier& id, 
			const VariableList& arguments, NBlock& block) :
		type(type), id(id), arguments(arguments), block(block) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NIfStatement : public NStatement {
public:
    NExpression& condition;
    NBlock& trueBlock;
    NBlock* falseBlock;
    NIfStatement(NExpression& condition, NBlock& trueBlock) :
        condition(condition), trueBlock(trueBlock), falseBlock(nullptr) { }
    NIfStatement(NExpression& condition, NBlock& trueBlock, NBlock& falseBlock) :
        condition(condition), trueBlock(trueBlock), falseBlock(&falseBlock) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NWhileStatement : public NStatement {
public:
    NExpression& condition;
    NBlock& body;

    NWhileStatement(NExpression& condition, NBlock& body) 
        : condition(condition), body(body) { }

    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NForStatement : public NStatement {
public:
    NStatement* init;
    NExpression* cond;
    NStatement* inc;
    NBlock& body;

    NForStatement(NStatement* init, NExpression* cond, NStatement* inc, NBlock& body) :
        init(init), cond(cond), inc(inc), body(body) { }

    virtual llvm::Value* codeGen(CodeGenContext& context);
};
