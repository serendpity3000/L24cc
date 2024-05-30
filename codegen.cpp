#include "node.h"
#include "codegen.h"
#include "parser.hpp"

using namespace std;

/* Compile the AST into a module */
/*
void CodeGenContext::generateCode(NBlock& root) {
    std::cout << "Generating code..." << std::endl;

    // Create the top level interpreter function to call as entry
    std::vector<Type*> argTypes;
    FunctionType *ftype = FunctionType::get(Type::getVoidTy(MyContext), makeArrayRef(argTypes), false);
    mainFunction = Function::Create(ftype, GlobalValue::InternalLinkage, "main", module);
    BasicBlock *bblock = BasicBlock::Create(MyContext, "entry", mainFunction, 0);

    // Push a new variable/block context
    pushBlock(bblock);
    root.codeGen(*this); // Emit bytecode for the toplevel block
    ReturnInst::Create(MyContext, bblock);
    popBlock();

    std::cout << "Code is generated." << std::endl;
	//module->print(llvm::errs(), nullptr);

	std::cout << "ok1" << std::endl;
    legacy::PassManager pm;
    pm.add(createPrintModulePass(outs()));
	std::cout << "ok2" << std::endl;
    pm.run(*module);
}
*/
void CodeGenContext::generateCode(NBlock& root) {
    std::cout << "Generating code..." << std::endl;

    // 创建顶级解释器函数以作为入口
    std::vector<Type*> argTypes;
    FunctionType *ftype = FunctionType::get(Type::getVoidTy(MyContext), makeArrayRef(argTypes), false);
    mainFunction = Function::Create(ftype, GlobalValue::InternalLinkage, "main", module);
    BasicBlock *bblock = BasicBlock::Create(MyContext, "entry", mainFunction, 0);

    // 推入新的变量/块上下文
    pushBlock(bblock);
    root.codeGen(*this); // 为顶层块生成字节码

    // 添加返回指令到入口块
    if (!currentBlock()->getTerminator()) {
        ReturnInst::Create(MyContext, currentBlock());
    }

    popBlock();

    std::cout << "Code is generated." << std::endl;

    // 打印生成的 LLVM IR 到 stderr
    //module->print(llvm::errs(), nullptr);
    std::cout << "LLVM IR printed." << std::endl;

    std::cout << "ok1" << std::endl;
    legacy::PassManager pm;
    pm.add(createPrintModulePass(outs()));
    std::cout << "ok2" << std::endl;
    pm.run(*module);
}


/* Executes the AST by running the main function */
GenericValue CodeGenContext::runCode() {
	std::cout << "Running code...\n";
	ExecutionEngine *ee = EngineBuilder( unique_ptr<Module>(module) ).create();
	ee->finalizeObject();
	vector<GenericValue> noargs;
	GenericValue v = ee->runFunction(mainFunction, noargs);
	std::cout << "Code was run.\n";
	return v;
}

/* Returns an LLVM type based on the identifier */
static Type *typeOf(const NIdentifier& type) 
{
	if (type.name.compare("int") == 0) {
		return Type::getInt64Ty(MyContext);
	}
	else if (type.name.compare("double") == 0) {
		return Type::getDoubleTy(MyContext);
	}
	return Type::getVoidTy(MyContext);
}

/* -- Code Generation -- */

Value* NInteger::codeGen(CodeGenContext& context)
{
	std::cout << "Creating integer: " << value << endl;
	return ConstantInt::get(Type::getInt64Ty(MyContext), value, true);
}

Value* NDouble::codeGen(CodeGenContext& context)
{
	std::cout << "Creating double: " << value << endl;
	return ConstantFP::get(Type::getDoubleTy(MyContext), value);
}

Value* NIdentifier::codeGen(CodeGenContext& context)
{
	std::cout << "Creating identifier reference: " << name << endl;
	if (context.locals().find(name) == context.locals().end()) {
		std::cerr << "undeclared variable " << name << endl;
		return NULL;
	}

	// return nullptr;  
	return new LoadInst(context.locals()[name]->getType(),context.locals()[name], name, false, context.currentBlock());
}

