; ModuleID = '/home/diana/llvm-tutor/test/hello.ll'
source_filename = "/home/diana/llvm-tutor/test/hello.ll"

define i32 @foo(i32 %0) {
  %2 = shl nsw i32 %0, 1
  ret i32 %2
}

define i32 @bar(i32 %0, i32 %1) {
  %3 = shl i32 %1, 1
  %4 = add nsw i32 %3, %0
  ret i32 %4
}

define i32 @fez(i32 %0, i32 %1, i32 %2) {
  %4 = shl i32 %1, 1
  %5 = add nsw i32 %4, %0
  %6 = mul nsw i32 %2, 3
  %7 = add nsw i32 %5, %6
  ret i32 %7
}

define i32 @main(i32 %0, ptr readnone captures(none) %1) {
  %3 = tail call i32 @foo(i32 1)
  %4 = tail call i32 @bar(i32 1, i32 11)
  %5 = add nsw i32 %4, %3
  %6 = tail call i32 @fez(i32 1, i32 11, i32 111)
  %7 = add nsw i32 %5, %6
  ret i32 %7
}
