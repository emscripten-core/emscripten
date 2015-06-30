; ModuleID = '/tmp/tmpHcVUBJ/a.out.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

%struct.Other = type { i8 }

@.str = private unnamed_addr constant [17 x i8] c"autorun called!\0A\00", align 1
@other = internal global %struct.Other zeroinitializer, align 1
@.str1 = private unnamed_addr constant [14 x i8] c"main called!\0A\00", align 1
@.str2 = private unnamed_addr constant [7 x i8] c"Other\0A\00", align 1
@llvm.global_ctors = appending global [2 x { i32, void ()* }] [{ i32, void ()* } { i32 65535, void ()* bitcast (i32 ()* @_Z7autorunv to void ()*) }, { i32, void ()* } { i32 65535, void ()* @_GLOBAL__I_a }]

define internal i32 @_Z7autorunv() {
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([17 x i8], [17 x i8]* @.str, i32 0, i32 0))
  ret i32 0
}

declare i32 @printf(i8*, ...)

define internal void @__cxx_global_var_init() {
  call void @_ZN5OtherC1Ev(%struct.Other* @other)
  ret void
}

define internal void @_ZN5OtherC1Ev(%struct.Other* %this) unnamed_addr align 2 {
  %1 = alloca %struct.Other*, align 4
  store %struct.Other* %this, %struct.Other** %1, align 4
  %2 = load %struct.Other*, %struct.Other** %1
  call void @_ZN5OtherC2Ev(%struct.Other* %2)
  ret void
}

define i32 @main() {
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str1, i32 0, i32 0))
  ret i32 0
}

define internal void @_ZN5OtherC2Ev(%struct.Other* %this) unnamed_addr align 2 {
  %1 = alloca %struct.Other*, align 4
  store %struct.Other* %this, %struct.Other** %1, align 4
  %2 = load %struct.Other*, %struct.Other** %1
  %3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str2, i32 0, i32 0))
  ret void
}

define internal void @_GLOBAL__I_a() {
  call void @__cxx_global_var_init()
  ret void
}
