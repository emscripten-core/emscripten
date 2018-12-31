; ModuleID = 'work.bc'
target datalayout = "e-p:32:32-i64:64-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

%struct.point = type { i8, i8, i8, i8, i8, i8, [20 x i8] }

@_ZTIi = external constant { i8*, i8* }

declare void @_Z4chak5point(%struct.point* byval align 1) #0

define i32 @main(i32 %argc, i8** %argv) #0 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i8**, align 4
  %x = alloca i32*, align 4
  %y = alloca i32*, align 4
  %p = alloca %struct.point, align 1
  %4 = alloca %struct.point, align 1
  %5 = alloca i8*
  %6 = alloca i32
  %7 = alloca %struct.point, align 1
  %x1 = alloca i32, align 4
  %8 = alloca i32
  store i32 0, i32* %1
  store i32 %argc, i32* %2, align 4
  store i8** %argv, i8*** %3, align 4
  %9 = load i8**, i8*** %3, align 4
  %10 = getelementptr inbounds i8*, i8** %9, i32 0
  %11 = load i8*, i8** %10, align 4
  %12 = bitcast i8* %11 to i32*
  store i32* %12, i32** %x, align 4
  %13 = load i8**, i8*** %3, align 4
  %14 = getelementptr inbounds i8*, i8** %13, i32 0
  %15 = load i8*, i8** %14, align 4
  %16 = bitcast i8* %15 to i32*
  store i32* %16, i32** %y, align 4
  %17 = load i32*, i32** %x, align 4
  %18 = load i32, i32* %17, align 4
  %19 = load i32*, i32** %y, align 4
  %20 = load i32, i32* %19, align 4
  call void @_ZN5pointC2Eii(%struct.point* %p, i32 %18, i32 %20)
  %21 = bitcast %struct.point* %4 to i8*
  %22 = bitcast %struct.point* %p to i8*
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %21, i8* %22, i32 26, i32 1, i1 false)
  invoke void @_Z4chak5point(%struct.point* byval align 1 %4)
          to label %23 unwind label %31

; <label>:23                                      ; preds = %0
  %24 = load i32*, i32** %x, align 4
  %25 = bitcast i32* %24 to i8*
  %26 = load i32*, i32** %y, align 4
  %27 = bitcast i32* %26 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %25, i8* %27, i32 8, i32 4, i1 false)
  %28 = bitcast %struct.point* %7 to i8*
  %29 = bitcast %struct.point* %p to i8*
  %waka = add i32 26, 0
  call void @emscripten_preinvoke(i32 1000)
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %28, i8* %29, i32 %waka, i32 1, i1 false)
  %fizzle = call i32 @emscripten_postinvoke(i32 1000)
  invoke void @_Z4chak5point(%struct.point* byval align 1 %7)
          to label %30 unwind label %31

; <label>:30                                      ; preds = %23
  br label %44

; <label>:31                                      ; preds = %23, %0
  %32 = landingpad { i8*, i32 }
          catch i8* bitcast ({ i8*, i8* }* @_ZTIi to i8*)
  %33 = extractvalue { i8*, i32 } %32, 0
  store i8* %33, i8** %5
  %34 = extractvalue { i8*, i32 } %32, 1
  store i32 %34, i32* %6
  br label %35

; <label>:35                                      ; preds = %31
  %36 = load i32, i32* %6
  %37 = call i32 @llvm.eh.typeid.for(i8* bitcast ({ i8*, i8* }* @_ZTIi to i8*)) #2
  %38 = icmp eq i32 %36, %37
  br i1 %38, label %39, label %47

; <label>:39                                      ; preds = %35
  %40 = load i8*, i8** %5
  %41 = call i8* @__cxa_begin_catch(i8* %40) #2
  %42 = bitcast i8* %41 to i32*
  %43 = load i32, i32* %42, align 4
  store i32 %43, i32* %x1, align 4
  store i32 1, i32* %1
  store i32 1, i32* %8
  call void @__cxa_end_catch() #2
  br label %45

; <label>:44                                      ; preds = %30
  store i32 0, i32* %1
  br label %45

; <label>:45                                      ; preds = %44, %39
  %46 = load i32, i32* %1
  ret i32 %46

; <label>:47                                      ; preds = %35
  %48 = load i8*, i8** %5
  %49 = load i32, i32* %6
  %50 = insertvalue { i8*, i32 } undef, i8* %48, 0
  %51 = insertvalue { i8*, i32 } %50, i32 %49, 1
  resume { i8*, i32 } %51
}

; Function Attrs: nounwind
declare void @_ZN5pointC2Eii(%struct.point*, i32, i32) unnamed_addr #1

; Function Attrs: nounwind
declare void @llvm.memcpy.p0i8.p0i8.i32(i8* nocapture, i8* nocapture readonly, i32, i32, i1) #2

declare i32 @__gxx_personality_v0(...)

; Function Attrs: nounwind readnone
declare i32 @llvm.eh.typeid.for(i8*) #3

declare i8* @__cxa_begin_catch(i8*)

declare void @__cxa_end_catch()

declare void @emscripten_preinvoke(i32)

declare i32 @emscripten_postinvoke(i32)

attributes #0 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { nounwind readnone }


