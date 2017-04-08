; ModuleID = 'bugpoint-reduced-simplified.bc'
source_filename = "bugpoint-output-7eb0c3b.bc"
target datalayout = "e-p:32:32-i64:64-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

%WrappedArray = type { [2 x i64] }

@.str = private unnamed_addr constant [19 x i8] c"hello, world! %ld\0A\00", align 1
@.wrapped = private constant %WrappedArray { [2 x i64] [ i64 100, i64 200 ] }

declare i32 @printf(i8*, ...)

define i64 @_read_wrapped_array(%WrappedArray*) { ; from rust
entry-block:
  %1 = load %WrappedArray, %WrappedArray* %0, align 8
  %2 = extractvalue %WrappedArray %1, 0
  %.elt = extractvalue [2 x i64] %2, 0
  ret i64 %.elt
}

define i64 @_read_wrapped_array2(%WrappedArray*) { ; from rust
entry-block:
  %1 = load %WrappedArray, %WrappedArray* %0, align 8
  %2 = extractvalue %WrappedArray %1, 0
  %.elt = extractvalue [2 x i64] %2, 1
  ret i64 %.elt
}

define i32 @main() {
entry:
  %val = call i64 @_read_wrapped_array(%WrappedArray* @.wrapped)
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([19 x i8], [19 x i8]* @.str, i32 0, i32 0), i64 %val)
  %val2 = call i64 @_read_wrapped_array2(%WrappedArray* @.wrapped)
  %call2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([19 x i8], [19 x i8]* @.str, i32 0, i32 0), i64 %val2)
  ret i32 0
}

