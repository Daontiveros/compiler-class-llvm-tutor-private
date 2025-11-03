define i32 @foo(i32 %n) {
entry:
  %i = alloca i32
  store i32 0, ptr %i
  br label %loop

loop:
  %iv = load i32, ptr %i
  %cond = icmp slt i32 %iv, %n
  br i1 %cond, label %body, label %exit

body:
  %x = add i32 2, 3
  %y = mul i32 %x, 10
  %iv.next = add i32 %iv, 1
  store i32 %iv.next, ptr %i
  br label %loop

exit:
  ret i32 0
}







