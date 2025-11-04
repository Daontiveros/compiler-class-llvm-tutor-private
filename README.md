
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

```cmake ../SimpleLICM ```

```make```

Go to directory called "build-ARDir"

Do the following commands:

```cmake ../ARDir```

```make```


Go  to directory called build-DIVDir

Do the following commands:

```cmake ../DIVdir ```

```make```



## How to Run:

[Your computer may use a different opt]

#### In "build-SimpleLICM" directory run:

```/usr/lib/llvm-21/bin/opt -load-pass-plugin=./libSimpleLICM.so -passes=simple-licm -S ~/llvm-tutor/test/test1.ll -o ~/llvm-tutor/test/test1_opt.l ```

#### In "build-ARDir" directory run:

```/usr/lib/llvm-21/bin/opt -load-pass-plugin=./libAffineRecurrence.so -passes=affine-recurrence -S ~/llvm-tutor/test/test2.ll -o ~/llvm-tutor/test/test2_opt.ll```

```/usr/lib/llvm-21/bin/opt -load-pass-plugin=./libAffineRecurrence.so -passes=affine-recurrence -S ~/llvm-tutor/test/test3.ll -o ~/llvm-tutor/test/test3_opt.ll```


#### In "build-DIVDir" directory run:

```/usr/lib/llvm-21/bin/opt -load-pass-plugin=./libDerivedInductionVar.so -passes=induction-var-elim -S ~/llvm-tutor/test/test2.ll -o ~/llvm-tutor/test/test2_opt.ll```

```/usr/lib/llvm-21/bin/opt -load-pass-plugin=./libDerivedInductionVar.so -passes=induction-var-elim -S ~/llvm-tutor/test/test3.ll -o ~/llvm-tutor/test/test3_opt.ll```

```/usr/lib/llvm-21/bin/opt -load-pass-plugin=./libDerivedInductionVar.so -passes=induction-var-elim -S ~/llvm-tutor/test/test4.ll -o ~/llvm-tutor/test/test4_opt.ll```

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









