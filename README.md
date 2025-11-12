
# LLVM Assignment 1

## To Use

Clone the github:

```
https://github.com/Daontiveros/compiler-class-llvm-tutor-private.git
```

## Setup

[Ignore any warnings]

Go  to directory called build-SimpleLICM

Do the following commands:

```rm -rf *```

```cmake ../SimpleLICM ```

```make```

Go to directory called "build-ARDir"

Do the following commands:

```rm -rf *```

```cmake ../ARDir```

```make```


Go  to directory called build-DIVDir

Do the following commands:

```rm -rf *```

```cmake ../DIVdir ```

```make```



## How to Run:

[Your computer may use a different opt]

#### In "build-SimpleLICM" directory run:

```/usr/lib/llvm-21/bin/opt -load-pass-plugin=./libSimpleLICM.so -passes=simple-licm -S ~/compiler-class-llvm-tutor-private/test/test1.ll -o ~/compiler-class-llvm-tutor-private/test/test1_opt.l```

#### In "build-ARDir" directory run:

```/usr/lib/llvm-21/bin/opt -load-pass-plugin=./libAffineRecurrence.so -passes=affine-recurrence -S ~/compiler-class-llvm-tutor-private/test/test2.ll -o ~/compiler-class-llvm-tutor-private/test/test2_opt.ll```

```/usr/lib/llvm-21/bin/opt -load-pass-plugin=./libAffineRecurrence.so -passes=affine-recurrence -S ~/compiler-class-llvm-tutor-private/test/test3.ll -o ~/compiler-class-llvm-tutor-private/test/test3_opt.ll```


#### In "build-DIVDir" directory run:

```/usr/lib/llvm-21/bin/opt -load-pass-plugin=./libDerivedInductionVar.so -passes=induction-var-elim -S ~/compiler-class-llvm-tutor-private/test/test2.ll -o ~/compiler-class-llvm-tutor-private/test/test2_opt.ll```

```/usr/lib/llvm-21/bin/opt -load-pass-plugin=./libDerivedInductionVar.so -passes=induction-var-elim -S ~/compiler-class-llvm-tutor-private/test/test3.ll -o ~/compiler-class-llvm-tutor-private/test/test3_opt.ll```

```/usr/lib/llvm-21/bin/opt -load-pass-plugin=./libDerivedInductionVar.so -passes=induction-var-elim -S ~/compiler-class-llvm-tutor-private/test/test4.ll -o ~/compiler-class-llvm-tutor-private/test/test4_opt.ll```

## Inputs & Outputs:

### SimpleLICM-skeleton

Detects and hoists loop-invariant instructions.

#### Input (test1.ll):

```define i32 @foo(i32 %n) {
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
```

#### Output:

```lvm-tutor/test/test1_opt.l
SimpleLICM plugin: llvmGetPassPluginInfo() called
SimpleLICM plugin: getSimpleLICMPluginInfo() called
SimpleLICM pass is running!
Hoisting:   %x = add i32 2, 3
Hoisting:   %y = mul i32 %x, 10
```

### Affine Recurrence



#### Input (test2.ll):

```
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
```

#### Output

```
Analyzing loop in function main:
  Affine recurrence: i.val = {0,+,1}<for.cond>
  Affine recurrence: i.inc = {1,+,1}<for.cond>
```
#### Input (test3.ll):

```; ModuleID = 'test_loop_elim'
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
```

#### Output:

```Analyzing loop in function sum_n:
  Affine recurrence: i = {0,+,1}<loop>
  Affine recurrence: double_i = {0,+,2}<loop>
  Affine recurrence: inc = {1,+,1}<loop>
  Affine recurrence: inc2 = {2,+,2}<loop>
```

### DerivedInductionVar

Identifies induction variables in the inner loops of loop nests and eliminates the induction variables.

#### Input (test2.ll):

```
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
```

#### Output:

```
=== Running Induction Variable Elimination on: main ===
Loop depth 1 canonical IV: i.val
```

#### Output from changed file (test2_opt.ll)

```
; ModuleID = '/home/diana/llvm-tutor/test/test2.ll'
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
```


#### Input (test3.ll):

```; ModuleID = 'test_loop_elim'
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
```

#### Output:

```
=== Running Induction Variable Elimination on: sum_n ===
Loop depth 1 canonical IV: i
  Derived IV to eliminate: double_i -> {0,+,2}<nuw><nsw><%loop>
  Removed derived IV: double_i
```

