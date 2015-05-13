; ModuleID = 'tests/hello_world.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str2 = private constant [9 x i8] c"*%d,%d*\0A\00", align 1 ; [#uses=1]

; [#uses=0]
define void @sub() {
entry:
  %retval = alloca i32, align 4                   ; [#uses=1 type=i32*]
  %mul7 = bitcast i32 -259741926 to i32
  %shl10 = shl i32 4014, 16
  %uadd1 = tail call { i32, i1 } @llvm.ssub.with.overflow.i32(i32 %mul7, i32 %shl10)
  %a0 = extractvalue { i32, i1 } %uadd1, 0
  %a1 = extractvalue { i32, i1 } %uadd1, 1
  %a2 = zext i1 %a1 to i32
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str2, i32 0, i32 0), i32 %a0, i32 %a2) ; [#uses=0]

  %buadd1prepre = tail call { i32, i1 } @llvm.ssub.with.overflow.i32(i32 %mul7, i32 %shl10)
  %buadd1pre = insertvalue { i32, i1 } %buadd1prepre, i1 0, 1
  %buadd1 = insertvalue { i32, i1 } %buadd1pre, i32 5177, 0
  %ba0 = extractvalue { i32, i1 } %buadd1, 0
  %ba1 = extractvalue { i32, i1 } %buadd1, 1
  %ba2 = zext i1 %ba1 to i32
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str2, i32 0, i32 0), i32 %ba0, i32 %ba2) ; [#uses=0]

  %z64buadd1pre = tail call { i64, i1 } @llvm.ssub.with.overflow.i64(i64 5000, i64 3000)
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

  ;

  %tauadd1 = tail call { i8, i1 } @llvm.ssub.with.overflow.i8(i8 127, i8 0)
  %ta0 = extractvalue { i8, i1 } %tauadd1, 0
  %ta1 = extractvalue { i8, i1 } %tauadd1, 1
  %ta2 = zext i1 %ta1 to i32
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str2, i32 0, i32 0), i32 0, i32 %ta2) ; [#uses=0]

  %tbuadd1 = tail call { i8, i1 } @llvm.ssub.with.overflow.i8(i8 127, i8 1)
  %tb0 = extractvalue { i8, i1 } %tbuadd1, 0
  %tb1 = extractvalue { i8, i1 } %tbuadd1, 1
  %tb2 = zext i1 %tb1 to i32
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str2, i32 0, i32 0), i32 0, i32 %tb2) ; [#uses=0]

  %tcuadd1 = tail call { i8, i1 } @llvm.ssub.with.overflow.i8(i8 -128, i8 0)
  %tc0 = extractvalue { i8, i1 } %tcuadd1, 0
  %tc1 = extractvalue { i8, i1 } %tcuadd1, 1
  %tc2 = zext i1 %tc1 to i32
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str2, i32 0, i32 0), i32 0, i32 %tc2) ; [#uses=0]

  %tduadd1 = tail call { i8, i1 } @llvm.ssub.with.overflow.i8(i8 -128, i8 -1)
  %td0 = extractvalue { i8, i1 } %tduadd1, 0
  %td1 = extractvalue { i8, i1 } %tduadd1, 1
  %td2 = zext i1 %td1 to i32
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str2, i32 0, i32 0), i32 0, i32 %td2) ; [#uses=0]

  %teuadd1 = tail call { i8, i1 } @llvm.ssub.with.overflow.i8(i8 0, i8 127)
  %te0 = extractvalue { i8, i1 } %teuadd1, 0
  %te1 = extractvalue { i8, i1 } %teuadd1, 1
  %te2 = zext i1 %te1 to i32
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str2, i32 0, i32 0), i32 0, i32 %te2) ; [#uses=0]

  %tfuadd1 = tail call { i8, i1 } @llvm.ssub.with.overflow.i8(i8 1, i8 127)
  %tf0 = extractvalue { i8, i1 } %tfuadd1, 0
  %tf1 = extractvalue { i8, i1 } %tfuadd1, 1
  %tf2 = zext i1 %tf1 to i32
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str2, i32 0, i32 0), i32 0, i32 %tf2) ; [#uses=0]

  %tguadd1 = tail call { i8, i1 } @llvm.ssub.with.overflow.i8(i8 0, i8 -128)
  %tg0 = extractvalue { i8, i1 } %tguadd1, 0
  %tg1 = extractvalue { i8, i1 } %tguadd1, 1
  %tg2 = zext i1 %tg1 to i32
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str2, i32 0, i32 0), i32 0, i32 %tg2) ; [#uses=0]

  %thuadd1 = tail call { i8, i1 } @llvm.ssub.with.overflow.i8(i8 -1, i8 -128)
  %th0 = extractvalue { i8, i1 } %thuadd1, 0
  %th1 = extractvalue { i8, i1 } %thuadd1, 1
  %th2 = zext i1 %th1 to i32
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str2, i32 0, i32 0), i32 0, i32 %th2) ; [#uses=0]

  %tiuadd1 = tail call { i8, i1 } @llvm.ssub.with.overflow.i8(i8 0, i8 -127)
  %ti0 = extractvalue { i8, i1 } %tiuadd1, 0
  %ti1 = extractvalue { i8, i1 } %tiuadd1, 1
  %ti2 = zext i1 %ti1 to i32
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str2, i32 0, i32 0), i32 0, i32 %ti2) ; [#uses=0]

  ret void
}