Value* NMethodCall::codeGen(CodeGenContext& context)
{
	Function *function = context.module->getFunction(id.name.c_str());
	if (function == NULL) {
		std::cerr << "no such function " << id.name << endl;
	}
	std::vector<Value*> args;
	ExpressionList::const_iterator it;
	for (it = arguments.begin(); it != arguments.end(); it++) {
		args.push_back((**it).codeGen(context));
	}
	CallInst *call = CallInst::Create(function, makeArrayRef(args), "", context.currentBlock());
	std::cout << "Creating method call: " << id.name << endl;
	return call;
}

/*
Value* NBinaryOperator::codeGen(CodeGenContext& context)
{

	std::cout << "Creating binary operation " << op << endl;
	Instruction::BinaryOps instr;
	switch (op) {
		case TPLUS: 	instr = Instruction::Add; goto math;
		case TMINUS: 	instr = Instruction::Sub; goto math;
		case TMUL: 		instr = Instruction::Mul; goto math;
		case TDIV: 		instr = Instruction::SDiv; goto math;
				
		
		case TCEQ: 
		case TCNE:  
		case TCLT: 
		case TCLE: 
		case TCGT:  
		case TCGE:
	}
	return NULL;
math:
	return BinaryOperator::Create(instr, lhs.codeGen(context), 
		rhs.codeGen(context), "", context.currentBlock());
}
*/

Value* NUnaryOperator::codeGen(CodeGenContext& context) {
    std::cout << "Creating unary operation " << "  " << op << std::endl;
    llvm::Value* exprValue = expr.codeGen(context);
    if (!exprValue) {
        return nullptr;
    }


	llvm::Value* result;
    switch (op) {
        case TINCRE:
            result = llvm::BinaryOperator::CreateAdd(exprValue, llvm::ConstantInt::get(llvm::Type::getInt64Ty(MyContext), 1, true), "inc", context.currentBlock());
			break;
		case TDECRE:
            result = llvm::BinaryOperator::CreateSub(exprValue, llvm::ConstantInt::get(llvm::Type::getInt64Ty(MyContext), 1, true), "dec", context.currentBlock());
			break;
		default:
            return nullptr;
    }

	llvm::Value* variablePtr = context.locals()[((NIdentifier&)expr).name];
    new llvm::StoreInst(result, variablePtr, false, context.currentBlock());

    return result;
}

