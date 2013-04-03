; ModuleID = '/tmp/tmpnNWXaG/a.out.bc'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S128"
target triple = "i386-pc-linux-gnu"

%class.A = type { %union.anon }
%union.anon = type { [256 x i32] }

@.str = private unnamed_addr constant [6 x i8] c"dummy\00", align 1
@.str1 = private unnamed_addr constant [19 x i8] c"I should be 5!:%d\0A\00", align 1

define i32 @main() nounwind {
  %a = alloca %class.A, align 4
  call void @_ZN1AC1EPKcii(%class.A* %a, i8* getelementptr inbounds ([6 x i8]* @.str, i32 0, i32 0), i32 3, i32 5)
  ret i32 0
}

define internal void @_ZN1AC1EPKcii(%class.A* nocapture %this, i8* %pattern, i32 %length, i32 %pattern_c) unnamed_addr nounwind noinline align 2 {
  tail call void @_ZN1AC2EPKcii(%class.A* %this, i8* %pattern, i32 %length, i32 %pattern_c)
  ret void
}

define internal void @_ZN1AC2EPKcii(%class.A* nocapture %this, i8* %pattern, i32 %length, i32 %pattern_c) unnamed_addr nounwind noinline align 2 {
  %1 = getelementptr inbounds %class.A* %this, i32 0, i32 0, i32 0, i32 64
  %pattern.c = ptrtoint i8* %pattern to i32
  store i32 %pattern.c, i32* %1, align 4
  %2 = getelementptr inbounds %class.A* %this, i32 0, i32 0, i32 0, i32 65
  store i32 %length, i32* %2, align 4
  %3 = tail call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([19 x i8]* @.str1, i32 0, i32 0), i32 %pattern_c)
  ret void
}

declare i32 @printf(i8* nocapture, ...) nounwind
