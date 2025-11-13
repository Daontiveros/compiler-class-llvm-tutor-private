; ModuleID = '/home/daontiveros/compiler-class-llvm-tutor-private/test/test4.ll'
source_filename = "nested_loop.c"

define i32 @sum_nested(i32 %n, i32 %m) {
entry:
  %sum = alloca i32, align 4
  store i32 0, ptr %sum, align 4
  br label %outer_loop

outer_loop:                                       ; preds = %outer_loop_inc, %entry
  %i = phi i32 [ 0, %entry ], [ %inc_i, %outer_loop_inc ]
  br label %inner_loop

inner_loop:                                       ; preds = %inner_loop_inc, %outer_loop
  %j = phi i32 [ 0, %outer_loop ], [ %inc_j, %inner_loop_inc ]
  %0 = shl nuw i32 %j, 1
  %oldsum = load i32, ptr %sum, align 4
  %newsum = add nsw i32 %oldsum, %i
  %newsum2 = add nsw i32 %newsum, %j
  store i32 %newsum2, ptr %sum, align 4
  %inc_j = add nsw i32 %j, 1
  %inc2_j = add nsw i32 %0, 2
  %cmp_j = icmp slt i32 %inc_j, %m
  br i1 %cmp_j, label %inner_loop_inc, label %outer_loop_inc

inner_loop_inc:                                   ; preds = %inner_loop
  br label %inner_loop

outer_loop_inc:                                   ; preds = %inner_loop
  %inc_i = add nsw i32 %i, 1
  %cmp_i = icmp slt i32 %inc_i, %n
  br i1 %cmp_i, label %outer_loop, label %exit

exit:                                             ; preds = %outer_loop_inc
  %finalsum = load i32, ptr %sum, align 4
  ret i32 %finalsum
}