Value* NBinaryOperator::codeGen(CodeGenContext& context)
{
    std::cout << "Creating binary operation " << "  " << op << endl;
    llvm::IRBuilder<> builder(context.currentBlock());
    Instruction::BinaryOps instr;
    CmpInst::Predicate pred;
    PHINode *phiNode;
	Value* result;

    switch (op) {
        case TPLUS:  instr = Instruction::Add; goto math;
        case TMINUS: instr = Instruction::Sub; goto math;
        case TMUL:   instr = Instruction::Mul; goto math;
        case TDIV:   instr = Instruction::SDiv; goto math;

        case TCEQ:   pred = CmpInst::ICMP_EQ; goto compare;
        case TCNE:   pred = CmpInst::ICMP_NE; goto compare;
        case TCLT:   pred = CmpInst::ICMP_SLT; goto compare;
        case TCLE:   pred = CmpInst::ICMP_SLE; goto compare;
        case TCGT:   pred = CmpInst::ICMP_SGT; goto compare;
        case TCGE:   pred = CmpInst::ICMP_SGE; goto compare;

        case TAND: goto logical_and;
        case TOR: goto logical_or;
    }
    return nullptr;

math:
    return BinaryOperator::Create(instr, lhs.codeGen(context), 
        rhs.codeGen(context), "", context.currentBlock());

compare:
    result = CmpInst::Create(Instruction::ICmp, pred, lhs.codeGen(context), rhs.codeGen(context), "", context.currentBlock());
    return result;

logical_and: {
    BasicBlock *lhsBlock = context.currentBlock();
    BasicBlock *rhsBlock = BasicBlock::Create(MyContext, "rhs", lhsBlock->getParent());
    BasicBlock *mergeBlock = BasicBlock::Create(MyContext, "merge", lhsBlock->getParent());

    Value *lhsValue = lhs.codeGen(context);
    if (!lhsValue->getType()->isIntegerTy(1)) {
        lhsValue = builder.CreateICmpNE(lhsValue, ConstantInt::get(lhsValue->getType(), 0), "tmp");
    }
    builder.CreateCondBr(lhsValue, rhsBlock, mergeBlock);

    context.pushBlock(rhsBlock);
    Value *rhsValue = rhs.codeGen(context);
    if (!rhsValue->getType()->isIntegerTy(1)) {
        rhsValue = builder.CreateICmpNE(rhsValue, ConstantInt::get(rhsValue->getType(), 0), "tmp");
    }
    builder.SetInsertPoint(rhsBlock);
    builder.CreateBr(mergeBlock);

    context.popBlock();
    context.pushBlock(mergeBlock);
    builder.SetInsertPoint(mergeBlock);

    phiNode = builder.CreatePHI(Type::getInt1Ty(MyContext), 2);
    phiNode->addIncoming(builder.getInt1(false), lhsBlock);
    phiNode->addIncoming(rhsValue, rhsBlock);

    return phiNode;
}

logical_or: {
    BasicBlock *lhsBlock = context.currentBlock();
    BasicBlock *rhsBlock = BasicBlock::Create(MyContext, "rhs", lhsBlock->getParent());
    BasicBlock *mergeBlock = BasicBlock::Create(MyContext, "merge", lhsBlock->getParent());

    Value *lhsValue = lhs.codeGen(context);
    if (!lhsValue->getType()->isIntegerTy(1)) {
        lhsValue = builder.CreateICmpNE(lhsValue, ConstantInt::get(lhsValue->getType(), 0), "tmp");
    }
    builder.CreateCondBr(lhsValue, mergeBlock, rhsBlock);

    context.pushBlock(rhsBlock);
    Value *rhsValue = rhs.codeGen(context);
    if (!rhsValue->getType()->isIntegerTy(1)) {
        rhsValue = builder.CreateICmpNE(rhsValue, ConstantInt::get(rhsValue->getType(), 0), "tmp");
    }
    builder.SetInsertPoint(rhsBlock);
    builder.CreateBr(mergeBlock);

    context.popBlock();
    context.pushBlock(mergeBlock);
    builder.SetInsertPoint(mergeBlock);

    phiNode = builder.CreatePHI(Type::getInt1Ty(MyContext), 2);
    phiNode->addIncoming(builder.getInt1(true), lhsBlock);
    phiNode->addIncoming(rhsValue, rhsBlock);

    return phiNode;
}
}



/*
Value* NAssignment::codeGen(CodeGenContext& context)
{
	std::cout << "Creating assignment for " << lhs.name << endl;
	if (context.locals().find(lhs.name) == context.locals().end()) {
		std::cerr << "undeclared variable " << lhs.name << endl;
		return NULL;
	}
	return new StoreInst(rhs.codeGen(context), context.locals()[lhs.name], false, context.currentBlock());
}
*/

Value* NAssignment::codeGen(CodeGenContext& context)
{
    if (context.locals().find(lhs.name) == context.locals().end()) {
        std::cerr << "undeclared variable " << lhs.name << std::endl;
        return nullptr;
    }

    llvm::Value* value = rhs.codeGen(context);
    if (!value) {
        return nullptr;
    }

    llvm::Value* variable = context.locals()[lhs.name];
    return new llvm::StoreInst(value, variable, false, context.currentBlock());
}


Value* NBlock::codeGen(CodeGenContext& context)
{
	StatementList::const_iterator it;
	Value *last = NULL;
	for (it = statements.begin(); it != statements.end(); it++) {
		std::cout << "Generating code for " << typeid(**it).name() << endl;
		last = (**it).codeGen(context);
	}
	std::cout << "Creating block" << endl;
	return last;
}

Value* NExpressionStatement::codeGen(CodeGenContext& context)
{
	std::cout << "Generating code for " << typeid(expression).name() << endl;
	return expression.codeGen(context);
}

Value* NReturnStatement::codeGen(CodeGenContext& context)
{
	std::cout << "Generating return code for " << typeid(expression).name() << endl;
	Value *returnValue = expression.codeGen(context);
	context.setCurrentReturnValue(returnValue);
	return returnValue;
}

