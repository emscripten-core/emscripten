; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str2 = private constant [9 x i8] c"*%d,%d*\0A\00", align 1 ; [#uses=1]

; [#uses=0]
define i32 @main() {
entry:
  %retval = alloca i32, align 4                   ; [#uses=1 type=i32*]
  %mul7 = bitcast i32 -259741926 to i32
  %shl10 = shl i32 4014, 16
  %uadd1 = tail call { i32, i1 } @llvm.uadd.with.overflow.i32(i32 %mul7, i32 %shl10)
  %a0 = extractvalue { i32, i1 } %uadd1, 0
  %a1 = extractvalue { i32, i1 } %uadd1, 1
  %a2 = zext i1 %a1 to i32
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str2, i32 0, i32 0), i32 %a0, i32 %a2) ; [#uses=0]

  %buadd1prepre = tail call { i32, i1 } @llvm.uadd.with.overflow.i32(i32 %mul7, i32 %shl10)
  %buadd1pre = insertvalue { i32, i1 } %buadd1prepre, i1 0, 1
  %buadd1 = insertvalue { i32, i1 } %buadd1pre, i32 5177, 0
  %ba0 = extractvalue { i32, i1 } %buadd1, 0
  %ba1 = extractvalue { i32, i1 } %buadd1, 1
  %ba2 = zext i1 %ba1 to i32
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str2, i32 0, i32 0), i32 %ba0, i32 %ba2) ; [#uses=0]

  %z64buadd1pre = tail call { i64, i1 } @llvm.uadd.with.overflow.i64(i64 5000, i64 3000)
  %z64buadd1 = insertvalue { i64, i1 } %z64buadd1pre, i64 9875, 0
  %z64buadd2 = insertvalue { i64, i1 } %z64buadd1, i1 1, 1
  %z64ba0pre = extractvalue { i64, i1 } %z64buadd2, 0
  %z64ba0 = trunc i64 %z64ba0pre to i32
  %z64ba1 = extractvalue { i64, i1 } %z64buadd2, 1
  %z64ba2 = zext i1 %z64ba1 to i32
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str2, i32 0, i32 0), i32 %z64ba0, i32 %z64ba2) ; [#uses=0]

  %zbuadd1 = insertvalue { i32, i1 } { i32 undef, i1 false }, i32 10, 0 ; undef and explicit
  %zba0 = extractvalue { i32, i1 } %zbuadd1, 0
  %zba1 = extractvalue { i32, i1 } %zbuadd1, 1
  %zba2 = zext i1 %ba1 to i32
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str2, i32 0, i32 0), i32 %zba0, i32 %zba2) ; [#uses=0]

  ret i32 1
}

; [#uses=1]
declare i32 @printf(i8*, ...)

declare { i32, i1 } @llvm.uadd.with.overflow.i32(i32, i32) nounwind readnone
declare { i64, i1 } @llvm.uadd.with.overflow.i64(i64, i64) nounwind readnone

