; ModuleID = '/tmp/emscripten_temp/src.cpp.o'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S128"
target triple = "i386-pc-linux-gnu"

@_ZL3buf = internal global [20 x i16] zeroinitializer, align 2
@.str = private unnamed_addr constant [13 x i8] c"hello world\0A\00", align 1
@.str1 = private unnamed_addr constant [6 x i8] c"more\0A\00", align 1
@.str2 = private unnamed_addr constant [6 x i8] c"*%d*\0A\00", align 1

define i32 @main() {
entry:
  br label %two

two:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval
  %call = call i32 @setjmp(i16* getelementptr inbounds ([20 x i16]* @_ZL3buf, i32 0, i32 0)) returns_twice, !dbg !20
  %tobool = icmp ne i32 %call, 0, !dbg !20
  br i1 %tobool, label %if.else, label %if.then, !dbg !20

if.then:                                          ; preds = %entry
  %call1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([13 x i8]* @.str, i32 0, i32 0)), !dbg !22
  call void @longjmp(i16* getelementptr inbounds ([20 x i16]* @_ZL3buf, i32 0, i32 0), i32 10), !dbg !24
  br label %if.end, !dbg !25

if.else:                                          ; preds = %entry
  %call2 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([6 x i8]* @.str1, i32 0, i32 0)), !dbg !26
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %aaa = phi i32 [ -1, %if.then ], [ 0, %if.else ], [ 1, %two ], [ 2, %entry ]
  %call3 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([6 x i8]* @.str2, i32 0, i32 0), i32 %aaa), !dbg !26
  ret i32 %aaa, !dbg !28
}

declare i32 @setjmp(i16*) returns_twice

declare i32 @printf(i8*, ...)

declare void @longjmp(i16*, i32)

!llvm.dbg.cu = !{!0}

!0 = metadata !{i32 786449, i32 0, i32 4, metadata !"/tmp/emscripten_temp/src.cpp", metadata !"/home/alon/Dev/emscripten", metadata !"clang version 3.1 (trunk 150936)", i1 true, i1 false, metadata !"", i32 0, metadata !1, metadata !1, metadata !3, metadata !12} ; [ DW_TAG_compile_unit ]
!1 = metadata !{metadata !2}
!2 = metadata !{i32 0}
!3 = metadata !{metadata !4}
!4 = metadata !{metadata !5}
!5 = metadata !{i32 786478, i32 0, metadata !6, metadata !"main", metadata !"main", metadata !"", metadata !6, i32 7, metadata !7, i1 false, i1 true, i32 0, i32 0, null, i32 256, i1 false, i32 ()* @main, null, null, metadata !10} ; [ DW_TAG_subprogram ]
!6 = metadata !{i32 786473, metadata !"/tmp/emscripten_temp/src.cpp", metadata !"/home/alon/Dev/emscripten", null} ; [ DW_TAG_file_type ]
!7 = metadata !{i32 786453, i32 0, metadata !"", i32 0, i32 0, i64 0, i64 0, i64 0, i32 0, null, metadata !8, i32 0, i32 0} ; [ DW_TAG_subroutine_type ]
!8 = metadata !{metadata !9}
!9 = metadata !{i32 786468, null, metadata !"int", null, i32 0, i64 32, i64 32, i64 0, i32 0, i32 5} ; [ DW_TAG_base_type ]
!10 = metadata !{metadata !11}
!11 = metadata !{i32 786468}                      ; [ DW_TAG_base_type ]
!12 = metadata !{metadata !13}
!13 = metadata !{metadata !14}
!14 = metadata !{i32 786484, i32 0, null, metadata !"buf", metadata !"buf", metadata !"_ZL3buf", metadata !6, i32 5, metadata !15, i32 1, i32 1, [20 x i16]* @_ZL3buf} ; [ DW_TAG_variable ]
!15 = metadata !{i32 786454, null, metadata !"jmp_buf", metadata !6, i32 279, i64 0, i64 0, i64 0, i32 0, metadata !16} ; [ DW_TAG_typedef ]
!16 = metadata !{i32 786433, null, metadata !"", null, i32 0, i64 320, i64 16, i32 0, i32 0, metadata !17, metadata !18, i32 0, i32 0} ; [ DW_TAG_array_type ]
!17 = metadata !{i32 786468, null, metadata !"unsigned short", null, i32 0, i64 16, i64 16, i64 0, i32 0, i32 7} ; [ DW_TAG_base_type ]
!18 = metadata !{metadata !19}
!19 = metadata !{i32 786465, i64 0, i64 19}       ; [ DW_TAG_subrange_type ]
!20 = metadata !{i32 8, i32 18, metadata !21, null}
!21 = metadata !{i32 786443, metadata !5, i32 7, i32 22, metadata !6, i32 0} ; [ DW_TAG_lexical_block ]
!22 = metadata !{i32 9, i32 15, metadata !23, null}
!23 = metadata !{i32 786443, metadata !21, i32 8, i32 31, metadata !6, i32 1} ; [ DW_TAG_lexical_block ]
!24 = metadata !{i32 10, i32 15, metadata !23, null}
!25 = metadata !{i32 11, i32 13, metadata !23, null}
!26 = metadata !{i32 12, i32 15, metadata !27, null}
!27 = metadata !{i32 786443, metadata !21, i32 11, i32 20, metadata !6, i32 2} ; [ DW_TAG_lexical_block ]
!28 = metadata !{i32 14, i32 13, metadata !21, null}