Value* NVariableDeclaration::codeGen(CodeGenContext& context)
{
	std::cout << "Creating variable declaration " << type.name << " " << id.name << endl;
	AllocaInst *alloc = new AllocaInst(typeOf(type),4, id.name.c_str(), context.currentBlock());
	context.locals()[id.name] = alloc;
	if (assignmentExpr != NULL) {
		NAssignment assn(id, *assignmentExpr);
		assn.codeGen(context);
	}
	return alloc;
}

Value* NExternDeclaration::codeGen(CodeGenContext& context)
{
    vector<Type*> argTypes;
    VariableList::const_iterator it;
    for (it = arguments.begin(); it != arguments.end(); it++) {
        argTypes.push_back(typeOf((**it).type));
    }
    FunctionType *ftype = FunctionType::get(typeOf(type), makeArrayRef(argTypes), false);
    Function *function = Function::Create(ftype, GlobalValue::ExternalLinkage, id.name.c_str(), context.module);
    return function;
}

Value* NFunctionDeclaration::codeGen(CodeGenContext& context)
{
	vector<Type*> argTypes;
	VariableList::const_iterator it;
	for (it = arguments.begin(); it != arguments.end(); it++) {
		argTypes.push_back(typeOf((**it).type));
	}
	FunctionType *ftype = FunctionType::get(typeOf(type), makeArrayRef(argTypes), false);
	Function *function = Function::Create(ftype, GlobalValue::InternalLinkage, id.name.c_str(), context.module);
	BasicBlock *bblock = BasicBlock::Create(MyContext, "entry", function, 0);

	context.pushBlock(bblock);

	Function::arg_iterator argsValues = function->arg_begin();
    Value* argumentValue;

	for (it = arguments.begin(); it != arguments.end(); it++) {
		(**it).codeGen(context);
		
		argumentValue = &*argsValues++;
		argumentValue->setName((*it)->id.name.c_str());
		StoreInst *inst = new StoreInst(argumentValue, context.locals()[(*it)->id.name], false, bblock);
	}
	
	block.codeGen(context);
	ReturnInst::Create(MyContext, context.getCurrentReturnValue(), bblock);

	context.popBlock();
	std::cout << "Creating function: " << id.name << endl;
	return function;
}