#### Output from changed file (test3_opt.ll)

The induction variables were eliminated

```
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
```


#### Input (test4.ll):

```
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
```

#### Output:

```
=== Running Induction Variable Elimination on: sum_nested ===
Loop depth 1 canonical IV: i
Loop depth 2 canonical IV: j
  Derived IV to eliminate: double_j -> {0,+,2}<nuw><nsw><%inner_loop>
  Removed derived IV: double_j
```

#### Output from changed file (test4_opt.ll)

```
; ModuleID = '/home/diana/llvm-tutor/test/test4.ll'
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
```

# Dead Store Elimination Assignment

## Setup

##### Go to build-MSD
Do:
..
```
cmake ../MSD
```
```
make
```

##### Turn the c file examples into IR
..
```
clang -O0 -Xclang -disable-O0-optnone -S -emit-llvm ~/llvm-tutor/test/demo.c -o ~/llvm-tutor/test/demo.ll
```

```
clang -O0 -Xclang -disable-O0-optnone -S -emit-llvm ~/llvm-tutor/test/demo2.c -o ~/llvm-tutor/test/demo2.ll
```

```
clang -O0 -Xclang -disable-O0-optnone -S -emit-llvm ~/llvm-tutor/test/array_loop1.c -o ~/llvm-tutor/test/array_loop1.ll
```

```
clang -O0 -Xclang -disable-O0-optnone -S -emit-llvm ~/llvm-tutor/test/dse_test1.c -o ~/llvm-tutor/test/dse_test1.ll
```

##### Compile the MSD:
...

```
/usr/lib/llvm-21/bin/opt   -load-pass-plugin=./libMemorySSADemo.so   -passes=memssa-demo   -S ~/llvm-tutor/test/demo.ll   -o ~/llvm-tutor/test/demo_opt.ll 
```

```
/usr/lib/llvm-21/bin/opt   -load-pass-plugin=./libMemorySSADemo.so   -passes=memssa-demo   -S ~/llvm-tutor/test/demo2.ll   -o ~/llvm-tutor/test/demo2_opt.ll 
```

```
/usr/lib/llvm-21/bin/opt   -load-pass-plugin=./libMemorySSADemo.so   -passes=memssa-demo   -S ~/llvm-tutor/test/array_loop1.ll   -o ~/llvm-tutor/test/array_loop1_opt.ll 
```


##### Go to build-DSE 
Do:
...
```
cmake ../DSE
```
```
make
```
##### Compile the DSE:
...
```
/usr/lib/llvm-21/bin/opt   -load-pass-plugin=./libDeadStoreElimination.so   -passes=dead-store   -S ~/llvm-tutor/test/dse_test1.ll   -o ~/llvm-tutor/test/dse_test1_opt.ll 
```

Input:

```
// dead_store1.c
#include <stdio.h>

int main() {
    int x = 0;    // Store 1
    x = 5;        // Store 2 (overwrites Store 1, Store 1 is dead)
    printf("%d\n", x);
    return 0;
}
```

```
; ModuleID = '/home/daontiveros/llvm-tutor/test/dse_test1.c'
source_filename = "/home/daontiveros/llvm-tutor/test/dse_test1.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  store i32 0, ptr %2, align 4
  store i32 5, ptr %2, align 4
  %3 = load i32, ptr %2, align 4
  %4 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %3)
  ret i32 0
}

declare i32 @printf(ptr noundef, ...) #1

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Ubuntu clang version 21.1.5 (++20251023083201+45afac62e373-1~exp1~20251023083316.53)"}
```

Output:
```
; ModuleID = '/home/daontiveros/llvm-tutor/test/dse_test1.ll'
source_filename = "/home/daontiveros/llvm-tutor/test/dse_test1.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  store i32 5, ptr %2, align 4
  %3 = load i32, ptr %2, align 4
  %4 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %3)
  ret i32 0
}

declare i32 @printf(ptr noundef, ...) #1

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Ubuntu clang version 21.1.5 (++20251023083201+45afac62e373-1~exp1~20251023083316.53)"}
```

```
Analyzing function: main
Dead store detected at:   store i32 0, ptr %2, align 4
```

### What changed?
Dead Store was eliminated:
store i32 0, ptr %2, align 4






















