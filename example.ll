; ModuleID = 'main'
source_filename = "main"

@.str = private constant [4 x i8] c"%d\0A\00"
@.str.1 = private constant [4 x i8] c"%ld\00"

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
  %a = alloca i64, addrspace(4)
  store i64 0, i64 addrspace(4)* %a
  %a1 = load i64 addrspace(4)*, i64 addrspace(4)* %a
  call void @scan(i64 addrspace(4)* %a1)
  %a2 = load i64 addrspace(4)*, i64 addrspace(4)* %a
  call void @print(i64 addrspace(4)* %a2)
  ret void
}
