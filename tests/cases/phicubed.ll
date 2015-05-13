; ModuleID = '/dev/shm/tmp/src.cpp.o'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

%struct.worker_args = type { i32, %struct.worker_args* }

@.str = private unnamed_addr constant [9 x i8] c"*%d,%d*\0A\00", align 1 ; [#uses=1 type=[9 x i8]*]
@.emscripten.autodebug.str = private constant [10 x i8] c"AD:%d,%d\0A\00", align 1 ; [#uses=1 type=[10 x i8]*]
@.emscripten.autodebug.str.f = private constant [11 x i8] c"AD:%d,%lf\0A\00", align 1 ; [#uses=1 type=[11 x i8]*]

; [#uses=0]
define i32 @main() nounwind {
entry:
  %a = alloca %struct.worker_args, align 4        ; [#uses=3 type=%struct.worker_args*]
  %b = alloca %struct.worker_args, align 4        ; [#uses=4 type=%struct.worker_args*]
  %chunk = alloca [10 x %struct.worker_args], align 4 ; [#uses=30 type=[10 x %struct.worker_args]*]
  %value = getelementptr inbounds %struct.worker_args, %struct.worker_args* %a, i32 0, i32 0  store i32 60, i32* %value, align 4  call void @emscripten_autodebug_i32(i32 16, i32 60)
  %next = getelementptr inbounds %struct.worker_args, %struct.worker_args* %a, i32 0, i32 1  store %struct.worker_args* %b, %struct.worker_args** %next, align 4  %value1 = getelementptr inbounds %struct.worker_args, %struct.worker_args* %b, i32 0, i32 0  store i32 900, i32* %value1, align 4  call void @emscripten_autodebug_i32(i32 26, i32 900)
  %next2 = getelementptr inbounds %struct.worker_args, %struct.worker_args* %b, i32 0, i32 1  store %struct.worker_args* null, %struct.worker_args** %next2, align 4  br label %while.body
for.cond.preheader:                               ; preds = %while.body
  %value5 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 0, i32 0  store i32 0, i32* %value5, align 4  call void @emscripten_autodebug_i32(i32 36, i32 0)
  %arrayidx7 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 1  %next9 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 0, i32 1  store %struct.worker_args* %arrayidx7, %struct.worker_args** %next9, align 4  %value5.1 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 1, i32 0  store i32 10, i32* %value5.1, align 4  call void @emscripten_autodebug_i32(i32 43, i32 10)
  %arrayidx7.1 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 2  %next9.1 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 1, i32 1  store %struct.worker_args* %arrayidx7.1, %struct.worker_args** %next9.1, align 4  %value5.2 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 2, i32 0  store i32 20, i32* %value5.2, align 4  call void @emscripten_autodebug_i32(i32 50, i32 20)
  %arrayidx7.2 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 3  %next9.2 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 2, i32 1  store %struct.worker_args* %arrayidx7.2, %struct.worker_args** %next9.2, align 4  %value5.3 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 3, i32 0  store i32 30, i32* %value5.3, align 4  call void @emscripten_autodebug_i32(i32 57, i32 30)
  %arrayidx7.3 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 4  %next9.3 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 3, i32 1  store %struct.worker_args* %arrayidx7.3, %struct.worker_args** %next9.3, align 4  %value5.4 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 4, i32 0  store i32 40, i32* %value5.4, align 4  call void @emscripten_autodebug_i32(i32 64, i32 40)
  %arrayidx7.4 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 5  %next9.4 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 4, i32 1  store %struct.worker_args* %arrayidx7.4, %struct.worker_args** %next9.4, align 4  %value5.5 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 5, i32 0  store i32 50, i32* %value5.5, align 4  call void @emscripten_autodebug_i32(i32 71, i32 50)
  %arrayidx7.5 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 6  %next9.5 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 5, i32 1  store %struct.worker_args* %arrayidx7.5, %struct.worker_args** %next9.5, align 4  %value5.6 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 6, i32 0  store i32 60, i32* %value5.6, align 4  call void @emscripten_autodebug_i32(i32 78, i32 60)
  %arrayidx7.6 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 7  %next9.6 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 6, i32 1  store %struct.worker_args* %arrayidx7.6, %struct.worker_args** %next9.6, align 4  %value5.7 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 7, i32 0  store i32 70, i32* %value5.7, align 4  call void @emscripten_autodebug_i32(i32 85, i32 70)
  %arrayidx7.7 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 8  %next9.7 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 7, i32 1  store %struct.worker_args* %arrayidx7.7, %struct.worker_args** %next9.7, align 4  %value5.8 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 8, i32 0  store i32 80, i32* %value5.8, align 4  call void @emscripten_autodebug_i32(i32 92, i32 80)
  %arrayidx7.8 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 9  %next9.8 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 8, i32 1  store %struct.worker_args* %arrayidx7.8, %struct.worker_args** %next9.8, align 4  %value11 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 9, i32 0  store i32 90, i32* %value11, align 4  call void @emscripten_autodebug_i32(i32 99, i32 90)
  %arrayidx12 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 0  %next14 = getelementptr inbounds [10 x %struct.worker_args], [10 x %struct.worker_args]* %chunk, i32 0, i32 9, i32 1  store %struct.worker_args* %arrayidx12, %struct.worker_args** %next14, align 4  br label %do.body
while.body:                                       ; preds = %while.body.while.body_crit_edge, %entry
  %0 = phi %struct.worker_args* [ %b, %entry ], [ %.pre, %while.body.while.body_crit_edge ] ; [#uses=3 type=%struct.worker_args*]
  %c.03 = phi %struct.worker_args* [ %a, %entry ], [ %0, %while.body.while.body_crit_edge ] ; [#uses=1 type=%struct.worker_args*]
  %total.02 = phi i32 [ 0, %entry ], [ %add, %while.body.while.body_crit_edge ] ; [#uses=1 type=i32]
  %value3 = getelementptr inbounds %struct.worker_args, %struct.worker_args* %c.03, i32 0, i32 0  %1 = load i32, i32* %value3, align 4  call void @emscripten_autodebug_i32(i32 112, i32 %1)
  %add = add nsw i32 %1, %total.02  %tobool = icmp eq %struct.worker_args* %0, null  br i1 %tobool, label %for.cond.preheader, label %while.body.while.body_crit_edge
while.body.while.body_crit_edge:                  ; preds = %while.body
  %next4.phi.trans.insert = getelementptr inbounds %struct.worker_args, %struct.worker_args* %0, i32 0, i32 1 ; [#uses=1 type=%struct.worker_args**]
  %.pre = load %struct.worker_args*, %struct.worker_args** %next4.phi.trans.insert, align 4  br label %while.body
do.body:                                          ; preds = %do.body, %for.cond.preheader
  %total.1 = phi i32 [ %add, %for.cond.preheader ], [ %add16, %do.body ] ; [#uses=1 type=i32]
  %c.1 = phi %struct.worker_args* [ %arrayidx12, %for.cond.preheader ], [ %3, %do.body ] ; [#uses=2 type=%struct.worker_args*]
  %value15 = getelementptr inbounds %struct.worker_args, %struct.worker_args* %c.1, i32 0, i32 0  %2 = load i32, i32* %value15, align 4  call void @emscripten_autodebug_i32(i32 129, i32 %2)
  %add16 = add nsw i32 %2, %total.1  %next17 = getelementptr inbounds %struct.worker_args, %struct.worker_args* %c.1, i32 0, i32 1  %3 = load %struct.worker_args*, %struct.worker_args** %next17, align 4  %cmp19 = icmp eq %struct.worker_args* %3, %arrayidx12  br i1 %cmp19, label %do.end, label %do.body
do.end:                                           ; preds = %do.body
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i32 0, i32 0), i32 %add16, %struct.worker_args* null)  ret i32 0}

; [#uses=7]
declare i32 @printf(i8* nocapture, ...) nounwind

; [#uses=0]
define void @emscripten_autodebug_i64(i32 %line, i64 %value) {
entry:
  %0 = sitofp i64 %value to double                ; [#uses=1 type=double]
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.emscripten.autodebug.str.f, i32 0, i32 0), i32 %line, double %0) ; [#uses=0 type=i32]
  br label %return

return:                                           ; preds = %entry
  ret void
}

; [#uses=14]
define void @emscripten_autodebug_i32(i32 %line, i32 %value) {
entry:
  %0 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.emscripten.autodebug.str, i32 0, i32 0), i32 %line, i32 %value) ; [#uses=0 type=i32]
  br label %return

return:                                           ; preds = %entry
  ret void
}

; [#uses=0]
define void @emscripten_autodebug_i16(i32 %line, i16 %value) {
entry:
  %0 = zext i16 %value to i32                     ; [#uses=1 type=i32]
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.emscripten.autodebug.str, i32 0, i32 0), i32 %line, i32 %0) ; [#uses=0 type=i32]
  br label %return

return:                                           ; preds = %entry
  ret void
}

; [#uses=0]
define void @emscripten_autodebug_i8(i32 %line, i8 %value) {
entry:
  %0 = zext i8 %value to i32                      ; [#uses=1 type=i32]
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.emscripten.autodebug.str, i32 0, i32 0), i32 %line, i32 %0) ; [#uses=0 type=i32]
  br label %return

return:                                           ; preds = %entry
  ret void
}

; [#uses=0]
define void @emscripten_autodebug_float(i32 %line, float %value) {
entry:
  %0 = fpext float %value to double               ; [#uses=1 type=double]
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.emscripten.autodebug.str.f, i32 0, i32 0), i32 %line, double %0) ; [#uses=0 type=i32]
  br label %return

return:                                           ; preds = %entry
  ret void
}

; [#uses=0]
define void @emscripten_autodebug_double(i32 %line, double %value) {
entry:
  %0 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.emscripten.autodebug.str.f, i32 0, i32 0), i32 %line, double %value) ; [#uses=0 type=i32]
  br label %return

return:                                           ; preds = %entry
  ret void
}


