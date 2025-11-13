; ModuleID = '/home/daontiveros/compiler-class-llvm-tutor-private/test/test2.ll'
source_filename = "test2.c"

define i32 @main() {
entry:
  %sum = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, ptr %sum, align 4
  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %i.val = phi i32 [ 0, %entry ], [ %i.inc, %for.body ]
  %cmp = icmp slt i32 %i.val, 10
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %i.inc = add i32 %i.val, 1
  store i32 %i.inc, ptr %i, align 4
  %0 = load i32, ptr %sum, align 4
  %1 = add i32 %0, %i.val
  store i32 %1, ptr %sum, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %2 = load i32, ptr %sum, align 4
  ret i32 %2
}