/*
Value* NIfStatement::codeGen(CodeGenContext& context) {
    std::cout << "Generating code for if statement" << std::endl;
    
    // 生成条件表达式代码
    Value* condValue = condition.codeGen(context);
    if (!condValue) {
        std::cerr << "Condition code generation failed." << std::endl;
        return nullptr;
    }

    // 将条件表达式转换为 i1 类型
    condValue = CastInst::CreateZExtOrBitCast(condValue, llvm::Type::getInt1Ty(MyContext), "", context.currentBlock());

    // 获取当前函数
    Function* function = context.currentBlock()->getParent();

    // 创建 then, else 和 merge 基本块
    BasicBlock* thenBB = BasicBlock::Create(MyContext, "then", function);
    BasicBlock* elseBB = BasicBlock::Create(MyContext, "else");
    BasicBlock* mergeBB = BasicBlock::Create(MyContext, "ifcont");

    // 创建条件分支指令
    BranchInst::Create(thenBB, elseBB, condValue, context.currentBlock());

    // 生成 then 块代码
    context.pushBlock(thenBB);
    std::cout << "Generating code for then block" << std::endl;
    trueBlock.codeGen(context);
    if (!context.currentBlock()->getTerminator()) {
        BranchInst::Create(mergeBB, context.currentBlock());
    }
    context.popBlock();

    // 将 elseBB 添加到函数中
    function->getBasicBlockList().push_back(elseBB);
    context.pushBlock(elseBB);
    std::cout << "Generating code for else block" << std::endl;
    if (falseBlock) {
        falseBlock->codeGen(context);
    }
    if (!context.currentBlock()->getTerminator()) {
        BranchInst::Create(mergeBB, context.currentBlock());
    }
    context.popBlock();

    // 将 mergeBB 添加到函数中
    function->getBasicBlockList().push_back(mergeBB);
    context.pushBlock(mergeBB);
    std::cout << "Finished generating code for if statement" << std::endl;

    return mergeBB;
}
*/
/*
Value* NIfStatement::codeGen(CodeGenContext& context) {
    std::cout << "Generating code for if statement" << std::endl;

    // 生成条件表达式代码
    Value* condValue = condition.codeGen(context);
    if (!condValue) {
        std::cerr << "Condition code generation failed." << std::endl;
        return nullptr;
    }

    // 将条件表达式转换为 i1 类型
    condValue = CastInst::CreateZExtOrBitCast(condValue, llvm::Type::getInt1Ty(MyContext), "", context.currentBlock());

    // 获取当前函数
    Function* function = context.currentBlock()->getParent();

    // 创建 then, else 和 merge 基本块
    BasicBlock* thenBB = BasicBlock::Create(MyContext, "then", function);
    BasicBlock* elseBB = BasicBlock::Create(MyContext, "else");
    BasicBlock* mergeBB = BasicBlock::Create(MyContext, "ifcont");

    // 创建条件分支指令
    BranchInst::Create(thenBB, elseBB, condValue, context.currentBlock());

    // 生成 then 块代码
    context.pushBlock(thenBB);
    std::cout << "Generating code for then block" << std::endl;
    trueBlock.codeGen(context);
    if (!context.currentBlock()->getTerminator()) {
        BranchInst::Create(mergeBB, context.currentBlock());
    }
    context.popBlock();

    // 将 elseBB 添加到函数中
    function->getBasicBlockList().push_back(elseBB);
    context.pushBlock(elseBB);
    std::cout << "Generating code for else block" << std::endl;
    if (falseBlock) {
        falseBlock->codeGen(context);
    }
    if (!context.currentBlock()->getTerminator()) {
        BranchInst::Create(mergeBB, context.currentBlock());
    }
    context.popBlock();

    // 将 mergeBB 添加到函数中
    function->getBasicBlockList().push_back(mergeBB);
    if (!context.currentBlock()->getTerminator()) {
        BranchInst::Create(mergeBB, context.currentBlock());
    }
    context.pushBlock(mergeBB);
    std::cout << "Finished generating code for if statement" << std::endl;

    return mergeBB;
}
*/
Value* NIfStatement::codeGen(CodeGenContext& context) {
    std::cout << "Generating code for if statement" << std::endl;

    // 生成条件表达式代码
    Value* condValue = condition.codeGen(context);
    if (!condValue) {
        std::cerr << "Condition code generation failed." << std::endl;
        return nullptr;
    }

    // 将条件表达式转换为 i1 类型
    //condValue = CastInst::CreateZExtOrBitCast(condValue, llvm::Type::getInt1Ty(MyContext), "", context.currentBlock());

    // 获取当前函数
    Function* function = context.currentBlock()->getParent();

    // 创建 then, else 和 merge 基本块
    BasicBlock* thenBB = BasicBlock::Create(MyContext, "then", function);
    BasicBlock* elseBB = BasicBlock::Create(MyContext, "else");
    BasicBlock* mergeBB = BasicBlock::Create(MyContext, "ifcont");

    // 创建条件分支指令
    BranchInst::Create(thenBB, elseBB, condValue, context.currentBlock());

    // 生成 then 块代码
    context.pushBlock(thenBB);
    std::cout << "Generating code for then block" << std::endl;
    trueBlock.codeGen(context);
    if (!context.currentBlock()->getTerminator()) {
        BranchInst::Create(mergeBB, context.currentBlock());
    }
    context.popBlock();

    // 将 elseBB 添加到函数中
    function->getBasicBlockList().push_back(elseBB);
    context.pushBlock(elseBB);
    std::cout << "Generating code for else block" << std::endl;
    if (falseBlock) {
        falseBlock->codeGen(context);
    }
    if (!context.currentBlock()->getTerminator()) {
        BranchInst::Create(mergeBB, context.currentBlock());
    }
    context.popBlock();

    // 将 mergeBB 添加到函数中
    function->getBasicBlockList().push_back(mergeBB);
    context.pushBlock(mergeBB);
    std::cout << "Finished generating code for if statement" << std::endl;

    return nullptr;
}

