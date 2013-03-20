; ModuleID = '/dev/shm/tmp/src.cpp.o'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i386-pc-linux-gnu"

; Load and store an entire structure as a whole (and also load as a whole, extract values and save separately, etc.)

%struct.X = type { i32, i32 }

@.str = private unnamed_addr constant [9 x i8] c"*%d,%d*\0A\00" ; [#uses=1]

; [#uses=0]
define i32 @main() {
entry:
  %retval = alloca i32, align 4                   ; [#uses=1]
  %x = alloca %struct.X, align 4                  ; [#uses=2]
  %y = alloca %struct.X, align 4                  ; [#uses=2]
  store i32 0, i32* %retval
  %a = getelementptr inbounds %struct.X* %x, i32 0, i32 0, !dbg !16 ; [#uses=1]
  store i32 5, i32* %a, align 4, !dbg !16
  %b = getelementptr inbounds %struct.X* %x, i32 0, i32 1, !dbg !17 ; [#uses=1]
  store i32 22, i32* %b, align 4, !dbg !17

  %allx = load %struct.X* %x, align 4, !dbg !13 ; [#uses=1]
  store %struct.X %allx, %struct.X* %y, align 4, !dbg !15

  %a1 = getelementptr inbounds %struct.X* %y, i32 0, i32 0, !dbg !18 ; [#uses=1]
  %tmp = load i32* %a1, align 4, !dbg !18         ; [#uses=1]
  %b2 = getelementptr inbounds %struct.X* %y, i32 0, i32 1, !dbg !18 ; [#uses=1]
  %tmp3 = load i32* %b2, align 4, !dbg !18        ; [#uses=1]
  %call = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([9 x i8]* @.str, i32 0, i32 0), i32 %tmp, i32 %tmp3), !dbg !18 ; [#uses=0]

  store i32 7, i32* %a, align 4, !dbg !16
  store i32 96, i32* %b, align 4, !dbg !17
  %allx2 = load %struct.X* %x, align 4, !dbg !13 ; [#uses=1]

  %x_a = extractvalue %struct.X %allx2, 0            ; [#uses=1]
  store i32 %x_a, i32* %a1, align 4
  %x_b = extractvalue %struct.X %allx2, 1            ; [#uses=1]
  store i32 %x_b, i32* %b2, align 4

  %tmp5 = load i32* %a1, align 4, !dbg !18         ; [#uses=1]
  %tmp6 = load i32* %b2, align 4, !dbg !18        ; [#uses=1]
  %call2 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([9 x i8]* @.str, i32 0, i32 0), i32 %tmp5, i32 %tmp6), !dbg !18 ; [#uses=0]

  %ptr = inttoptr i32 52 to i32*                  ; [#uses=1]
  store %struct.X { i32 ptrtoint (i32* getelementptr inbounds (i32* %ptr, i32 1, i32 0) to i32), i32 3 }, %struct.X* %y, align 4 ; store entire struct at once

  %tmp5b = load i32* %a1, align 4, !dbg !18         ; [#uses=1]
  %tmp6b = load i32* %b2, align 4, !dbg !18        ; [#uses=1]
  %call3 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([9 x i8]* @.str, i32 0, i32 0), i32 %tmp5b, i32 %tmp6b), !dbg !18 ; [#uses=0]

  ret i32 0, !dbg !19
}

; [#uses=1]
declare i32 @printf(i8*, ...)

!llvm.dbg.sp = !{!0}

!0 = metadata !{i32 589870, i32 0, metadata !1, metadata !"main", metadata !"main", metadata !"", metadata !1, i32 5, metadata !3, i1 false, i1 true, i32 0, i32 0, i32 0, i32 256, i1 false, i32 ()* @main} ; [ DW_TAG_subprogram ]
!1 = metadata !{i32 589865, metadata !"/dev/shm/tmp/src.cpp", metadata !"/dev/shm/tmp", metadata !2} ; [ DW_TAG_file_type ]
!2 = metadata !{i32 589841, i32 0, i32 4, metadata !"/dev/shm/tmp/src.cpp", metadata !"/dev/shm/tmp", metadata !"clang version 2.9 (tags/RELEASE_29/final)", i1 true, i1 false, metadata !"", i32 0} ; [ DW_TAG_compile_unit ]
!3 = metadata !{i32 589845, metadata !1, metadata !"", metadata !1, i32 0, i64 0, i64 0, i32 0, i32 0, i32 0, metadata !4, i32 0, i32 0} ; [ DW_TAG_subroutine_type ]
!4 = metadata !{metadata !5}
!5 = metadata !{i32 589860, metadata !2, metadata !"int", null, i32 0, i64 32, i64 32, i64 0, i32 0, i32 5} ; [ DW_TAG_base_type ]
!6 = metadata !{i32 590080, metadata !7, metadata !"x", metadata !1, i32 6, metadata !8, i32 0} ; [ DW_TAG_auto_variable ]
!7 = metadata !{i32 589835, metadata !0, i32 5, i32 11, metadata !1, i32 0} ; [ DW_TAG_lexical_block ]
!8 = metadata !{i32 589826, metadata !2, metadata !"X", metadata !1, i32 3, i64 64, i64 32, i32 0, i32 0, null, metadata !9, i32 0, null, metadata !12} ; [ DW_TAG_class_type ]
!9 = metadata !{metadata !10, metadata !11}
!10 = metadata !{i32 589837, metadata !1, metadata !"a", metadata !1, i32 3, i64 32, i64 32, i64 0, i32 0, metadata !5} ; [ DW_TAG_member ]
!11 = metadata !{i32 589837, metadata !1, metadata !"b", metadata !1, i32 3, i64 32, i64 32, i64 32, i32 0, metadata !5} ; [ DW_TAG_member ]
!12 = metadata !{i32 0}
!13 = metadata !{i32 6, i32 3, metadata !7, null}
!14 = metadata !{i32 590080, metadata !7, metadata !"y", metadata !1, i32 6, metadata !8, i32 0} ; [ DW_TAG_auto_variable ]
!15 = metadata !{i32 6, i32 6, metadata !7, null}
!16 = metadata !{i32 7, i32 1, metadata !7, null}
!17 = metadata !{i32 8, i32 1, metadata !7, null}
!18 = metadata !{i32 9, i32 13, metadata !7, null}
!19 = metadata !{i32 10, i32 13, metadata !7, null}
