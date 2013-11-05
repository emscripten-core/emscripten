; ModuleID = '/dev/shm/tmp/src.cpp.o'

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
  %value = getelementptr inbounds %struct.worker_args* %a, i32 0, i32 0, !dbg !26 ; [#uses=1 type=i32*] [debug line = 11:13]
  store i32 60, i32* %value, align 4, !dbg !26    ; [debug line = 11:13]
  call void @emscripten_autodebug_i32(i32 16, i32 60)
  %next = getelementptr inbounds %struct.worker_args* %a, i32 0, i32 1, !dbg !27 ; [#uses=1 type=%struct.worker_args**] [debug line = 12:13]
  store %struct.worker_args* %b, %struct.worker_args** %next, align 4, !dbg !27 ; [debug line = 12:13]
  %value1 = getelementptr inbounds %struct.worker_args* %b, i32 0, i32 0, !dbg !28 ; [#uses=1 type=i32*] [debug line = 13:13]
  store i32 900, i32* %value1, align 4, !dbg !28  ; [debug line = 13:13]
  call void @emscripten_autodebug_i32(i32 26, i32 900)
  %next2 = getelementptr inbounds %struct.worker_args* %b, i32 0, i32 1, !dbg !29 ; [#uses=1 type=%struct.worker_args**] [debug line = 14:13]
  store %struct.worker_args* null, %struct.worker_args** %next2, align 4, !dbg !29 ; [debug line = 14:13]
  br label %while.body, !dbg !34                  ; [debug line = 17:13]

for.cond.preheader:                               ; preds = %while.body
  %value5 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 0, i32 0, !dbg !35 ; [#uses=1 type=i32*] [debug line = 25:15]
  store i32 0, i32* %value5, align 4, !dbg !35    ; [debug line = 25:15]
  call void @emscripten_autodebug_i32(i32 36, i32 0)
  %arrayidx7 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 1, !dbg !38 ; [#uses=1 type=%struct.worker_args*] [debug line = 26:15]
  %next9 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 0, i32 1, !dbg !38 ; [#uses=1 type=%struct.worker_args**] [debug line = 26:15]
  store %struct.worker_args* %arrayidx7, %struct.worker_args** %next9, align 4, !dbg !38 ; [debug line = 26:15]
  %value5.1 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 1, i32 0, !dbg !35 ; [#uses=1 type=i32*] [debug line = 25:15]
  store i32 10, i32* %value5.1, align 4, !dbg !35 ; [debug line = 25:15]
  call void @emscripten_autodebug_i32(i32 43, i32 10)
  %arrayidx7.1 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 2, !dbg !38 ; [#uses=1 type=%struct.worker_args*] [debug line = 26:15]
  %next9.1 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 1, i32 1, !dbg !38 ; [#uses=1 type=%struct.worker_args**] [debug line = 26:15]
  store %struct.worker_args* %arrayidx7.1, %struct.worker_args** %next9.1, align 4, !dbg !38 ; [debug line = 26:15]
  %value5.2 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 2, i32 0, !dbg !35 ; [#uses=1 type=i32*] [debug line = 25:15]
  store i32 20, i32* %value5.2, align 4, !dbg !35 ; [debug line = 25:15]
  call void @emscripten_autodebug_i32(i32 50, i32 20)
  %arrayidx7.2 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 3, !dbg !38 ; [#uses=1 type=%struct.worker_args*] [debug line = 26:15]
  %next9.2 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 2, i32 1, !dbg !38 ; [#uses=1 type=%struct.worker_args**] [debug line = 26:15]
  store %struct.worker_args* %arrayidx7.2, %struct.worker_args** %next9.2, align 4, !dbg !38 ; [debug line = 26:15]
  %value5.3 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 3, i32 0, !dbg !35 ; [#uses=1 type=i32*] [debug line = 25:15]
  store i32 30, i32* %value5.3, align 4, !dbg !35 ; [debug line = 25:15]
  call void @emscripten_autodebug_i32(i32 57, i32 30)
  %arrayidx7.3 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 4, !dbg !38 ; [#uses=1 type=%struct.worker_args*] [debug line = 26:15]
  %next9.3 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 3, i32 1, !dbg !38 ; [#uses=1 type=%struct.worker_args**] [debug line = 26:15]
  store %struct.worker_args* %arrayidx7.3, %struct.worker_args** %next9.3, align 4, !dbg !38 ; [debug line = 26:15]
  %value5.4 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 4, i32 0, !dbg !35 ; [#uses=1 type=i32*] [debug line = 25:15]
  store i32 40, i32* %value5.4, align 4, !dbg !35 ; [debug line = 25:15]
  call void @emscripten_autodebug_i32(i32 64, i32 40)
  %arrayidx7.4 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 5, !dbg !38 ; [#uses=1 type=%struct.worker_args*] [debug line = 26:15]
  %next9.4 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 4, i32 1, !dbg !38 ; [#uses=1 type=%struct.worker_args**] [debug line = 26:15]
  store %struct.worker_args* %arrayidx7.4, %struct.worker_args** %next9.4, align 4, !dbg !38 ; [debug line = 26:15]
  %value5.5 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 5, i32 0, !dbg !35 ; [#uses=1 type=i32*] [debug line = 25:15]
  store i32 50, i32* %value5.5, align 4, !dbg !35 ; [debug line = 25:15]
  call void @emscripten_autodebug_i32(i32 71, i32 50)
  %arrayidx7.5 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 6, !dbg !38 ; [#uses=1 type=%struct.worker_args*] [debug line = 26:15]
  %next9.5 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 5, i32 1, !dbg !38 ; [#uses=1 type=%struct.worker_args**] [debug line = 26:15]
  store %struct.worker_args* %arrayidx7.5, %struct.worker_args** %next9.5, align 4, !dbg !38 ; [debug line = 26:15]
  %value5.6 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 6, i32 0, !dbg !35 ; [#uses=1 type=i32*] [debug line = 25:15]
  store i32 60, i32* %value5.6, align 4, !dbg !35 ; [debug line = 25:15]
  call void @emscripten_autodebug_i32(i32 78, i32 60)
  %arrayidx7.6 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 7, !dbg !38 ; [#uses=1 type=%struct.worker_args*] [debug line = 26:15]
  %next9.6 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 6, i32 1, !dbg !38 ; [#uses=1 type=%struct.worker_args**] [debug line = 26:15]
  store %struct.worker_args* %arrayidx7.6, %struct.worker_args** %next9.6, align 4, !dbg !38 ; [debug line = 26:15]
  %value5.7 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 7, i32 0, !dbg !35 ; [#uses=1 type=i32*] [debug line = 25:15]
  store i32 70, i32* %value5.7, align 4, !dbg !35 ; [debug line = 25:15]
  call void @emscripten_autodebug_i32(i32 85, i32 70)
  %arrayidx7.7 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 8, !dbg !38 ; [#uses=1 type=%struct.worker_args*] [debug line = 26:15]
  %next9.7 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 7, i32 1, !dbg !38 ; [#uses=1 type=%struct.worker_args**] [debug line = 26:15]
  store %struct.worker_args* %arrayidx7.7, %struct.worker_args** %next9.7, align 4, !dbg !38 ; [debug line = 26:15]
  %value5.8 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 8, i32 0, !dbg !35 ; [#uses=1 type=i32*] [debug line = 25:15]
  store i32 80, i32* %value5.8, align 4, !dbg !35 ; [debug line = 25:15]
  call void @emscripten_autodebug_i32(i32 92, i32 80)
  %arrayidx7.8 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 9, !dbg !38 ; [#uses=1 type=%struct.worker_args*] [debug line = 26:15]
  %next9.8 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 8, i32 1, !dbg !38 ; [#uses=1 type=%struct.worker_args**] [debug line = 26:15]
  store %struct.worker_args* %arrayidx7.8, %struct.worker_args** %next9.8, align 4, !dbg !38 ; [debug line = 26:15]
  %value11 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 9, i32 0, !dbg !42 ; [#uses=1 type=i32*] [debug line = 28:13]
  store i32 90, i32* %value11, align 4, !dbg !42  ; [debug line = 28:13]
  call void @emscripten_autodebug_i32(i32 99, i32 90)
  %arrayidx12 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 0, !dbg !43 ; [#uses=3 type=%struct.worker_args*] [debug line = 29:13]
  %next14 = getelementptr inbounds [10 x %struct.worker_args]* %chunk, i32 0, i32 9, i32 1, !dbg !43 ; [#uses=1 type=%struct.worker_args**] [debug line = 29:13]
  store %struct.worker_args* %arrayidx12, %struct.worker_args** %next14, align 4, !dbg !43 ; [debug line = 29:13]
  br label %do.body, !dbg !45                     ; [debug line = 32:13]

while.body:                                       ; preds = %while.body.while.body_crit_edge, %entry
  %0 = phi %struct.worker_args* [ %b, %entry ], [ %.pre, %while.body.while.body_crit_edge ] ; [#uses=3 type=%struct.worker_args*]
  %c.03 = phi %struct.worker_args* [ %a, %entry ], [ %0, %while.body.while.body_crit_edge ] ; [#uses=1 type=%struct.worker_args*]
  %total.02 = phi i32 [ 0, %entry ], [ %add, %while.body.while.body_crit_edge ] ; [#uses=1 type=i32]
  %value3 = getelementptr inbounds %struct.worker_args* %c.03, i32 0, i32 0, !dbg !46 ; [#uses=1 type=i32*] [debug line = 18:15]
  %1 = load i32* %value3, align 4, !dbg !46       ; [#uses=2 type=i32] [debug line = 18:15]
  call void @emscripten_autodebug_i32(i32 112, i32 %1)
  %add = add nsw i32 %1, %total.02, !dbg !46      ; [#uses=2 type=i32] [debug line = 18:15]
  %tobool = icmp eq %struct.worker_args* %0, null, !dbg !34 ; [#uses=1 type=i1] [debug line = 17:13]
  br i1 %tobool, label %for.cond.preheader, label %while.body.while.body_crit_edge, !dbg !34 ; [debug line = 17:13]

while.body.while.body_crit_edge:                  ; preds = %while.body
  %next4.phi.trans.insert = getelementptr inbounds %struct.worker_args* %0, i32 0, i32 1 ; [#uses=1 type=%struct.worker_args**]
  %.pre = load %struct.worker_args** %next4.phi.trans.insert, align 4, !dbg !48 ; [#uses=1 type=%struct.worker_args*] [debug line = 19:15]
  br label %while.body, !dbg !34                  ; [debug line = 17:13]

do.body:                                          ; preds = %do.body, %for.cond.preheader
  %total.1 = phi i32 [ %add, %for.cond.preheader ], [ %add16, %do.body ] ; [#uses=1 type=i32]
  %c.1 = phi %struct.worker_args* [ %arrayidx12, %for.cond.preheader ], [ %3, %do.body ] ; [#uses=2 type=%struct.worker_args*]
  %value15 = getelementptr inbounds %struct.worker_args* %c.1, i32 0, i32 0, !dbg !49 ; [#uses=1 type=i32*] [debug line = 33:15]
  %2 = load i32* %value15, align 4, !dbg !49      ; [#uses=2 type=i32] [debug line = 33:15]
  call void @emscripten_autodebug_i32(i32 129, i32 %2)
  %add16 = add nsw i32 %2, %total.1, !dbg !49     ; [#uses=2 type=i32] [debug line = 33:15]
  %next17 = getelementptr inbounds %struct.worker_args* %c.1, i32 0, i32 1, !dbg !51 ; [#uses=1 type=%struct.worker_args**] [debug line = 34:15]
  %3 = load %struct.worker_args** %next17, align 4, !dbg !51 ; [#uses=2 type=%struct.worker_args*] [debug line = 34:15]
  %cmp19 = icmp eq %struct.worker_args* %3, %arrayidx12, !dbg !52 ; [#uses=1 type=i1] [debug line = 35:13]
  br i1 %cmp19, label %do.end, label %do.body, !dbg !52 ; [debug line = 35:13]

do.end:                                           ; preds = %do.body
  %call = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([9 x i8]* @.str, i32 0, i32 0), i32 %add16, %struct.worker_args* null), !dbg !53 ; [#uses=0 type=i32] [debug line = 37:13]
  ret i32 0, !dbg !54                             ; [debug line = 40:13]
}

; [#uses=7]
declare i32 @printf(i8* nocapture, ...) nounwind

; [#uses=0]
define void @emscripten_autodebug_i64(i32 %line, i64 %value) {
entry:
  %0 = sitofp i64 %value to double                ; [#uses=1 type=double]
  %1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([11 x i8]* @.emscripten.autodebug.str.f, i32 0, i32 0), i32 %line, double %0) ; [#uses=0 type=i32]
  br label %return

return:                                           ; preds = %entry
  ret void
}

; [#uses=14]
define void @emscripten_autodebug_i32(i32 %line, i32 %value) {
entry:
  %0 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([10 x i8]* @.emscripten.autodebug.str, i32 0, i32 0), i32 %line, i32 %value) ; [#uses=0 type=i32]
  br label %return

return:                                           ; preds = %entry
  ret void
}

; [#uses=0]
define void @emscripten_autodebug_i16(i32 %line, i16 %value) {
entry:
  %0 = zext i16 %value to i32                     ; [#uses=1 type=i32]
  %1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([10 x i8]* @.emscripten.autodebug.str, i32 0, i32 0), i32 %line, i32 %0) ; [#uses=0 type=i32]
  br label %return

return:                                           ; preds = %entry
  ret void
}

; [#uses=0]
define void @emscripten_autodebug_i8(i32 %line, i8 %value) {
entry:
  %0 = zext i8 %value to i32                      ; [#uses=1 type=i32]
  %1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([10 x i8]* @.emscripten.autodebug.str, i32 0, i32 0), i32 %line, i32 %0) ; [#uses=0 type=i32]
  br label %return

return:                                           ; preds = %entry
  ret void
}

; [#uses=0]
define void @emscripten_autodebug_float(i32 %line, float %value) {
entry:
  %0 = fpext float %value to double               ; [#uses=1 type=double]
  %1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([11 x i8]* @.emscripten.autodebug.str.f, i32 0, i32 0), i32 %line, double %0) ; [#uses=0 type=i32]
  br label %return

return:                                           ; preds = %entry
  ret void
}

; [#uses=0]
define void @emscripten_autodebug_double(i32 %line, double %value) {
entry:
  %0 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([11 x i8]* @.emscripten.autodebug.str.f, i32 0, i32 0), i32 %line, double %value) ; [#uses=0 type=i32]
  br label %return

return:                                           ; preds = %entry
  ret void
}

!llvm.dbg.cu = !{!0}

!0 = metadata !{i32 720913, i32 0, i32 4, metadata !"/dev/shm/tmp/src.cpp", metadata !"/run/shm/tmp", metadata !"clang version 3.0 (trunk 141881)", i1 true, i1 false, metadata !"", i32 0, metadata !1, metadata !1, metadata !3, metadata !1} ; [ DW_TAG_compile_unit ]
!1 = metadata !{metadata !2}
!2 = metadata !{i32 0}
!3 = metadata !{metadata !4}
!4 = metadata !{metadata !5}
!5 = metadata !{i32 720942, i32 0, metadata !6, metadata !"main", metadata !"main", metadata !"", metadata !6, i32 8, metadata !7, i1 false, i1 true, i32 0, i32 0, i32 0, i32 256, i1 false, i32 ()* @main, null, null, metadata !10} ; [ DW_TAG_subprogram ]
!6 = metadata !{i32 720937, metadata !"/dev/shm/tmp/src.cpp", metadata !"/run/shm/tmp", null} ; [ DW_TAG_file_type ]
!7 = metadata !{i32 720917, i32 0, metadata !"", i32 0, i32 0, i64 0, i64 0, i32 0, i32 0, i32 0, metadata !8, i32 0, i32 0} ; [ DW_TAG_subroutine_type ]
!8 = metadata !{metadata !9}
!9 = metadata !{i32 720932, null, metadata !"int", null, i32 0, i64 32, i64 32, i64 0, i32 0, i32 5} ; [ DW_TAG_base_type ]
!10 = metadata !{metadata !11}
!11 = metadata !{i32 720932}                      ; [ DW_TAG_base_type ]
!12 = metadata !{i32 721152, metadata !13, metadata !"a", metadata !6, i32 9, metadata !14, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!13 = metadata !{i32 720907, metadata !5, i32 8, i32 11, metadata !6, i32 0} ; [ DW_TAG_lexical_block ]
!14 = metadata !{i32 720898, null, metadata !"worker_args", metadata !6, i32 3, i64 64, i64 32, i32 0, i32 0, null, metadata !15, i32 0, null, null} ; [ DW_TAG_class_type ]
!15 = metadata !{metadata !16, metadata !17, metadata !19}
!16 = metadata !{i32 720909, metadata !14, metadata !"value", metadata !6, i32 4, i64 32, i64 32, i64 0, i32 0, metadata !9} ; [ DW_TAG_member ]
!17 = metadata !{i32 720909, metadata !14, metadata !"next", metadata !6, i32 5, i64 32, i64 32, i64 32, i32 0, metadata !18} ; [ DW_TAG_member ]
!18 = metadata !{i32 720911, null, metadata !"", null, i32 0, i64 32, i64 32, i64 0, i32 0, metadata !14} ; [ DW_TAG_pointer_type ]
!19 = metadata !{i32 720942, i32 0, metadata !14, metadata !"worker_args", metadata !"worker_args", metadata !"", metadata !6, i32 3, metadata !20, i1 false, i1 false, i32 0, i32 0, null, i32 320, i1 false, null, null, i32 0, metadata !10} ; [ DW_TAG_subprogram ]
!20 = metadata !{i32 720917, i32 0, metadata !"", i32 0, i32 0, i64 0, i64 0, i32 0, i32 0, i32 0, metadata !21, i32 0, i32 0} ; [ DW_TAG_subroutine_type ]
!21 = metadata !{null, metadata !22}
!22 = metadata !{i32 720911, i32 0, metadata !"", i32 0, i32 0, i64 32, i64 32, i64 0, i32 64, metadata !14} ; [ DW_TAG_pointer_type ]
!23 = metadata !{i32 9, i32 25, metadata !13, null}
!24 = metadata !{i32 721152, metadata !13, metadata !"b", metadata !6, i32 10, metadata !14, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!25 = metadata !{i32 10, i32 25, metadata !13, null}
!26 = metadata !{i32 11, i32 13, metadata !13, null}
!27 = metadata !{i32 12, i32 13, metadata !13, null}
!28 = metadata !{i32 13, i32 13, metadata !13, null}
!29 = metadata !{i32 14, i32 13, metadata !13, null}
!30 = metadata !{i32 721152, metadata !13, metadata !"c", metadata !6, i32 15, metadata !22, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!31 = metadata !{i32 15, i32 32, metadata !13, null}
!32 = metadata !{i32 721152, metadata !13, metadata !"total", metadata !6, i32 16, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!33 = metadata !{i32 16, i32 26, metadata !13, null}
!34 = metadata !{i32 17, i32 13, metadata !13, null}
!35 = metadata !{i32 25, i32 15, metadata !36, null}
!36 = metadata !{i32 720907, metadata !37, i32 24, i32 41, metadata !6, i32 3} ; [ DW_TAG_lexical_block ]
!37 = metadata !{i32 720907, metadata !13, i32 24, i32 13, metadata !6, i32 2} ; [ DW_TAG_lexical_block ]
!38 = metadata !{i32 26, i32 15, metadata !36, null}
!39 = metadata !{i32 1}
!40 = metadata !{i32 721152, metadata !37, metadata !"i", metadata !6, i32 24, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!41 = metadata !{i32 24, i32 36, metadata !37, null}
!42 = metadata !{i32 28, i32 13, metadata !13, null}
!43 = metadata !{i32 29, i32 13, metadata !13, null}
!44 = metadata !{i32 31, i32 13, metadata !13, null}
!45 = metadata !{i32 32, i32 13, metadata !13, null}
!46 = metadata !{i32 18, i32 15, metadata !47, null}
!47 = metadata !{i32 720907, metadata !13, i32 17, i32 23, metadata !6, i32 1} ; [ DW_TAG_lexical_block ]
!48 = metadata !{i32 19, i32 15, metadata !47, null}
!49 = metadata !{i32 33, i32 15, metadata !50, null}
!50 = metadata !{i32 720907, metadata !13, i32 32, i32 16, metadata !6, i32 4} ; [ DW_TAG_lexical_block ]
!51 = metadata !{i32 34, i32 15, metadata !50, null}
!52 = metadata !{i32 35, i32 13, metadata !50, null}
!53 = metadata !{i32 37, i32 13, metadata !13, null}
!54 = metadata !{i32 40, i32 13, metadata !13, null}
