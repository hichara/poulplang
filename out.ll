; ModuleID = 'main'

@.str0 = private constant [11 x i8] c"%d salut\0B\0D\00"
@.str1 = private constant [5 x i8] c"%d\0B\0D\00"

define void @main() {
entry:
  %0 = call i32 @test(i32 4)
  %1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([5 x i8]* @.str1, i32 0, i32 0), i32 %0)
  ret void
}

declare i32 @printf(i8*, ...)

declare void @putchar()

define i32 @test(i32 %a1) {
entry:
  %a = alloca i32
  store i32 %a1, i32* %a
  %0 = load i32* %a
  %1 = icmp slt i32 %0, 5
  br i1 %1, label %branch0, label %branch1
  ret i32 1

branch0:                                          ; preds = %entry
  %aa = alloca i32
  store i32 4, i32* %aa
  %2 = load i32* %aa
  %3 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([11 x i8]* @.str0, i32 0, i32 0), i32 %2)
  ret i32 2

branch1:                                          ; preds = %entry
  ret i32 3
}
