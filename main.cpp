#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include "codegen.h"
#include "node.h"

using namespace std;

extern int yyparse();
extern NBlock* programBlock;

void open_file(const char* filename) {
	// openfile
	freopen(filename, "r", stdin);
}

void createCoreFunctions(CodeGenContext& context);



int main(int argc, char **argv)
{
	if (argc > 1) {
		open_file(argv[1]);// parser xxx.txt（待编译的文件）
	}
	yyparse();
	cout << programBlock << endl;
	if (argc > 2) {
		freopen(argv[2], "r", stdin);
    }
    // see http://comments.gmane.org/gmane.comp.compilers.llvm.devel/33877
	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();
	InitializeNativeTargetAsmParser();
	CodeGenContext context;
	createCoreFunctions(context);
	context.generateCode(*programBlock);
	
	std::error_code EC;
    llvm::raw_fd_ostream outFile("example.ll", EC, llvm::sys::fs::OF_None);
    //context.module->print(outFile, nullptr);
    outFile.close();
	context.runCode();
	
	return 0;
}

