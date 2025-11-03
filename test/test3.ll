; ModuleID = 'test_loop_elim'
source_filename = "test_loop_elim.c"

define i32 @sum_n(i32 %n) {
entry:
  %sum = alloca i32, align 4
  store i32 0, i32* %sum, align 4
  br label %loop

loop:                                             ; preds = %loop, %entry
  %i = phi i32 [ 0, %entry ], [ %inc, %loop ]        ; canonical IV
  %double_i = phi i32 [ 0, %entry ], [ %inc2, %loop ] ; derived IV

  %oldsum = load i32, i32* %sum, align 4
  %newsum = add nsw i32 %oldsum, %i
  store i32 %newsum, i32* %sum, align 4

  ; increments
  %inc = add nsw i32 %i, 1
  %inc2 = add nsw i32 %double_i, 2       ; derived IV increments by 2 each iteration

  %cmp = icmp slt i32 %inc, %n
  br i1 %cmp, label %loop, label %exit

exit:                                             ; preds = %loop
  %finalsum = load i32, i32* %sum, align 4
  ret i32 %finalsum
}













