; ModuleID = '/home/diana/llvm-tutor/test/test3.ll'
source_filename = "test_loop_elim.c"

define i32 @sum_n(i32 %n) {
entry:
  %sum = alloca i32, align 4
  store i32 0, ptr %sum, align 4
  br label %loop

loop:                                             ; preds = %loop, %entry
  %i = phi i32 [ 0, %entry ], [ %inc, %loop ]
  %0 = shl nuw i32 %i, 1
  %oldsum = load i32, ptr %sum, align 4
  %newsum = add nsw i32 %oldsum, %i
  store i32 %newsum, ptr %sum, align 4
  %inc = add nsw i32 %i, 1
  %inc2 = add nsw i32 %0, 2
  %cmp = icmp slt i32 %inc, %n
  br i1 %cmp, label %loop, label %exit

exit:                                             ; preds = %loop
  %finalsum = load i32, ptr %sum, align 4
  ret i32 %finalsum
}
