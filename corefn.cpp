#include <iostream>
#include "codegen.h"
#include "node.h"

using namespace std;

extern int yyparse();
extern NBlock* programBlock;

llvm::Function* createPrintfFunction(CodeGenContext& context)
{
    std::vector<llvm::Type*> printf_arg_types;
    printf_arg_types.push_back(llvm::Type::getInt8PtrTy(MyContext)); //char*

    std::cout << "printf" << std::endl;

    llvm::FunctionType* printf_type =
        llvm::FunctionType::get(
            llvm::Type::getInt32Ty(MyContext), printf_arg_types, true);

    llvm::Function *func = llvm::Function::Create(
                printf_type, llvm::Function::ExternalLinkage,
                llvm::Twine("printf"),
                context.module
           );
    func->setCallingConv(llvm::CallingConv::C);
    return func;
}

void createEchoFunction(CodeGenContext& context, llvm::Function* printfFn)
{
    std::vector<llvm::Type*> echo_arg_types;
    echo_arg_types.push_back(llvm::Type::getInt64Ty(MyContext));

    llvm::FunctionType* echo_type =
        llvm::FunctionType::get(
            llvm::Type::getVoidTy(MyContext), echo_arg_types, false);

    llvm::Function *func = llvm::Function::Create(
                echo_type, llvm::Function::InternalLinkage,
                llvm::Twine("print"),
                context.module
           );
    llvm::BasicBlock *bblock = llvm::BasicBlock::Create(MyContext, "entry", func, 0);
	context.pushBlock(bblock);
    
    const char *constValue = "%d\n";
    llvm::Constant *format_const = llvm::ConstantDataArray::getString(MyContext, constValue);
    llvm::GlobalVariable *var =
        new llvm::GlobalVariable(
            *context.module, llvm::ArrayType::get(llvm::IntegerType::get(MyContext, 8), strlen(constValue)+1),
            true, llvm::GlobalValue::PrivateLinkage, format_const, ".str");
    llvm::Constant *zero =
        llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(MyContext));

    std::vector<llvm::Constant*> indices;
    indices.push_back(zero);
    indices.push_back(zero);
    llvm::Constant *var_ref = llvm::ConstantExpr::getGetElementPtr(
	llvm::ArrayType::get(llvm::IntegerType::get(MyContext, 8), strlen(constValue)+1),
        var, indices);

    std::vector<Value*> args;
    args.push_back(var_ref);

    Function::arg_iterator argsValues = func->arg_begin();
    Value* toPrint = &*argsValues++;
    toPrint->setName("toPrint");
    args.push_back(toPrint);
    
	CallInst *call = CallInst::Create(printfFn, makeArrayRef(args), "", bblock);
	ReturnInst::Create(MyContext, bblock);
	context.popBlock();
}


// 创建scanf函数的LLVM IR表示
llvm::Function* createScanfFunction(CodeGenContext& context) {
    std::vector<llvm::Type*> scanf_arg_types;
    scanf_arg_types.push_back(llvm::Type::getInt8PtrTy(MyContext)); // char*

    llvm::FunctionType* scanf_type = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(MyContext), scanf_arg_types, true);

    llvm::Function *func = llvm::Function::Create(
                scanf_type, llvm::Function::ExternalLinkage,
                llvm::Twine("scanf"),
                context.module
           );
    func->setCallingConv(llvm::CallingConv::C);
    return func;
}

// 创建scan函数，该函数将调用scanf函数来读取一个整数值
void createScanFunction(CodeGenContext& context, llvm::Function* scanfFn) {
    // 定义scan函数的参数类型，这里包含一个64位整数指针
    std::vector<llvm::Type*> scan_arg_types;
    scan_arg_types.push_back(llvm::Type::getInt64PtrTy(MyContext)->getPointerTo());

    // 定义scan函数的函数类型，返回类型为void，参数类型为64位整数指针
    llvm::FunctionType* scan_type =
        llvm::FunctionType::get(
            llvm::Type::getVoidTy(MyContext), scan_arg_types, false);

    // 创建scan函数，内部链接类型
    llvm::Function *func = llvm::Function::Create(
                scan_type, llvm::Function::InternalLinkage,
                llvm::Twine("scan"),
                context.module
           );

    // 创建基本块，函数体将插入这个基本块中
    llvm::BasicBlock *bblock = llvm::BasicBlock::Create(MyContext, "entry", func, 0);
    context.pushBlock(bblock);
    
    // 定义格式化字符串"%d"并存储为全局变量
    const char *constValue = "%ld";
    llvm::Constant *format_const = llvm::ConstantDataArray::getString(MyContext, constValue);
    llvm::GlobalVariable *var =
        new llvm::GlobalVariable(
            *context.module, llvm::ArrayType::get(llvm::IntegerType::get(MyContext, 8), strlen(constValue)+1),
            true, llvm::GlobalValue::PrivateLinkage, format_const, ".str");

    // 获取格式化字符串的地址
    llvm::Constant *zero = llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(MyContext));
    std::vector<llvm::Constant*> indices;
    indices.push_back(zero);
    indices.push_back(zero);
    llvm::Constant *var_ref = llvm::ConstantExpr::getGetElementPtr(
        llvm::ArrayType::get(llvm::IntegerType::get(MyContext, 8), strlen(constValue)+1),
        var, indices);

    // 设置scanf函数的参数
    std::vector<Value*> args;
    args.push_back(var_ref);

    // 获取scan函数的参数，即要读取的整数指针
    Function::arg_iterator argsValues = func->arg_begin();
    Value* toScan = &*argsValues++;
    toScan->setName("toScan");
    args.push_back(toScan);
    
    // 创建对scanf函数的调用
    CallInst *call = CallInst::Create(scanfFn, makeArrayRef(args), "", bblock);

    // 返回void
    ReturnInst::Create(MyContext, bblock);
    context.popBlock();
}

// 创建核心函数，包括printf和scanf
void createCoreFunctions(CodeGenContext& context) {
    llvm::Function* printfFn = createPrintfFunction(context);
    context.printfFn = printfFn; // 设置printfFn
    createEchoFunction(context, printfFn);

    llvm::Function* scanfFn = createScanfFunction(context);
    context.scanfFn = scanfFn; // 设置scanfFn
    createScanFunction(context, scanfFn);
}