; ModuleID = '/tmp/emscripten_temp/src.cpp.o'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [7 x i8] c"cheez\0A\00", align 1
@.str1 = private unnamed_addr constant [6 x i8] c"*%d*\0A\00", align 1

define i32 @main() {
entry:
  %retval = alloca i32, align 4
  %x = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, i32* %retval
  store i32 5, i32* %x, align 4  store i32 0, i32* %i, align 4  br label %for.cond
for.cond:                                         ; preds = %for.inc, %entry
  %xxx = phi i32 [ 43, %entry ], [add (i32 %xxx, i32 3), %for.inc ]        ; [#uses=1 type=i1]
  %yyy = phi i32 [ 32, %entry ], [add (i32 %yyy, i32 2), %for.inc ]        ; [#uses=1 type=i1]
  %zzz = phi i32 [ 21, %entry ], [add (i32 %zzz, i32 1), %for.inc ]        ; [#uses=1 type=i1]
  %0 = load i32, i32* %i, align 4  %cmp = icmp slt i32 %0, 6  br i1 %cmp, label %for.body, label %for.end
for.body:                                         ; preds = %for.cond
  %1 = load i32, i32* %x, align 4  %2 = load i32, i32* %i, align 4  %mul = mul nsw i32 %1, %2  %3 = load i32, i32* %x, align 4  %add = add nsw i32 %3, %mul  store i32 %add, i32* %x, align 4  %4 = load i32, i32* %x, align 4  %cmp1 = icmp sgt i32 %4, 1000  br i1 %cmp1, label %if.then, label %if.end4
if.then:                                          ; preds = %for.body
  %5 = load i32, i32* %x, align 4  %rem = srem i32 %5, 7  %cmp2 = icmp eq i32 %rem, 0  br i1 %cmp2, label %if.then3, label %if.end
if.then3:                                         ; preds = %if.then
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0))  br label %if.end
if.end:                                           ; preds = %if.then3, %if.then
  %6 = load i32, i32* %x, align 4  %div = sdiv i32 %6, 2  store i32 %div, i32* %x, align 4  br label %for.end
if.end4:                                          ; preds = %for.body
  br label %for.inc
for.inc:                                          ; preds = %if.end4
  %7 = load i32, i32* %i, align 4  %inc = add nsw i32 %7, 1  store i32 %inc, i32* %i, align 4  br label %for.cond
for.end:                                          ; preds = %if.end, %for.cond
  %8 = load i32, i32* %x, align 4  %call5 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str1, i32 0, i32 0), i32 %zzz)  ret i32 0}

declare i32 @printf(i8*, ...)

