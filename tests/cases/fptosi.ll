; ModuleID = '/dev/shm/tmp/src.cpp.o'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [8 x i8] c"*%.3f*\0A\00", align 1 ; [#uses=1 type=[8 x i8]*]
@.str2 = private unnamed_addr constant [6 x i8] c"*%d*\0A\00", align 1 ; [#uses=1 type=[6 x i8]*]

; [#uses=0]
define i32 @main() {
entry:
  %f = fadd float 1.000, 0.500
  %d = fadd double 3.333, 0.444
  %fd = fpext float %f to double
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), double %fd) ; [#uses=0 type=i32]
  %call2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), double %d) ; [#uses=0 type=i32]
  %fs = fptosi float %f to i64
  %fu = fptoui float %f to i64
  %ds = fptosi double %d to i64
  %du = fptoui double %d to i64
  %call3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str2, i32 0, i32 0), i64 %fs) ; [#uses=0 type=i32]
  %call4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str2, i32 0, i32 0), i64 %fu) ; [#uses=0 type=i32]
  %call5 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str2, i32 0, i32 0), i64 %ds) ; [#uses=0 type=i32]
  %call6 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str2, i32 0, i32 0), i64 %du) ; [#uses=0 type=i32]
  ret i32 1
}

; [#uses=1]
declare i32 @printf(i8*, ...)
