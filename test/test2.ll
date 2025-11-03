; ModuleID = 'test2.c'
source_filename = "test2.c"

define i32 @main() {
entry:
  %sum = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, i32* %sum, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                          ; preds = %entry, %for.body
  %i.val = phi i32 [ 0, %entry ], [ %i.inc, %for.body ]
  %cmp = icmp slt i32 %i.val, 10
  br i1 %cmp, label %for.body, label %for.end

for.body:                                          ; preds = %for.cond
  %i.inc = add i32 %i.val, 1
  store i32 %i.inc, i32* %i, align 4
  %3 = load i32, i32* %sum, align 4
  %4 = add i32 %3, %i.val
  store i32 %4, i32* %sum, align 4
  br label %for.cond

for.end:                                           ; preds = %for.cond
  %5 = load i32, i32* %sum, align 4
  ret i32 %5
}


