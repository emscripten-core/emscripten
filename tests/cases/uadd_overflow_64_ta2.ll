; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str2 = private constant [11 x i8] c"*%llx,%d*\0A\00", align 1 ; [#uses=1]

; [#uses=0]
define i32 @main() {
entry:
  %uadd1 = tail call { i64, i1 } @llvm.uadd.with.overflow.i64(i64 18446744073709551606, i64 9999)
  %a0 = extractvalue { i64, i1 } %uadd1, 0
  %a1 = extractvalue { i64, i1 } %uadd1, 1
  %a2 = zext i1 %a1 to i32
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str2, i32 0, i32 0), i64 %a0, i32 %a2) ; [#uses=0]

  %uadd2 = tail call { i64, i1 } @llvm.uadd.with.overflow.i64(i64 18446744073709, i64 9999)
  %b0 = extractvalue { i64, i1 } %uadd2, 0
  %b1 = extractvalue { i64, i1 } %uadd2, 1
  %b2 = zext i1 %b1 to i32
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str2, i32 0, i32 0), i64 %b0, i32 %b2) ; [#uses=0]

  %uadd3 = tail call { i64, i1 } @llvm.uadd.with.overflow.i64(i64 4294967297, i64 18446744073709551615)
  %c0 = extractvalue { i64, i1 } %uadd3, 0
  %c1 = extractvalue { i64, i1 } %uadd3, 1
  %c2 = zext i1 %c1 to i32
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str2, i32 0, i32 0), i64 %c0, i32 %c2) ; [#uses=0]

  ret i32 1
}

; [#uses=1]
declare i32 @printf(i8*, ...)

declare { i32, i1 } @llvm.uadd.with.overflow.i32(i32, i32) nounwind readnone
declare { i64, i1 } @llvm.uadd.with.overflow.i64(i64, i64) nounwind readnone

