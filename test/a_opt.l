; ModuleID = '/home/diana/llvm-tutor/test/StaticCallCounter_Loop.ll'
source_filename = "/home/diana/llvm-tutor/test/StaticCallCounter_Loop.ll"

define i32 @foo(i32 %0) {
  br label %2

2:                                                ; preds = %2, %1
  %3 = phi i32 [ 0, %1 ], [ %7, %2 ]
  %4 = phi i32 [ %0, %1 ], [ %6, %2 ]
  %5 = tail call i32 @bar(i32 %4)
  %6 = add nsw i32 %5, %4
  %7 = add nuw nsw i32 %3, 1
  %8 = icmp eq i32 %7, 10
  br i1 %8, label %9, label %2

9:                                                ; preds = %2
  ret i32 %6
}

declare i32 @bar(i32)
