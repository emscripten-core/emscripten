
; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S128"
target triple = "i386-pc-linux-gnu"

@.str = private unnamed_addr constant [15 x i8] c"hello, world!\0A\00", align 1 ; [#uses=1 type=[15 x i8]*]

; [#uses=0]
define i32 @main() {
entry:
  br label %zero

zero:
  %.3.ph.i757 = phi i8* [ getelementptr ([15 x i8]* @.str, i32 0, i32 add (i32 xor (i32 ptrtoint (i8* getelementptr ([15 x i8]* @.str, i32 0, i32 select (i1 icmp ugt (i32 sub (i32 0, i32 add (i32 ptrtoint ([15 x i8]* @.str to i32), i32 25)), i32 xor (i32 ptrtoint ([15 x i8]* @.str to i32), i32 -1)), i32 sub (i32 0, i32 add (i32 ptrtoint ([15 x i8]* @.str to i32), i32 25)), i32 xor (i32 ptrtoint ([15 x i8]* @.str to i32), i32 -1))) to i32), i32 -1), i32 25)), %entry ], [ getelementptr ([15 x i8]* @.str, i32 0, i32 ptrtoint (i8* getelementptr ([15 x i8]* @.str, i32 0, i32 add (i32 sub (i32 0, i32 ptrtoint ([15 x i8]* @.str to i32)), i32 25)) to i32)), %other ]

  %retval = alloca i32, align 4                   ; [#uses=1 type=i32*]
  store i32 0, i32* %retval
  %call = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([15 x i8]* @.str, i32 0, i32 0)) ; [#uses=0 type=i32]
  br label %other

other:
  br i1 0, label %zero, label %last

last:
  ret i32 1
}

declare i32 @printf(i8*, ...)

