; ModuleID = '/tmp/emscripten_temp/src.cpp.o'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S128"
target triple = "i386-pc-linux-gnu"

; Check aligned operations that are of non-aligned sizes (e.g., memcpy of 7 bytes at alignment 4)

@.str = private unnamed_addr constant [13 x i8] c"hello worlds\00", align 1 ; [#uses=1 type=[13 x i8]*]

; [#uses=0]
define i32 @main() {
entry:
  %retval = alloca i32, align 4                   ; [#uses=3 type=i32*]
  %str = alloca i8*, align 4                      ; [#uses=25 type=i8**]
  %len = alloca i32, align 4                      ; [#uses=38 type=i32*]
  %saved_stack = alloca i8*                       ; [#uses=2 type=i8**]
  %cleanup.dest.slot = alloca i32                 ; [#uses=1 type=i32*]
  store i32 0, i32* %retval
  store i8* getelementptr inbounds ([13 x i8]* @.str, i32 0, i32 0), i8** %str, align 4, !dbg !17 ; [debug line = 6:39]
  %0 = load i8** %str, align 4, !dbg !20          ; [#uses=1 type=i8*] [debug line = 7:23]
  %call = call i32 @strlen(i8* %0), !dbg !20      ; [#uses=1 type=i32] [debug line = 7:23]
  store i32 %call, i32* %len, align 4, !dbg !20   ; [debug line = 7:23]
  %1 = load i32* %len, align 4, !dbg !21          ; [#uses=1 type=i32] [debug line = 8:29]
  %add = add nsw i32 %1, 1, !dbg !21              ; [#uses=1 type=i32] [debug line = 8:29]
  %2 = call i8* @llvm.stacksave(), !dbg !21       ; [#uses=1 type=i8*] [debug line = 8:29]
  store i8* %2, i8** %saved_stack, !dbg !21       ; [debug line = 8:29]
  %vla = alloca i8, i32 %add, align 1, !dbg !21   ; [#uses=93 type=i8*] [debug line = 8:29]
  %3 = load i32* %len, align 4, !dbg !27          ; [#uses=1 type=i32] [debug line = 13:13]
  call void @llvm.memset.p0i8.i32(i8* %vla, i8 46, i32 %3, i32 4, i1 false), !dbg !27 ; [debug line = 13:13]
  %4 = load i32* %len, align 4, !dbg !27          ; [#uses=1 type=i32] [debug line = 13:13]
  %arrayidx = getelementptr inbounds i8* %vla, i32 %4, !dbg !27 ; [#uses=1 type=i8*] [debug line = 13:13]
  store i8 0, i8* %arrayidx, !dbg !27             ; [debug line = 13:13]
  %call1 = call i32 @puts(i8* %vla), !dbg !29     ; [#uses=0 type=i32] [debug line = 13:24]
  %5 = load i32* %len, align 4, !dbg !30          ; [#uses=1 type=i32] [debug line = 14:13]
  call void @llvm.memset.p0i8.i32(i8* %vla, i8 46, i32 %5, i32 4, i1 false), !dbg !30 ; [debug line = 14:13]
  %6 = load i32* %len, align 4, !dbg !30          ; [#uses=1 type=i32] [debug line = 14:13]
  %arrayidx2 = getelementptr inbounds i8* %vla, i32 %6, !dbg !30 ; [#uses=1 type=i8*] [debug line = 14:13]
  store i8 0, i8* %arrayidx2, !dbg !30            ; [debug line = 14:13]
  %7 = load i8** %str, align 4, !dbg !32          ; [#uses=1 type=i8*] [debug line = 14:24]
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %vla, i8* %7, i32 0, i32 4, i1 false), !dbg !32 ; [debug line = 14:24]
  %call3 = call i32 @puts(i8* %vla), !dbg !33     ; [#uses=0 type=i32] [debug line = 14:33]
  %8 = load i32* %len, align 4, !dbg !34          ; [#uses=1 type=i32] [debug line = 15:13]
  call void @llvm.memset.p0i8.i32(i8* %vla, i8 46, i32 %8, i32 4, i1 false), !dbg !34 ; [debug line = 15:13]
  %9 = load i32* %len, align 4, !dbg !34          ; [#uses=1 type=i32] [debug line = 15:13]
  %arrayidx4 = getelementptr inbounds i8* %vla, i32 %9, !dbg !34 ; [#uses=1 type=i8*] [debug line = 15:13]
  store i8 0, i8* %arrayidx4, !dbg !34            ; [debug line = 15:13]
  %10 = load i8** %str, align 4, !dbg !36         ; [#uses=1 type=i8*] [debug line = 15:24]
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %vla, i8* %10, i32 1, i32 4, i1 false), !dbg !36 ; [debug line = 15:24]
  %call5 = call i32 @puts(i8* %vla), !dbg !37     ; [#uses=0 type=i32] [debug line = 15:33]
  %11 = load i32* %len, align 4, !dbg !38         ; [#uses=1 type=i32] [debug line = 16:13]
  call void @llvm.memset.p0i8.i32(i8* %vla, i8 46, i32 %11, i32 4, i1 false), !dbg !38 ; [debug line = 16:13]
  %12 = load i32* %len, align 4, !dbg !38         ; [#uses=1 type=i32] [debug line = 16:13]
  %arrayidx6 = getelementptr inbounds i8* %vla, i32 %12, !dbg !38 ; [#uses=1 type=i8*] [debug line = 16:13]
  store i8 0, i8* %arrayidx6, !dbg !38            ; [debug line = 16:13]
  %13 = load i8** %str, align 4, !dbg !40         ; [#uses=1 type=i8*] [debug line = 16:24]
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %vla, i8* %13, i32 2, i32 4, i1 false), !dbg !40 ; [debug line = 16:24]
  %call7 = call i32 @puts(i8* %vla), !dbg !41     ; [#uses=0 type=i32] [debug line = 16:33]
  %14 = load i32* %len, align 4, !dbg !42         ; [#uses=1 type=i32] [debug line = 17:13]
  call void @llvm.memset.p0i8.i32(i8* %vla, i8 46, i32 %14, i32 4, i1 false), !dbg !42 ; [debug line = 17:13]
  %15 = load i32* %len, align 4, !dbg !42         ; [#uses=1 type=i32] [debug line = 17:13]
  %arrayidx8 = getelementptr inbounds i8* %vla, i32 %15, !dbg !42 ; [#uses=1 type=i8*] [debug line = 17:13]
  store i8 0, i8* %arrayidx8, !dbg !42            ; [debug line = 17:13]
  %16 = load i8** %str, align 4, !dbg !44         ; [#uses=1 type=i8*] [debug line = 17:24]
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %vla, i8* %16, i32 3, i32 4, i1 false), !dbg !44 ; [debug line = 17:24]
  %call9 = call i32 @puts(i8* %vla), !dbg !45     ; [#uses=0 type=i32] [debug line = 17:33]
  %17 = load i32* %len, align 4, !dbg !46         ; [#uses=1 type=i32] [debug line = 18:13]
  call void @llvm.memset.p0i8.i32(i8* %vla, i8 46, i32 %17, i32 4, i1 false), !dbg !46 ; [debug line = 18:13]
  %18 = load i32* %len, align 4, !dbg !46         ; [#uses=1 type=i32] [debug line = 18:13]
  %arrayidx10 = getelementptr inbounds i8* %vla, i32 %18, !dbg !46 ; [#uses=1 type=i8*] [debug line = 18:13]
  store i8 0, i8* %arrayidx10, !dbg !46           ; [debug line = 18:13]
  %19 = load i8** %str, align 4, !dbg !48         ; [#uses=1 type=i8*] [debug line = 18:24]
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %vla, i8* %19, i32 4, i32 4, i1 false), !dbg !48 ; [debug line = 18:24]
  %call11 = call i32 @puts(i8* %vla), !dbg !49    ; [#uses=0 type=i32] [debug line = 18:33]
  %20 = load i32* %len, align 4, !dbg !50         ; [#uses=1 type=i32] [debug line = 19:13]
  call void @llvm.memset.p0i8.i32(i8* %vla, i8 46, i32 %20, i32 4, i1 false), !dbg !50 ; [debug line = 19:13]
  %21 = load i32* %len, align 4, !dbg !50         ; [#uses=1 type=i32] [debug line = 19:13]
  %arrayidx12 = getelementptr inbounds i8* %vla, i32 %21, !dbg !50 ; [#uses=1 type=i8*] [debug line = 19:13]
  store i8 0, i8* %arrayidx12, !dbg !50           ; [debug line = 19:13]
  %22 = load i8** %str, align 4, !dbg !52         ; [#uses=1 type=i8*] [debug line = 19:24]
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %vla, i8* %22, i32 5, i32 4, i1 false), !dbg !52 ; [debug line = 19:24]
  %call13 = call i32 @puts(i8* %vla), !dbg !53    ; [#uses=0 type=i32] [debug line = 19:33]
  %23 = load i32* %len, align 4, !dbg !54         ; [#uses=1 type=i32] [debug line = 20:13]
  call void @llvm.memset.p0i8.i32(i8* %vla, i8 46, i32 %23, i32 4, i1 false), !dbg !54 ; [debug line = 20:13]
  %24 = load i32* %len, align 4, !dbg !54         ; [#uses=1 type=i32] [debug line = 20:13]
  %arrayidx14 = getelementptr inbounds i8* %vla, i32 %24, !dbg !54 ; [#uses=1 type=i8*] [debug line = 20:13]
  store i8 0, i8* %arrayidx14, !dbg !54           ; [debug line = 20:13]
  %25 = load i8** %str, align 4, !dbg !56         ; [#uses=1 type=i8*] [debug line = 20:24]
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %vla, i8* %25, i32 6, i32 4, i1 false), !dbg !56 ; [debug line = 20:24]
  %call15 = call i32 @puts(i8* %vla), !dbg !57    ; [#uses=0 type=i32] [debug line = 20:33]
  %26 = load i32* %len, align 4, !dbg !58         ; [#uses=1 type=i32] [debug line = 21:13]
  call void @llvm.memset.p0i8.i32(i8* %vla, i8 46, i32 %26, i32 4, i1 false), !dbg !58 ; [debug line = 21:13]
  %27 = load i32* %len, align 4, !dbg !58         ; [#uses=1 type=i32] [debug line = 21:13]
  %arrayidx16 = getelementptr inbounds i8* %vla, i32 %27, !dbg !58 ; [#uses=1 type=i8*] [debug line = 21:13]
  store i8 0, i8* %arrayidx16, !dbg !58           ; [debug line = 21:13]
  %28 = load i8** %str, align 4, !dbg !60         ; [#uses=1 type=i8*] [debug line = 21:24]
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %vla, i8* %28, i32 7, i32 4, i1 false), !dbg !60 ; [debug line = 21:24]
  %call17 = call i32 @puts(i8* %vla), !dbg !61    ; [#uses=0 type=i32] [debug line = 21:33]
  %29 = load i32* %len, align 4, !dbg !62         ; [#uses=1 type=i32] [debug line = 22:13]
  call void @llvm.memset.p0i8.i32(i8* %vla, i8 46, i32 %29, i32 4, i1 false), !dbg !62 ; [debug line = 22:13]
  %30 = load i32* %len, align 4, !dbg !62         ; [#uses=1 type=i32] [debug line = 22:13]
  %arrayidx18 = getelementptr inbounds i8* %vla, i32 %30, !dbg !62 ; [#uses=1 type=i8*] [debug line = 22:13]
  store i8 0, i8* %arrayidx18, !dbg !62           ; [debug line = 22:13]
  %31 = load i8** %str, align 4, !dbg !64         ; [#uses=1 type=i8*] [debug line = 22:24]
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %vla, i8* %31, i32 8, i32 4, i1 false), !dbg !64 ; [debug line = 22:24]
  %call19 = call i32 @puts(i8* %vla), !dbg !65    ; [#uses=0 type=i32] [debug line = 22:33]
  %32 = load i32* %len, align 4, !dbg !66         ; [#uses=1 type=i32] [debug line = 23:13]
  call void @llvm.memset.p0i8.i32(i8* %vla, i8 46, i32 %32, i32 4, i1 false), !dbg !66 ; [debug line = 23:13]
  %33 = load i32* %len, align 4, !dbg !66         ; [#uses=1 type=i32] [debug line = 23:13]
  %arrayidx20 = getelementptr inbounds i8* %vla, i32 %33, !dbg !66 ; [#uses=1 type=i8*] [debug line = 23:13]
  store i8 0, i8* %arrayidx20, !dbg !66           ; [debug line = 23:13]
  %34 = load i8** %str, align 4, !dbg !68         ; [#uses=1 type=i8*] [debug line = 23:24]
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %vla, i8* %34, i32 9, i32 4, i1 false), !dbg !68 ; [debug line = 23:24]
  %call21 = call i32 @puts(i8* %vla), !dbg !69    ; [#uses=0 type=i32] [debug line = 23:33]
  %35 = load i32* %len, align 4, !dbg !70         ; [#uses=1 type=i32] [debug line = 24:13]
  call void @llvm.memset.p0i8.i32(i8* %vla, i8 46, i32 %35, i32 4, i1 false), !dbg !70 ; [debug line = 24:13]
  %36 = load i32* %len, align 4, !dbg !70         ; [#uses=1 type=i32] [debug line = 24:13]
  %arrayidx22 = getelementptr inbounds i8* %vla, i32 %36, !dbg !70 ; [#uses=1 type=i8*] [debug line = 24:13]
  store i8 0, i8* %arrayidx22, !dbg !70           ; [debug line = 24:13]
  %37 = load i8** %str, align 4, !dbg !72         ; [#uses=1 type=i8*] [debug line = 24:24]
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %vla, i8* %37, i32 10, i32 4, i1 false), !dbg !72 ; [debug line = 24:24]
  %call23 = call i32 @puts(i8* %vla), !dbg !73    ; [#uses=0 type=i32] [debug line = 24:34]
  %38 = load i8** %str, align 4, !dbg !74         ; [#uses=1 type=i8*] [debug line = 26:13]
  %39 = load i32* %len, align 4, !dbg !74         ; [#uses=1 type=i32] [debug line = 26:13]
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %vla, i8* %38, i32 %39, i32 4, i1 false), !dbg !74 ; [debug line = 26:13]
  %call24 = call i32 @puts(i8* %vla), !dbg !75    ; [#uses=0 type=i32] [debug line = 26:24]
  %40 = load i8** %str, align 4, !dbg !76         ; [#uses=1 type=i8*] [debug line = 27:13]
  %41 = load i32* %len, align 4, !dbg !76         ; [#uses=1 type=i32] [debug line = 27:13]
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %vla, i8* %40, i32 %41, i32 4, i1 false), !dbg !76 ; [debug line = 27:13]
  call void @llvm.memset.p0i8.i32(i8* %vla, i8 46, i32 0, i32 4, i1 false), !dbg !77 ; [debug line = 27:24]
  %arrayidx25 = getelementptr inbounds i8* %vla, i32 0, !dbg !77 ; [#uses=1 type=i8*] [debug line = 27:24]
  store i8 0, i8* %arrayidx25, !dbg !77           ; [debug line = 27:24]
  %call26 = call i32 @puts(i8* %vla), !dbg !79    ; [#uses=0 type=i32] [debug line = 27:33]
  %42 = load i8** %str, align 4, !dbg !80         ; [#uses=1 type=i8*] [debug line = 28:13]
  %43 = load i32* %len, align 4, !dbg !80         ; [#uses=1 type=i32] [debug line = 28:13]
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %vla, i8* %42, i32 %43, i32 4, i1 false), !dbg !80 ; [debug line = 28:13]
  call void @llvm.memset.p0i8.i32(i8* %vla, i8 46, i32 1, i32 4, i1 false), !dbg !81 ; [debug line = 28:24]
  %arrayidx27 = getelementptr inbounds i8* %vla, i32 1, !dbg !81 ; [#uses=1 type=i8*] [debug line = 28:24]
  store i8 0, i8* %arrayidx27, !dbg !81           ; [debug line = 28:24]
  %call28 = call i32 @puts(i8* %vla), !dbg !83    ; [#uses=0 type=i32] [debug line = 28:33]
  %44 = load i8** %str, align 4, !dbg !84         ; [#uses=1 type=i8*] [debug line = 29:13]
  %45 = load i32* %len, align 4, !dbg !84         ; [#uses=1 type=i32] [debug line = 29:13]
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %vla, i8* %44, i32 %45, i32 4, i1 false), !dbg !84 ; [debug line = 29:13]
  call void @llvm.memset.p0i8.i32(i8* %vla, i8 46, i32 2, i32 4, i1 false), !dbg !85 ; [debug line = 29:24]
  %arrayidx29 = getelementptr inbounds i8* %vla, i32 2, !dbg !85 ; [#uses=1 type=i8*] [debug line = 29:24]
  store i8 0, i8* %arrayidx29, !dbg !85           ; [debug line = 29:24]
  %call30 = call i32 @puts(i8* %vla), !dbg !87    ; [#uses=0 type=i32] [debug line = 29:33]
  %46 = load i8** %str, align 4, !dbg !88         ; [#uses=1 type=i8*] [debug line = 30:13]
  %47 = load i32* %len, align 4, !dbg !88         ; [#uses=1 type=i32] [debug line = 30:13]
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %vla, i8* %46, i32 %47, i32 4, i1 false), !dbg !88 ; [debug line = 30:13]
  call void @llvm.memset.p0i8.i32(i8* %vla, i8 46, i32 3, i32 4, i1 false), !dbg !89 ; [debug line = 30:24]
  %arrayidx31 = getelementptr inbounds i8* %vla, i32 3, !dbg !89 ; [#uses=1 type=i8*] [debug line = 30:24]
  store i8 0, i8* %arrayidx31, !dbg !89           ; [debug line = 30:24]
  %call32 = call i32 @puts(i8* %vla), !dbg !91    ; [#uses=0 type=i32] [debug line = 30:33]
  %48 = load i8** %str, align 4, !dbg !92         ; [#uses=1 type=i8*] [debug line = 31:13]
  %49 = load i32* %len, align 4, !dbg !92         ; [#uses=1 type=i32] [debug line = 31:13]
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %vla, i8* %48, i32 %49, i32 4, i1 false), !dbg !92 ; [debug line = 31:13]
  call void @llvm.memset.p0i8.i32(i8* %vla, i8 46, i32 4, i32 4, i1 false), !dbg !93 ; [debug line = 31:24]
  %arrayidx33 = getelementptr inbounds i8* %vla, i32 4, !dbg !93 ; [#uses=1 type=i8*] [debug line = 31:24]
  store i8 0, i8* %arrayidx33, !dbg !93           ; [debug line = 31:24]
  %call34 = call i32 @puts(i8* %vla), !dbg !95    ; [#uses=0 type=i32] [debug line = 31:33]
  %50 = load i8** %str, align 4, !dbg !96         ; [#uses=1 type=i8*] [debug line = 32:13]
  %51 = load i32* %len, align 4, !dbg !96         ; [#uses=1 type=i32] [debug line = 32:13]
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %vla, i8* %50, i32 %51, i32 4, i1 false), !dbg !96 ; [debug line = 32:13]
  call void @llvm.memset.p0i8.i32(i8* %vla, i8 46, i32 5, i32 4, i1 false), !dbg !97 ; [debug line = 32:24]
  %arrayidx35 = getelementptr inbounds i8* %vla, i32 5, !dbg !97 ; [#uses=1 type=i8*] [debug line = 32:24]
  store i8 0, i8* %arrayidx35, !dbg !97           ; [debug line = 32:24]
  %call36 = call i32 @puts(i8* %vla), !dbg !99    ; [#uses=0 type=i32] [debug line = 32:33]
  %52 = load i8** %str, align 4, !dbg !100        ; [#uses=1 type=i8*] [debug line = 33:13]
  %53 = load i32* %len, align 4, !dbg !100        ; [#uses=1 type=i32] [debug line = 33:13]
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %vla, i8* %52, i32 %53, i32 4, i1 false), !dbg !100 ; [debug line = 33:13]
  call void @llvm.memset.p0i8.i32(i8* %vla, i8 46, i32 6, i32 4, i1 false), !dbg !101 ; [debug line = 33:24]
  %arrayidx37 = getelementptr inbounds i8* %vla, i32 6, !dbg !101 ; [#uses=1 type=i8*] [debug line = 33:24]
  store i8 0, i8* %arrayidx37, !dbg !101          ; [debug line = 33:24]
  %call38 = call i32 @puts(i8* %vla), !dbg !103   ; [#uses=0 type=i32] [debug line = 33:33]
  %54 = load i8** %str, align 4, !dbg !104        ; [#uses=1 type=i8*] [debug line = 34:13]
  %55 = load i32* %len, align 4, !dbg !104        ; [#uses=1 type=i32] [debug line = 34:13]
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %vla, i8* %54, i32 %55, i32 4, i1 false), !dbg !104 ; [debug line = 34:13]
  call void @llvm.memset.p0i8.i32(i8* %vla, i8 46, i32 7, i32 4, i1 false), !dbg !105 ; [debug line = 34:24]
  %arrayidx39 = getelementptr inbounds i8* %vla, i32 7, !dbg !105 ; [#uses=1 type=i8*] [debug line = 34:24]
  store i8 0, i8* %arrayidx39, !dbg !105          ; [debug line = 34:24]
  %call40 = call i32 @puts(i8* %vla), !dbg !107   ; [#uses=0 type=i32] [debug line = 34:33]
  %56 = load i8** %str, align 4, !dbg !108        ; [#uses=1 type=i8*] [debug line = 35:13]
  %57 = load i32* %len, align 4, !dbg !108        ; [#uses=1 type=i32] [debug line = 35:13]
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %vla, i8* %56, i32 %57, i32 4, i1 false), !dbg !108 ; [debug line = 35:13]
  call void @llvm.memset.p0i8.i32(i8* %vla, i8 46, i32 8, i32 4, i1 false), !dbg !109 ; [debug line = 35:24]
  %arrayidx41 = getelementptr inbounds i8* %vla, i32 8, !dbg !109 ; [#uses=1 type=i8*] [debug line = 35:24]
  store i8 0, i8* %arrayidx41, !dbg !109          ; [debug line = 35:24]
  %call42 = call i32 @puts(i8* %vla), !dbg !111   ; [#uses=0 type=i32] [debug line = 35:33]
  %58 = load i8** %str, align 4, !dbg !112        ; [#uses=1 type=i8*] [debug line = 36:13]
  %59 = load i32* %len, align 4, !dbg !112        ; [#uses=1 type=i32] [debug line = 36:13]
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %vla, i8* %58, i32 %59, i32 4, i1 false), !dbg !112 ; [debug line = 36:13]
  call void @llvm.memset.p0i8.i32(i8* %vla, i8 46, i32 9, i32 4, i1 false), !dbg !113 ; [debug line = 36:24]
  %arrayidx43 = getelementptr inbounds i8* %vla, i32 9, !dbg !113 ; [#uses=1 type=i8*] [debug line = 36:24]
  store i8 0, i8* %arrayidx43, !dbg !113          ; [debug line = 36:24]
  %call44 = call i32 @puts(i8* %vla), !dbg !115   ; [#uses=0 type=i32] [debug line = 36:33]
  %60 = load i8** %str, align 4, !dbg !116        ; [#uses=1 type=i8*] [debug line = 37:13]
  %61 = load i32* %len, align 4, !dbg !116        ; [#uses=1 type=i32] [debug line = 37:13]
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %vla, i8* %60, i32 %61, i32 4, i1 false), !dbg !116 ; [debug line = 37:13]
  call void @llvm.memset.p0i8.i32(i8* %vla, i8 46, i32 10, i32 4, i1 false), !dbg !117 ; [debug line = 37:24]
  %arrayidx45 = getelementptr inbounds i8* %vla, i32 10, !dbg !117 ; [#uses=1 type=i8*] [debug line = 37:24]
  store i8 0, i8* %arrayidx45, !dbg !117          ; [debug line = 37:24]
  %call46 = call i32 @puts(i8* %vla), !dbg !119   ; [#uses=0 type=i32] [debug line = 37:34]
  store i32 0, i32* %retval, !dbg !120            ; [debug line = 39:13]
  store i32 1, i32* %cleanup.dest.slot
  %62 = load i8** %saved_stack, !dbg !121         ; [#uses=1 type=i8*] [debug line = 40:11]
  call void @llvm.stackrestore(i8* %62), !dbg !121 ; [debug line = 40:11]
  %63 = load i32* %retval, !dbg !122              ; [#uses=1 type=i32] [debug line = 40:11]
  ret i32 %63, !dbg !122                          ; [debug line = 40:11]
}

; [#uses=1]
declare i32 @strlen(i8*)

; [#uses=1]
declare i8* @llvm.stacksave() nounwind

; [#uses=23]
declare void @llvm.memset.p0i8.i32(i8* nocapture, i8, i32, i32, i1) nounwind

; [#uses=24]
declare i32 @puts(i8*)

; [#uses=23]
declare void @llvm.memcpy.p0i8.p0i8.i32(i8* nocapture, i8* nocapture, i32, i32, i1) nounwind

; [#uses=1]
declare void @llvm.stackrestore(i8*) nounwind

!llvm.dbg.cu = !{!0}

!0 = metadata !{i32 720913, i32 0, i32 4, metadata !"/tmp/emscripten_temp/src.cpp", metadata !"/tmp/emscripten_temp", metadata !"clang version 3.0 (tags/RELEASE_30/rc3)", i1 true, i1 false, metadata !"", i32 0, metadata !1, metadata !1, metadata !3, metadata !1} ; [ DW_TAG_compile_unit ]
!1 = metadata !{metadata !2}
!2 = metadata !{i32 0}
!3 = metadata !{metadata !4}
!4 = metadata !{metadata !5}
!5 = metadata !{i32 720942, i32 0, metadata !6, metadata !"main", metadata !"main", metadata !"", metadata !6, i32 5, metadata !7, i1 false, i1 true, i32 0, i32 0, i32 0, i32 256, i1 false, i32 ()* @main, null, null, metadata !10} ; [ DW_TAG_subprogram ]
!6 = metadata !{i32 720937, metadata !"/tmp/emscripten_temp/src.cpp", metadata !"/tmp/emscripten_temp", null} ; [ DW_TAG_file_type ]
!7 = metadata !{i32 720917, i32 0, metadata !"", i32 0, i32 0, i64 0, i64 0, i32 0, i32 0, i32 0, metadata !8, i32 0, i32 0} ; [ DW_TAG_subroutine_type ]
!8 = metadata !{metadata !9}
!9 = metadata !{i32 720932, null, metadata !"int", null, i32 0, i64 32, i64 32, i64 0, i32 0, i32 5} ; [ DW_TAG_base_type ]
!10 = metadata !{metadata !11}
!11 = metadata !{i32 720932}                      ; [ DW_TAG_base_type ]
!12 = metadata !{i32 721152, metadata !13, metadata !"str", metadata !6, i32 6, metadata !14, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!13 = metadata !{i32 720907, metadata !5, i32 5, i32 22, metadata !6, i32 0} ; [ DW_TAG_lexical_block ]
!14 = metadata !{i32 720911, null, metadata !"", null, i32 0, i64 32, i64 32, i64 0, i32 0, metadata !15} ; [ DW_TAG_pointer_type ]
!15 = metadata !{i32 720932, null, metadata !"char", null, i32 0, i64 8, i64 8, i64 0, i32 0, i32 6} ; [ DW_TAG_base_type ]
!16 = metadata !{i32 6, i32 19, metadata !13, null}
!17 = metadata !{i32 6, i32 39, metadata !13, null}
!18 = metadata !{i32 721152, metadata !13, metadata !"len", metadata !6, i32 7, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!19 = metadata !{i32 7, i32 17, metadata !13, null}
!20 = metadata !{i32 7, i32 23, metadata !13, null}
!21 = metadata !{i32 8, i32 29, metadata !13, null}
!22 = metadata !{i32 721152, metadata !13, metadata !"curr", metadata !6, i32 8, metadata !23, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!23 = metadata !{i32 720897, null, metadata !"", null, i32 0, i64 0, i64 8, i32 0, i32 0, metadata !15, metadata !24, i32 0, i32 0} ; [ DW_TAG_array_type ]
!24 = metadata !{metadata !25}
!25 = metadata !{i32 720929, i64 1, i64 0}        ; [ DW_TAG_subrange_type ]
!26 = metadata !{i32 8, i32 18, metadata !13, null}
!27 = metadata !{i32 13, i32 13, metadata !28, null}
!28 = metadata !{i32 720907, metadata !13, i32 13, i32 13, metadata !6, i32 1} ; [ DW_TAG_lexical_block ]
!29 = metadata !{i32 13, i32 24, metadata !13, null}
!30 = metadata !{i32 14, i32 13, metadata !31, null}
!31 = metadata !{i32 720907, metadata !13, i32 14, i32 13, metadata !6, i32 2} ; [ DW_TAG_lexical_block ]
!32 = metadata !{i32 14, i32 24, metadata !13, null}
!33 = metadata !{i32 14, i32 33, metadata !13, null}
!34 = metadata !{i32 15, i32 13, metadata !35, null}
!35 = metadata !{i32 720907, metadata !13, i32 15, i32 13, metadata !6, i32 3} ; [ DW_TAG_lexical_block ]
!36 = metadata !{i32 15, i32 24, metadata !13, null}
!37 = metadata !{i32 15, i32 33, metadata !13, null}
!38 = metadata !{i32 16, i32 13, metadata !39, null}
!39 = metadata !{i32 720907, metadata !13, i32 16, i32 13, metadata !6, i32 4} ; [ DW_TAG_lexical_block ]
!40 = metadata !{i32 16, i32 24, metadata !13, null}
!41 = metadata !{i32 16, i32 33, metadata !13, null}
!42 = metadata !{i32 17, i32 13, metadata !43, null}
!43 = metadata !{i32 720907, metadata !13, i32 17, i32 13, metadata !6, i32 5} ; [ DW_TAG_lexical_block ]
!44 = metadata !{i32 17, i32 24, metadata !13, null}
!45 = metadata !{i32 17, i32 33, metadata !13, null}
!46 = metadata !{i32 18, i32 13, metadata !47, null}
!47 = metadata !{i32 720907, metadata !13, i32 18, i32 13, metadata !6, i32 6} ; [ DW_TAG_lexical_block ]
!48 = metadata !{i32 18, i32 24, metadata !13, null}
!49 = metadata !{i32 18, i32 33, metadata !13, null}
!50 = metadata !{i32 19, i32 13, metadata !51, null}
!51 = metadata !{i32 720907, metadata !13, i32 19, i32 13, metadata !6, i32 7} ; [ DW_TAG_lexical_block ]
!52 = metadata !{i32 19, i32 24, metadata !13, null}
!53 = metadata !{i32 19, i32 33, metadata !13, null}
!54 = metadata !{i32 20, i32 13, metadata !55, null}
!55 = metadata !{i32 720907, metadata !13, i32 20, i32 13, metadata !6, i32 8} ; [ DW_TAG_lexical_block ]
!56 = metadata !{i32 20, i32 24, metadata !13, null}
!57 = metadata !{i32 20, i32 33, metadata !13, null}
!58 = metadata !{i32 21, i32 13, metadata !59, null}
!59 = metadata !{i32 720907, metadata !13, i32 21, i32 13, metadata !6, i32 9} ; [ DW_TAG_lexical_block ]
!60 = metadata !{i32 21, i32 24, metadata !13, null}
!61 = metadata !{i32 21, i32 33, metadata !13, null}
!62 = metadata !{i32 22, i32 13, metadata !63, null}
!63 = metadata !{i32 720907, metadata !13, i32 22, i32 13, metadata !6, i32 10} ; [ DW_TAG_lexical_block ]
!64 = metadata !{i32 22, i32 24, metadata !13, null}
!65 = metadata !{i32 22, i32 33, metadata !13, null}
!66 = metadata !{i32 23, i32 13, metadata !67, null}
!67 = metadata !{i32 720907, metadata !13, i32 23, i32 13, metadata !6, i32 11} ; [ DW_TAG_lexical_block ]
!68 = metadata !{i32 23, i32 24, metadata !13, null}
!69 = metadata !{i32 23, i32 33, metadata !13, null}
!70 = metadata !{i32 24, i32 13, metadata !71, null}
!71 = metadata !{i32 720907, metadata !13, i32 24, i32 13, metadata !6, i32 12} ; [ DW_TAG_lexical_block ]
!72 = metadata !{i32 24, i32 24, metadata !13, null}
!73 = metadata !{i32 24, i32 34, metadata !13, null}
!74 = metadata !{i32 26, i32 13, metadata !13, null}
!75 = metadata !{i32 26, i32 24, metadata !13, null}
!76 = metadata !{i32 27, i32 13, metadata !13, null}
!77 = metadata !{i32 27, i32 24, metadata !78, null}
!78 = metadata !{i32 720907, metadata !13, i32 27, i32 24, metadata !6, i32 13} ; [ DW_TAG_lexical_block ]
!79 = metadata !{i32 27, i32 33, metadata !13, null}
!80 = metadata !{i32 28, i32 13, metadata !13, null}
!81 = metadata !{i32 28, i32 24, metadata !82, null}
!82 = metadata !{i32 720907, metadata !13, i32 28, i32 24, metadata !6, i32 14} ; [ DW_TAG_lexical_block ]
!83 = metadata !{i32 28, i32 33, metadata !13, null}
!84 = metadata !{i32 29, i32 13, metadata !13, null}
!85 = metadata !{i32 29, i32 24, metadata !86, null}
!86 = metadata !{i32 720907, metadata !13, i32 29, i32 24, metadata !6, i32 15} ; [ DW_TAG_lexical_block ]
!87 = metadata !{i32 29, i32 33, metadata !13, null}
!88 = metadata !{i32 30, i32 13, metadata !13, null}
!89 = metadata !{i32 30, i32 24, metadata !90, null}
!90 = metadata !{i32 720907, metadata !13, i32 30, i32 24, metadata !6, i32 16} ; [ DW_TAG_lexical_block ]
!91 = metadata !{i32 30, i32 33, metadata !13, null}
!92 = metadata !{i32 31, i32 13, metadata !13, null}
!93 = metadata !{i32 31, i32 24, metadata !94, null}
!94 = metadata !{i32 720907, metadata !13, i32 31, i32 24, metadata !6, i32 17} ; [ DW_TAG_lexical_block ]
!95 = metadata !{i32 31, i32 33, metadata !13, null}
!96 = metadata !{i32 32, i32 13, metadata !13, null}
!97 = metadata !{i32 32, i32 24, metadata !98, null}
!98 = metadata !{i32 720907, metadata !13, i32 32, i32 24, metadata !6, i32 18} ; [ DW_TAG_lexical_block ]
!99 = metadata !{i32 32, i32 33, metadata !13, null}
!100 = metadata !{i32 33, i32 13, metadata !13, null}
!101 = metadata !{i32 33, i32 24, metadata !102, null}
!102 = metadata !{i32 720907, metadata !13, i32 33, i32 24, metadata !6, i32 19} ; [ DW_TAG_lexical_block ]
!103 = metadata !{i32 33, i32 33, metadata !13, null}
!104 = metadata !{i32 34, i32 13, metadata !13, null}
!105 = metadata !{i32 34, i32 24, metadata !106, null}
!106 = metadata !{i32 720907, metadata !13, i32 34, i32 24, metadata !6, i32 20} ; [ DW_TAG_lexical_block ]
!107 = metadata !{i32 34, i32 33, metadata !13, null}
!108 = metadata !{i32 35, i32 13, metadata !13, null}
!109 = metadata !{i32 35, i32 24, metadata !110, null}
!110 = metadata !{i32 720907, metadata !13, i32 35, i32 24, metadata !6, i32 21} ; [ DW_TAG_lexical_block ]
!111 = metadata !{i32 35, i32 33, metadata !13, null}
!112 = metadata !{i32 36, i32 13, metadata !13, null}
!113 = metadata !{i32 36, i32 24, metadata !114, null}
!114 = metadata !{i32 720907, metadata !13, i32 36, i32 24, metadata !6, i32 22} ; [ DW_TAG_lexical_block ]
!115 = metadata !{i32 36, i32 33, metadata !13, null}
!116 = metadata !{i32 37, i32 13, metadata !13, null}
!117 = metadata !{i32 37, i32 24, metadata !118, null}
!118 = metadata !{i32 720907, metadata !13, i32 37, i32 24, metadata !6, i32 23} ; [ DW_TAG_lexical_block ]
!119 = metadata !{i32 37, i32 34, metadata !13, null}
!120 = metadata !{i32 39, i32 13, metadata !13, null}
!121 = metadata !{i32 40, i32 11, metadata !13, null}
!122 = metadata !{i32 40, i32 11, metadata !5, null}