Value* NWhileStatement::codeGen(CodeGenContext& context) {
    std::cout << "Generating code for while statement" << std::endl;

    Function* function = context.currentBlock()->getParent();

    // 创建基本块
    BasicBlock* condBB = BasicBlock::Create(MyContext, "whilecond", function);
    BasicBlock* bodyBB = BasicBlock::Create(MyContext, "whilebody");
    BasicBlock* afterBB = BasicBlock::Create(MyContext, "afterwhile");

    // 跳转到条件基本块
    BranchInst::Create(condBB, context.currentBlock());

    // 生成条件基本块代码
    context.pushBlock(condBB);
    Value* condValue = condition.codeGen(context);
    if (!condValue) {
        std::cerr << "Condition code generation failed." << std::endl;
        return nullptr;
    }

    // 将条件表达式转换为 i1 类型
    condValue = CastInst::CreateZExtOrBitCast(condValue, llvm::Type::getInt1Ty(MyContext), "", context.currentBlock());
    BranchInst::Create(bodyBB, afterBB, condValue, context.currentBlock());
    context.popBlock();

    // 生成循环体基本块代码
    function->getBasicBlockList().push_back(bodyBB);
    context.pushBlock(bodyBB);
    body.codeGen(context);
    if (!context.currentBlock()->getTerminator()) {
        BranchInst::Create(condBB, context.currentBlock());
    }
    context.popBlock();

    // 将 afterBB 添加到函数中
    function->getBasicBlockList().push_back(afterBB);
    context.pushBlock(afterBB);

    std::cout << "Finished generating code for while statement" << std::endl;
    return nullptr;
}


Value* NForStatement::codeGen(CodeGenContext& context) {
    llvm::Function* function = context.currentBlock()->getParent();

    // 创建基本块
    llvm::BasicBlock* preheaderBB = context.currentBlock();
    llvm::BasicBlock* condBB = llvm::BasicBlock::Create(MyContext, "forcond", function);
    llvm::BasicBlock* loopBB = llvm::BasicBlock::Create(MyContext, "forloop");
    llvm::BasicBlock* incBB = llvm::BasicBlock::Create(MyContext, "forinc");
    llvm::BasicBlock* afterBB = llvm::BasicBlock::Create(MyContext, "forafter");

    // 初始化部分
    if (init) {
        init->codeGen(context);
    }

    // 跳转到条件判断块
    llvm::BranchInst::Create(condBB, context.currentBlock());

    // 生成条件判断块
    context.pushBlock(condBB);
    llvm::Value* condValue = cond ? cond->codeGen(context) : llvm::ConstantInt::getTrue(MyContext);
    condValue = llvm::CastInst::CreateZExtOrBitCast(condValue, llvm::Type::getInt1Ty(MyContext), "", context.currentBlock());
    llvm::BranchInst::Create(loopBB, afterBB, condValue, context.currentBlock());
    context.popBlock();

    // 生成循环体块
    function->getBasicBlockList().push_back(loopBB);
    context.pushBlock(loopBB);
    body.codeGen(context);
    llvm::BranchInst::Create(incBB, context.currentBlock());
    context.popBlock();

    // 生成增量块
    function->getBasicBlockList().push_back(incBB);
    context.pushBlock(incBB);
    if (inc) {
        inc->codeGen(context);
    }
    llvm::BranchInst::Create(condBB, context.currentBlock());
    context.popBlock();

    // 生成结束块
    function->getBasicBlockList().push_back(afterBB);
    context.pushBlock(afterBB);

    return nullptr;
}

Value* NScanStatement::codeGen(CodeGenContext& context) {
    llvm::IRBuilder<> builder(context.currentBlock());

    // Declare scanf function if not already declared
    if (!context.scanfFn) {
        llvm::FunctionType* scanfType = llvm::FunctionType::get(
            llvm::Type::getInt32Ty(MyContext), 
            llvm::PointerType::get(llvm::Type::getInt8Ty(MyContext), 0), 
            true
        );
        context.scanfFn = llvm::Function::Create(scanfType, llvm::Function::ExternalLinkage, "scanf", context.module);
    }

    // Generate the format string for scanf
    llvm::Value* formatStr = builder.CreateGlobalStringPtr("%d");

    for (auto ident : identifiers) {
        if (context.locals().find(ident->name) == context.locals().end()) {
            std::cerr << "undeclared variable " << ident->name << std::endl;
            return nullptr;
        }

        llvm::Value* variable = context.locals()[ident->name];
        llvm::Value* ptr = builder.CreateBitCast(variable, llvm::Type::getInt32PtrTy(MyContext));
        builder.CreateCall(context.scanfFn, { formatStr, ptr });
    }

    return nullptr;
}
