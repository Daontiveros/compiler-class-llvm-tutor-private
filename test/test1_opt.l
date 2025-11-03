; ModuleID = '/home/diana/llvm-tutor/test/test1.ll'
source_filename = "/home/diana/llvm-tutor/test/test1.ll"

define i32 @foo(i32 %n) {
entry:
  %i = alloca i32, align 4
  store i32 0, ptr %i, align 4
  br label %loop

loop:                                             ; preds = %body, %entry
  %iv = load i32, ptr %i, align 4
  %cond = icmp slt i32 %iv, %n
  br i1 %cond, label %body, label %exit

body:                                             ; preds = %loop
  %x = add i32 2, 3
  %y = mul i32 %x, 10
  %iv.next = add i32 %iv, 1
  store i32 %iv.next, ptr %i, align 4
  br label %loop

exit:                                             ; preds = %loop
  ret i32 0
}
