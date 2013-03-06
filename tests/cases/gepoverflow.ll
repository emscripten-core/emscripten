; ModuleID = 'src.cpp.o'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i386-pc-linux-gnu"

; XXX Note: the .txt file contains raw pointer data! The important thing is that the two pointers
;     on the first line are of the same scale. If one is huge and the other not, then we failed.
;     That is an overflow, which overflow checks should catch.
;     If we change memory stuff - like the default stack! - we will need to change this.

@.str2 = private constant [14 x i8] c"hello, world!\00", align 1 ; [#uses=0]
@.str = private constant [9 x i8] c"*%d,%d*\0A\00", align 1 ; [#uses=1]
@base = internal constant [31 x i16] [i16 3, i16 4, i16 5, i16 6, i16 7, i16 8, i16 9, i16 10, i16 11, i16 13, i16 15, i16 17, i16 19, i16 23, i16 27, i16 31, i16 35, i16 43, i16 51, i16 59, i16 67, i16 83, i16 99, i16 115, i16 131, i16 163, i16 195, i16 227, i16 258, i16 0, i16 0], align 32 ; [#uses=2]

; [#uses=2]
declare i32 @printf(i8* noalias, ...) nounwind

; [#uses=0]
define i32 @main() {
entry:
  %retval = alloca i32                            ; [#uses=1]
  %"alloca point" = bitcast i32 0 to i32          ; [#uses=0]
  %baseint = ptrtoint [31 x i16]* @base to i32    ; [#uses=1]
  %0 = getelementptr [31 x i16]* @base, i32 69273657, i32 24 ; [#uses=1]
  %1 = ptrtoint i16* %0 to i32                    ; [#uses=2]
  %2 = add i32 %1, 570                            ; [#uses=2]
  %3 = sub i32 %1, %baseint                       ; [#uses=1]
  %4 = sub i32 %2, %baseint                       ; [#uses=1]
  %5 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([9 x i8]* @.str, i32 0, i32 0), i32 %1, i32 %2) ; [#uses=0]
  %6 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([9 x i8]* @.str, i32 0, i32 0), i32 %3, i32 %4) ; [#uses=0]
  br label %return

return:                                           ; preds = %entry
  store i32 0, i32* %retval
  %retval1 = load i32* %retval                    ; [#uses=1]
  ret i32 %retval1
}