; [#uses=0]
define i32 @main() {
entry:
  %retval = alloca i32, align 4                   ; [#uses=1 type=i32*]
  %mul7 = bitcast i32 -259741926 to i32
  %shl10 = shl i32 4014, 16
  %uadd1 = tail call { i32, i1 } @llvm.sadd.with.overflow.i32(i32 %mul7, i32 %shl10)
  %a0 = extractvalue { i32, i1 } %uadd1, 0
  %a1 = extractvalue { i32, i1 } %uadd1, 1
  %a2 = zext i1 %a1 to i32
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str2, i32 0, i32 0), i32 %a0, i32 %a2) ; [#uses=0]

  %buadd1prepre = tail call { i32, i1 } @llvm.sadd.with.overflow.i32(i32 %mul7, i32 %shl10)
  %buadd1pre = insertvalue { i32, i1 } %buadd1prepre, i1 0, 1
  %buadd1 = insertvalue { i32, i1 } %buadd1pre, i32 5177, 0
  %ba0 = extractvalue { i32, i1 } %buadd1, 0
  %ba1 = extractvalue { i32, i1 } %buadd1, 1
  %ba2 = zext i1 %ba1 to i32
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str2, i32 0, i32 0), i32 %ba0, i32 %ba2) ; [#uses=0]

  %z64buadd1pre = tail call { i64, i1 } @llvm.sadd.with.overflow.i64(i64 5000, i64 3000)
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

  ;

  %tauadd1 = tail call { i8, i1 } @llvm.sadd.with.overflow.i8(i8 127, i8 0)
  %ta0 = extractvalue { i8, i1 } %tauadd1, 0
  %ta1 = extractvalue { i8, i1 } %tauadd1, 1
  %ta2 = zext i1 %ta1 to i32
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str2, i32 0, i32 0), i32 0, i32 %ta2) ; [#uses=0]

  %tbuadd1 = tail call { i8, i1 } @llvm.sadd.with.overflow.i8(i8 127, i8 1)
  %tb0 = extractvalue { i8, i1 } %tbuadd1, 0
  %tb1 = extractvalue { i8, i1 } %tbuadd1, 1
  %tb2 = zext i1 %tb1 to i32
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str2, i32 0, i32 0), i32 0, i32 %tb2) ; [#uses=0]

  %tcuadd1 = tail call { i8, i1 } @llvm.sadd.with.overflow.i8(i8 -128, i8 0)
  %tc0 = extractvalue { i8, i1 } %tcuadd1, 0
  %tc1 = extractvalue { i8, i1 } %tcuadd1, 1
  %tc2 = zext i1 %tc1 to i32
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str2, i32 0, i32 0), i32 0, i32 %tc2) ; [#uses=0]

  %tduadd1 = tail call { i8, i1 } @llvm.sadd.with.overflow.i8(i8 -128, i8 -1)
  %td0 = extractvalue { i8, i1 } %tduadd1, 0
  %td1 = extractvalue { i8, i1 } %tduadd1, 1
  %td2 = zext i1 %td1 to i32
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str2, i32 0, i32 0), i32 0, i32 %td2) ; [#uses=0]

  %teuadd1 = tail call { i8, i1 } @llvm.sadd.with.overflow.i8(i8 0, i8 127)
  %te0 = extractvalue { i8, i1 } %teuadd1, 0
  %te1 = extractvalue { i8, i1 } %teuadd1, 1
  %te2 = zext i1 %te1 to i32
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str2, i32 0, i32 0), i32 0, i32 %te2) ; [#uses=0]

  %tfuadd1 = tail call { i8, i1 } @llvm.sadd.with.overflow.i8(i8 1, i8 127)
  %tf0 = extractvalue { i8, i1 } %tfuadd1, 0
  %tf1 = extractvalue { i8, i1 } %tfuadd1, 1
  %tf2 = zext i1 %tf1 to i32
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str2, i32 0, i32 0), i32 0, i32 %tf2) ; [#uses=0]

  %tguadd1 = tail call { i8, i1 } @llvm.sadd.with.overflow.i8(i8 0, i8 -128)
  %tg0 = extractvalue { i8, i1 } %tguadd1, 0
  %tg1 = extractvalue { i8, i1 } %tguadd1, 1
  %tg2 = zext i1 %tg1 to i32
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str2, i32 0, i32 0), i32 0, i32 %tg2) ; [#uses=0]

  %thuadd1 = tail call { i8, i1 } @llvm.sadd.with.overflow.i8(i8 -1, i8 -128)
  %th0 = extractvalue { i8, i1 } %thuadd1, 0
  %th1 = extractvalue { i8, i1 } %thuadd1, 1
  %th2 = zext i1 %th1 to i32
  call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str2, i32 0, i32 0), i32 0, i32 %th2) ; [#uses=0]

  call void () @sub()

  ret i32 1
}

; [#uses=1]
declare i32 @printf(i8*, ...)

declare { i8, i1 } @llvm.sadd.with.overflow.i8(i8, i8) nounwind readnone
declare { i32, i1 } @llvm.sadd.with.overflow.i32(i32, i32) nounwind readnone
declare { i64, i1 } @llvm.sadd.with.overflow.i64(i64, i64) nounwind readnone

declare { i8, i1 } @llvm.ssub.with.overflow.i8(i8, i8) nounwind readnone
declare { i32, i1 } @llvm.ssub.with.overflow.i32(i32, i32) nounwind readnone
declare { i64, i1 } @llvm.ssub.with.overflow.i64(i64, i64) nounwind readnone

