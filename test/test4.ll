; ModuleID = 'nested_loop_example'
source_filename = "nested_loop.c"

define i32 @sum_nested(i32 %n, i32 %m) {
entry:
  %sum = alloca i32, align 4
  store i32 0, i32* %sum, align 4
  br label %outer_loop

; Outer loop
outer_loop:
  %i = phi i32 [0, %entry], [%inc_i, %outer_loop_inc]
  br label %inner_loop

; Inner loop
inner_loop:
  %j = phi i32 [0, %outer_loop], [%inc_j, %inner_loop_inc]       ; canonical IV
  %double_j = phi i32 [0, %outer_loop], [%inc2_j, %inner_loop_inc] ; derived IV (2x)
  
  ; sum += i + j
  %oldsum = load i32, i32* %sum, align 4
  %newsum = add nsw i32 %oldsum, %i
  %newsum2 = add nsw i32 %newsum, %j
  store i32 %newsum2, i32* %sum, align 4
  
  ; increment IVs
  %inc_j = add nsw i32 %j, 1
  %inc2_j = add nsw i32 %double_j, 2

  ; inner loop condition
  %cmp_j = icmp slt i32 %inc_j, %m
  br i1 %cmp_j, label %inner_loop_inc, label %outer_loop_inc

inner_loop_inc:
  br label %inner_loop

outer_loop_inc:
  %inc_i = add nsw i32 %i, 1
  %cmp_i = icmp slt i32 %inc_i, %n
  br i1 %cmp_i, label %outer_loop, label %exit

exit:
  %finalsum = load i32, i32* %sum, align 4
  ret i32 %finalsum
}

