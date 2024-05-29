## 目录
- [L24cc](#l24cc)
  - [項目介紹](#項目介紹)
  - [開發工具和環境：](#開發工具和環境)
  - [L24語法概述](#l24語法概述)
  - [L24語句示例](#l24語句示例)
    - [1.賦值語句(包含變量聲明)](#1賦值語句包含變量聲明)
    - [2.條件跳轉語句](#2條件跳轉語句)
    - [3.循環語句](#3循環語句)
      - [while循環](#while循環)
      - [for循環(新增)](#for循環新增)
    - [4.輸入/輸出語句](#4輸入輸出語句)
    - [5.函數定義與調用](#5函數定義與調用)
      - [函數定義](#函數定義)
      - [函數調用](#函數調用)
    - [6.完整程序示例(test.l24)](#6完整程序示例testl24)
  - [使用方法説明](#使用方法説明)
    - [編譯編譯器](#編譯編譯器)
    - [運行編譯器](#運行編譯器)
    - [輸出](#輸出)
      - [1. L24語句-\>LLVM IR生成記錄](#1-l24語句-llvm-ir生成記錄)
      - [2. L24程序對應的LLVM IR](#2-l24程序對應的llvm-ir)
      - [3.程序運行狀態與輸出流](#3程序運行狀態與輸出流)
  - [編譯器設計説明](#編譯器設計説明)
    - [詞法分析](#詞法分析)
    - [語法分析](#語法分析)
    - [語義分析](#語義分析)
    - [中間代碼生成](#中間代碼生成)
  - [致謝](#致謝)

# L24cc
## 項目介紹
**L24cc**是一個用於L24編程語言的編譯器，旨在將L24代碼翻譯爲可執行的機器代碼。 \
該項目包含**詞法分析**(tokens.l)，**語法分析**(parser.y)，**語義分析**(node.h)和**代碼生成**(codegen.h/codegen.cpp)四個部分。

## 開發工具和環境：
編程語言：C++ \
開發工具：Flex，Bison，LLVM \
編譯器：Clang

## L24語法概述
L24語言的語法系統與C語言相近。 \
以下是L24語言基礎語法的EBNF(擴展巴科斯範式)表示:
```
<program> = "main" "{" <stmt_list> "}"
<stmt_list> = {<stmt> ";"}
<stmt> = <assign_stmt> | <if_stmt> | <while_stmt> | <scan_stmt> | <print_stmt>
<assign_stmt> = <ident> "=" <expr>
<if_stmt> = "if" "("<bool_expr>")" "then" "{"<stmt_list>"}" "end"
               | "if" "("<bool_expr>")" "then" "{"<stmt_list>"}" "else" "{"<stmt_list>"}" "end"
<while_stmt> = "while" "("<bool_expr>")" "{"<stmt_list>"}"
<scan_stmt> = "scan" "(" <ident> {"," <ident>} ")"
<print_stmt> = "print" "(" <expr> {"," <expr>} ")"
<bool_expr> = <expr> ("==" | "!=" | "<" | "<=" | ">" | ">=") <expr>
<expr> = ["+" | "-"] <term> {("+" | "-") <term>}
<term> = <factor> {("*" | "/") <factor>}
<factor> = <ident> | <number> | "("<expr>")"
```
## L24語句示例
### 1.賦值語句(包含變量聲明)
```
int x = 114514; 
int y = 1919810;
int inm = x + y;
```

### 2.條件跳轉語句
```
int x = 100;
if (x > 0) then {
    print(x);
} end

int y = 100;
if (x < y) then {
    print(x);
} else {
    print(y);
} end
```

### 3.循環語句

#### while循環
```
while (x < 10) {
    print(x);
    x = x + 1;
}
```

#### for循環(新增)
```
for (int i = 0; i < 10; i = i++) {
    print(i);
}

```

### 4.輸入/輸出語句
```
scan(x, y);
print(x, y);
```
注意： \
輸入流的内容要提前存入文件input.txt中 \
Makefile后, \
輸入：
```
./parser test.l24 input.txt
```
便能將input.txt中的内容導入輸入流中。
### 5.函數定義與調用
#### 函數定義
```
int add(int a, int b) {
    return a + b;
}
```

#### 函數調用
```
int add(int a, int b) {
    return a + b;
}
```

### 6.完整程序示例(test.l24)
```
main {
    int add(int a, int b){
        return a+b;
    }

    int x = 10;
    int y = 20;
    
    if (x > y) then {
        print(x);
    } else {
        print(y);
    } end

    while (x > 0) {
        x = x - 1;
    }

    scan(x, y);

    int sum = add(x, y);
    print(sum);
}
```

## 使用方法説明
要使用L24cc編譯器，請按照以下步驟操作。 
### 編譯編譯器
在src文件夾下打開cmd \
輸入：
```
make
```

### 運行編譯器
```
./parser test.l24
```

### 輸出
以"test.l24"為例，得到結果：
#### 1. L24語句->LLVM IR生成記錄
```
Generating code...
Generating code for 20NFunctionDeclaration
Creating variable declaration int a
Creating variable declaration int b
Generating code for 16NReturnStatement
Generating return code for 15NBinaryOperator
Creating binary operation   275
Creating identifier reference: a
Creating identifier reference: b
Creating block
Creating function: add
Generating code for 20NVariableDeclaration
Creating variable declaration int x
Creating integer: 10
Generating code for 20NVariableDeclaration
Creating variable declaration int y
Creating integer: 20
Generating code for 12NIfStatement
Generating code for if statement
Creating binary operation   265
Creating identifier reference: x
Creating identifier reference: y
Generating code for then block
Generating code for 20NExpressionStatement
Generating code for 11NMethodCall
Creating identifier reference: x
Creating method call: print
Creating block
Generating code for else block
Generating code for 20NExpressionStatement
Generating code for 11NMethodCall
Creating identifier reference: y
Creating method call: print
Creating block
Finished generating code for if statement
Generating code for 15NWhileStatement
Generating code for while statement
Creating binary operation   265
Creating identifier reference: x
Creating integer: 0
Generating code for 20NExpressionStatement
Generating code for 11NAssignment
Creating binary operation   276
Creating identifier reference: x
Creating integer: 1
Creating block
Finished generating code for while statement
Generating code for 14NScanStatement
Generating code for 20NVariableDeclaration
Creating variable declaration int sum
Creating identifier reference: x
Creating identifier reference: y
Creating method call: add
Generating code for 20NExpressionStatement
Generating code for 11NMethodCall
Creating identifier reference: sum
Creating method call: print
Creating block
Code is generated.
LLVM IR printed.
```

#### 2. L24程序對應的LLVM IR
```
; ModuleID = 'main'
source_filename = "main"

@.str = private constant [4 x i8] c"%d\0A\00"
@.str.1 = private constant [4 x i8] c"%ld\00"
@0 = private unnamed_addr constant [3 x i8] c"%d\00", align 1

declare i32 @printf(i8*, ...)

define internal void @print(i64 %toPrint) {
entry:
  %0 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i64 %toPrint)
  ret void
}

declare i32 @scanf(i8*, ...)

define internal void @scan(i64** %toScan) {
entry:
  %0 = call i32 (i8*, ...) @scanf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.1, i32 0, i32 0), i64** %toScan)
  ret void
}

define internal void @main() {
entry:
  %x = alloca i64, addrspace(4)
  store i64 10, i64 addrspace(4)* %x
  %y = alloca i64, addrspace(4)
  store i64 20, i64 addrspace(4)* %y
  %x1 = load i64 addrspace(4)*, i64 addrspace(4)* %x
  %y2 = load i64 addrspace(4)*, i64 addrspace(4)* %y
  %0 = icmp sgt i64 addrspace(4)* %x1, %y2
  br i1 %0, label %then, label %else

then:                                             ; preds = %entry
  %x3 = load i64 addrspace(4)*, i64 addrspace(4)* %x
  call void @print(i64 addrspace(4)* %x3)
  br label %ifcont

else:                                             ; preds = %entry
  %y4 = load i64 addrspace(4)*, i64 addrspace(4)* %y
  call void @print(i64 addrspace(4)* %y4)
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  br label %whilecond

whilecond:                                        ; preds = %whilebody, %ifcont
  %x5 = load i64 addrspace(4)*, i64 addrspace(4)* %x
  %1 = icmp sgt i64 addrspace(4)* %x5, i64 0
  %2 = bitcast i1 %1 to i1
  br i1 %2, label %whilebody, label %afterwhile

whilebody:                                        ; preds = %whilecond
  %x6 = load i64 addrspace(4)*, i64 addrspace(4)* %x
  %3 = sub i64 addrspace(4)* %x6, i64 1
  store i64 addrspace(4)* %3, i64 addrspace(4)* %x
  br label %whilecond

afterwhile:                                       ; preds = %whilecond
  %4 = bitcast i64 addrspace(4)* %x to i32*
  %5 = call i32 (i8*, ...) @scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i32* %4)
  %6 = bitcast i64 addrspace(4)* %y to i32*
  %7 = call i32 (i8*, ...) @scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i32* %6)
  %sum = alloca i64, addrspace(4)
  %x7 = load i64 addrspace(4)*, i64 addrspace(4)* %x
  %y8 = load i64 addrspace(4)*, i64 addrspace(4)* %y
  %8 = call i64 @add(i64 addrspace(4)* %x7, i64 addrspace(4)* %y8)
  store i64 %8, i64 addrspace(4)* %sum
  %sum9 = load i64 addrspace(4)*, i64 addrspace(4)* %sum
  call void @print(i64 addrspace(4)* %sum9)
  ret void
}

define internal i64 @add(i64 %a1, i64 %b2) {
entry:
  %a = alloca i64, addrspace(4)
  store i64 %a1, i64 addrspace(4)* %a
  %b = alloca i64, addrspace(4)
  store i64 %b2, i64 addrspace(4)* %b
  %a3 = load i64 addrspace(4)*, i64 addrspace(4)* %a
  %b4 = load i64 addrspace(4)*, i64 addrspace(4)* %b
  %0 = add i64 addrspace(4)* %a3, %b4
  ret i64 addrspace(4)* %0
}
```

#### 3.程序運行狀態與輸出流
```
Running code...
20
20
Code was run.
```


## 編譯器設計説明

### 詞法分析
L24cc采用**Flex**工具完成詞法分析。 \
**tokens.l**文件中預設了關鍵字，數字，合法標識符名，運算符的**正則表達式**。 \
例如：
```
[ \t\n]					        ;
[a-zA-Z_][a-zA-Z0-9_]*  SAVE_TOKEN; return TIDENTIFIER;
"="						          return TOKEN(TEQUAL);
"main"                          return TOKEN(TMAIN);
```
以上四條語句分別實現了： \
```
[ \t\n]					        ;
```
忽略程序中語句閒的所有空白字符。
```
[a-zA-Z_][a-zA-Z0-9_]*  SAVE_TOKEN; return TIDENTIFIER;
```
接受並返回一個合法的標識符名稱，并返回token TIDENTIFIER。
```
"="						          return TOKEN(TEQUAL);
```
接受運算符"="，并返回token TEQUAL。
```
"main"                          return TOKEN(TMAIN);
```
接受關鍵字"main"，并返回token TMAIN。

### 語法分析
L24cc采用**Bison**工具完成語法分析。 \
**parser.y**文件中定義了各語法的**語法規則**與**語義動作**。 \
同時也定義了各種運算符的優先級。 \
以 if_stmt 和 while_stmt 產生式箭頭右側共有的stmt_list 爲例：
```
stmt_list : stmt 
		{ 
			$$ = new NBlock();
			$$->statements.push_back($<stmt>1);
			$$->locals = currentBlock->locals;
			currentBlock = $$;
		}
          | stmt_list stmt { $1->statements.push_back($<stmt>2); }
          ;
```
該語法規則與語義動作實現了：
新建作用域，同時繼承主函數作用域的本地變量。使if語句與while語句能夠使用大括號外的變量。

### 語義分析
**node.h**中定義了各類型語句對應的抽象語法樹(Abstract Syntax Tree)節點。 \
parser在語法分析的過程中創建出分析樹，並添加各個節點的信息。

### 中間代碼生成
本項目采用**LLVM**進行中間代碼生成。 \
根據node.h中的節點類定義，在codegen.cpp中編寫各個節點類的codegen函數，用於生成中間代碼 \
經歷語法分析與語義分析后，AST的根節點保存在全局變量programBlock中 \
編寫中間代碼生成函數generateCode如下：
```
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
```
在主程序main.cpp中調用該函數，傳入AST根節點：
```
context.generateCode(*programBlock);
```
LLVM的中間代碼優化模塊在優化中間代碼后，將中間代碼轉換爲機器代碼。 \
最終得到機器代碼parser文件。

## 致謝
在编写 L24cc 编译器的过程中，我参考了lsegal的[my_toy_compiler](https://github.com/lsegal/my_toy_compiler)與s0duku的[Fibol](https://github.com/s0duku/Fibol)项目。在此特别感谢這些项目的开源者，感谢他們提供了宝贵的参考和启示。
