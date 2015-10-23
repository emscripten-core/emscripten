; ModuleID = '/tmp/tmpe4Pk1F/a.out.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

%"struct.std::__1::__atomic_base.0" = type { i8 }
%"struct.std::__1::__atomic_base" = type { %"struct.std::__1::__atomic_base.0" }
%"struct.std::__1::atomic" = type { %"struct.std::__1::__atomic_base" }

@.str = private unnamed_addr constant [8 x i8] c"ta2.cpp\00", align 1
@__PRETTY_FUNCTION__._Z7do_testIVNSt3__16atomicIcEEcEvv = private unnamed_addr constant [63 x i8] c"void do_test() [A = volatile std::__1::atomic<char>, T = char]\00", align 1
@.str1 = private unnamed_addr constant [43 x i8] c"obj.compare_exchange_weak(x, T(2)) == true\00", align 1
@.str2 = private unnamed_addr constant [12 x i8] c"obj == T(2)\00", align 1
@.str3 = private unnamed_addr constant [10 x i8] c"x == T(3)\00", align 1
@.str4 = private unnamed_addr constant [44 x i8] c"obj.compare_exchange_weak(x, T(1)) == false\00", align 1
@.str5 = private unnamed_addr constant [10 x i8] c"x == T(2)\00", align 1
@.str6 = private unnamed_addr constant [45 x i8] c"obj.compare_exchange_strong(x, T(1)) == true\00", align 1
@.str7 = private unnamed_addr constant [12 x i8] c"obj == T(1)\00", align 1
@.str8 = private unnamed_addr constant [15 x i8] c"hello, world!\0A\00", align 1 ; [#uses=1 type=[15 x i8]*]

define i32 @main() ssp {
entry:
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str8, i32 0, i32 0)) ; [#uses=0 type=i32]
  call void @_Z4testIVNSt3__16atomicIcEEcEvv()
  ret i32 0
}

define linkonce_odr void @_Z4testIVNSt3__16atomicIcEEcEvv() ssp {
entry:
  call void @_Z7do_testIVNSt3__16atomicIcEEcEvv()
  call void @_Z7do_testIVNSt3__16atomicIcEEcEvv()
  ret void
}

define linkonce_odr void @_Z7do_testIVNSt3__16atomicIcEEcEvv() ssp {
entry:
  %this.addr.i.i110 = alloca %"struct.std::__1::__atomic_base.0"*, align 4
  %__m.addr.i.i111 = alloca i32, align 4
  %.atomicdst.i.i112 = alloca i8, align 1
  %this.addr.i113 = alloca %"struct.std::__1::__atomic_base.0"*, align 4
  %this.addr.i90 = alloca %"struct.std::__1::__atomic_base.0"*, align 4
  %__e.addr.i91 = alloca i8*, align 4
  %__d.addr.i92 = alloca i8, align 1
  %__m.addr.i93 = alloca i32, align 4
  %.atomictmp.i94 = alloca i8, align 1
  %.atomicdst.i95 = alloca i8, align 1
  %this.addr.i.i79 = alloca %"struct.std::__1::__atomic_base.0"*, align 4
  %__m.addr.i.i80 = alloca i32, align 4
  %.atomicdst.i.i81 = alloca i8, align 1
  %this.addr.i82 = alloca %"struct.std::__1::__atomic_base.0"*, align 4
  %this.addr.i60 = alloca %"struct.std::__1::__atomic_base.0"*, align 4
  %__e.addr.i61 = alloca i8*, align 4
  %__d.addr.i62 = alloca i8, align 1
  %__m.addr.i63 = alloca i32, align 4
  %.atomictmp.i64 = alloca i8, align 1
  %.atomicdst.i65 = alloca i8, align 1
  %this.addr.i.i49 = alloca %"struct.std::__1::__atomic_base.0"*, align 4
  %__m.addr.i.i50 = alloca i32, align 4
  %.atomicdst.i.i51 = alloca i8, align 1
  %this.addr.i52 = alloca %"struct.std::__1::__atomic_base.0"*, align 4
  %this.addr.i46 = alloca %"struct.std::__1::__atomic_base.0"*, align 4
  %__e.addr.i = alloca i8*, align 4
  %__d.addr.i47 = alloca i8, align 1
  %__m.addr.i = alloca i32, align 4
  %.atomictmp.i = alloca i8, align 1
  %.atomicdst.i = alloca i8, align 1
  %this.addr.i.i42 = alloca %"struct.std::__1::__atomic_base.0"*, align 4
  %__m.addr.i.i = alloca i32, align 4
  %.atomicdst.i.i = alloca i8, align 1
  %this.addr.i43 = alloca %"struct.std::__1::__atomic_base.0"*, align 4
  %this.addr.i.i.i.i = alloca %"struct.std::__1::__atomic_base.0"*, align 4
  %__d.addr.i.i.i.i = alloca i8, align 1
  %this.addr.i.i.i = alloca %"struct.std::__1::__atomic_base"*, align 4
  %__d.addr.i.i.i = alloca i8, align 1
  %this.addr.i.i = alloca %"struct.std::__1::atomic"*, align 4
  %__d.addr.i.i = alloca i8, align 1
  %this.addr.i = alloca %"struct.std::__1::atomic"*, align 4
  %__d.addr.i = alloca i8, align 1
  %obj = alloca %"struct.std::__1::atomic", align 1
  %x = alloca i8, align 1
  store %"struct.std::__1::atomic"* %obj, %"struct.std::__1::atomic"** %this.addr.i, align 4
  store i8 0, i8* %__d.addr.i, align 1
  %this1.i = load %"struct.std::__1::atomic"*, %"struct.std::__1::atomic"** %this.addr.i
  %0 = load i8, i8* %__d.addr.i, align 1
  store %"struct.std::__1::atomic"* %this1.i, %"struct.std::__1::atomic"** %this.addr.i.i, align 4
  store i8 %0, i8* %__d.addr.i.i, align 1
  %this1.i.i = load %"struct.std::__1::atomic"*, %"struct.std::__1::atomic"** %this.addr.i.i
  %1 = bitcast %"struct.std::__1::atomic"* %this1.i.i to %"struct.std::__1::__atomic_base"*
  %2 = load i8, i8* %__d.addr.i.i, align 1
  store %"struct.std::__1::__atomic_base"* %1, %"struct.std::__1::__atomic_base"** %this.addr.i.i.i, align 4
  store i8 %2, i8* %__d.addr.i.i.i, align 1
  %this1.i.i.i = load %"struct.std::__1::__atomic_base"*, %"struct.std::__1::__atomic_base"** %this.addr.i.i.i
  %3 = bitcast %"struct.std::__1::__atomic_base"* %this1.i.i.i to %"struct.std::__1::__atomic_base.0"*
  %4 = load i8, i8* %__d.addr.i.i.i, align 1
  store %"struct.std::__1::__atomic_base.0"* %3, %"struct.std::__1::__atomic_base.0"** %this.addr.i.i.i.i, align 4
  store i8 %4, i8* %__d.addr.i.i.i.i, align 1
  %this1.i.i.i.i = load %"struct.std::__1::__atomic_base.0"*, %"struct.std::__1::__atomic_base.0"** %this.addr.i.i.i.i
  %__a_.i.i.i.i = getelementptr inbounds %"struct.std::__1::__atomic_base.0", %"struct.std::__1::__atomic_base.0"* %this1.i.i.i.i, i32 0, i32 0
  %5 = load i8, i8* %__d.addr.i.i.i.i, align 1
  store i8 %5, i8* %__a_.i.i.i.i, align 1
  %6 = bitcast %"struct.std::__1::atomic"* %obj to %"struct.std::__1::__atomic_base.0"*
  store %"struct.std::__1::__atomic_base.0"* %6, %"struct.std::__1::__atomic_base.0"** %this.addr.i113, align 4
  %this1.i114 = load %"struct.std::__1::__atomic_base.0"*, %"struct.std::__1::__atomic_base.0"** %this.addr.i113
  store %"struct.std::__1::__atomic_base.0"* %this1.i114, %"struct.std::__1::__atomic_base.0"** %this.addr.i.i110, align 4
  store i32 5, i32* %__m.addr.i.i111, align 4
  %this1.i.i115 = load %"struct.std::__1::__atomic_base.0"*, %"struct.std::__1::__atomic_base.0"** %this.addr.i.i110
  %__a_.i.i116 = getelementptr inbounds %"struct.std::__1::__atomic_base.0", %"struct.std::__1::__atomic_base.0"* %this1.i.i115, i32 0, i32 0
  %7 = load i32, i32* %__m.addr.i.i111, align 4
  switch i32 %7, label %monotonic.i.i117 [
    i32 1, label %acquire.i.i118
    i32 2, label %acquire.i.i118
    i32 5, label %seqcst.i.i119
  ]

monotonic.i.i117:                                 ; preds = %entry
  %8 = load atomic volatile i8, i8* %__a_.i.i116 monotonic, align 1
  store i8 %8, i8* %.atomicdst.i.i112, align 1
  br label %_ZNVKSt3__113__atomic_baseIcLb0EEcvcEv.exit120

acquire.i.i118:                                   ; preds = %entry, %entry
  %9 = load atomic volatile i8, i8* %__a_.i.i116 acquire, align 1
  store i8 %9, i8* %.atomicdst.i.i112, align 1
  br label %_ZNVKSt3__113__atomic_baseIcLb0EEcvcEv.exit120

seqcst.i.i119:                                    ; preds = %entry
  %10 = load atomic volatile i8, i8* %__a_.i.i116 seq_cst, align 1
  store i8 %10, i8* %.atomicdst.i.i112, align 1
  br label %_ZNVKSt3__113__atomic_baseIcLb0EEcvcEv.exit120

_ZNVKSt3__113__atomic_baseIcLb0EEcvcEv.exit120:   ; preds = %seqcst.i.i119, %acquire.i.i118, %monotonic.i.i117
  %11 = load i8, i8* %.atomicdst.i.i112
  store i8 %11, i8* %x, align 1
  %12 = bitcast %"struct.std::__1::atomic"* %obj to %"struct.std::__1::__atomic_base.0"*
  store %"struct.std::__1::__atomic_base.0"* %12, %"struct.std::__1::__atomic_base.0"** %this.addr.i90, align 4
  store i8* %x, i8** %__e.addr.i91, align 4
  store i8 2, i8* %__d.addr.i92, align 1
  store i32 5, i32* %__m.addr.i93, align 4
  %this1.i96 = load %"struct.std::__1::__atomic_base.0"*, %"struct.std::__1::__atomic_base.0"** %this.addr.i90
  %__a_.i97 = getelementptr inbounds %"struct.std::__1::__atomic_base.0", %"struct.std::__1::__atomic_base.0"* %this1.i96, i32 0, i32 0
  %13 = load i32, i32* %__m.addr.i93, align 4
  %14 = load i8*, i8** %__e.addr.i91, align 4
  %15 = load i8, i8* %__d.addr.i92, align 1
  store i8 %15, i8* %.atomictmp.i94
  %16 = load i32, i32* %__m.addr.i93, align 4
  switch i32 %13, label %monotonic.i99 [
    i32 1, label %acquire.i101
    i32 2, label %acquire.i101
    i32 3, label %release.i103
    i32 4, label %acqrel.i105
    i32 5, label %seqcst.i107
  ]

monotonic.i99:                                    ; preds = %_ZNVKSt3__113__atomic_baseIcLb0EEcvcEv.exit120
  %17 = load i8, i8* %14, align 1
  %18 = load i8, i8* %.atomictmp.i94, align 1
  %19 = cmpxchg volatile i8* %__a_.i97, i8 %17, i8 %18 monotonic monotonic
  %a19 = extractvalue { i8, i1 } %19, 0
  store i8 %a19, i8* %14, align 1
  %20 = icmp eq i8 %a19, %17
  %frombool.i98 = zext i1 %20 to i8
  store i8 %frombool.i98, i8* %.atomicdst.i95
  br label %_ZNVSt3__113__atomic_baseIcLb0EE21compare_exchange_weakERccNS_12memory_orderE.exit109

acquire.i101:                                     ; preds = %_ZNVKSt3__113__atomic_baseIcLb0EEcvcEv.exit120, %_ZNVKSt3__113__atomic_baseIcLb0EEcvcEv.exit120
  %21 = load i8, i8* %14, align 1
  %22 = load i8, i8* %.atomictmp.i94, align 1
  %23 = cmpxchg volatile i8* %__a_.i97, i8 %21, i8 %22 acquire acquire
  %a23 = extractvalue { i8, i1 } %23, 0
  store i8 %a23, i8* %14, align 1
  %24 = icmp eq i8 %a23, %21
  %frombool2.i100 = zext i1 %24 to i8
  store i8 %frombool2.i100, i8* %.atomicdst.i95
  br label %_ZNVSt3__113__atomic_baseIcLb0EE21compare_exchange_weakERccNS_12memory_orderE.exit109

release.i103:                                     ; preds = %_ZNVKSt3__113__atomic_baseIcLb0EEcvcEv.exit120
  %25 = load i8, i8* %14, align 1
  %26 = load i8, i8* %.atomictmp.i94, align 1
  %27 = cmpxchg volatile i8* %__a_.i97, i8 %25, i8 %26 release acquire
  %a27 = extractvalue { i8, i1 } %27, 0
  store i8 %a27, i8* %14, align 1
  %28 = icmp eq i8 %a27, %25
  %frombool3.i102 = zext i1 %28 to i8
  store i8 %frombool3.i102, i8* %.atomicdst.i95
  br label %_ZNVSt3__113__atomic_baseIcLb0EE21compare_exchange_weakERccNS_12memory_orderE.exit109

acqrel.i105:                                      ; preds = %_ZNVKSt3__113__atomic_baseIcLb0EEcvcEv.exit120
  %29 = load i8, i8* %14, align 1
  %30 = load i8, i8* %.atomictmp.i94, align 1
  %31 = cmpxchg volatile i8* %__a_.i97, i8 %29, i8 %30 acq_rel acquire
  %a31 = extractvalue { i8, i1 } %31, 0
  store i8 %a31, i8* %14, align 1
  %32 = icmp eq i8 %a31, %29
  %frombool4.i104 = zext i1 %32 to i8
  store i8 %frombool4.i104, i8* %.atomicdst.i95
  br label %_ZNVSt3__113__atomic_baseIcLb0EE21compare_exchange_weakERccNS_12memory_orderE.exit109

seqcst.i107:                                      ; preds = %_ZNVKSt3__113__atomic_baseIcLb0EEcvcEv.exit120
  %33 = load i8, i8* %14, align 1
  %34 = load i8, i8* %.atomictmp.i94, align 1
  %35 = cmpxchg volatile i8* %__a_.i97, i8 %33, i8 %34 seq_cst acquire
  %a35 = extractvalue { i8, i1 } %35, 0
  store i8 %a35, i8* %14, align 1
  %36 = icmp eq i8 %a35, %33
  %frombool5.i106 = zext i1 %36 to i8
  store i8 %frombool5.i106, i8* %.atomicdst.i95
  br label %_ZNVSt3__113__atomic_baseIcLb0EE21compare_exchange_weakERccNS_12memory_orderE.exit109

_ZNVSt3__113__atomic_baseIcLb0EE21compare_exchange_weakERccNS_12memory_orderE.exit109: ; preds = %seqcst.i107, %acqrel.i105, %release.i103, %acquire.i101, %monotonic.i99
  %37 = load i8, i8* %.atomicdst.i95
  %tobool.i108 = trunc i8 %37 to i1
  %conv = zext i1 %tobool.i108 to i32
  %cmp = icmp eq i32 %conv, 1
  br i1 %cmp, label %cond.true, label %cond.false

cond.true:                                        ; preds = %_ZNVSt3__113__atomic_baseIcLb0EE21compare_exchange_weakERccNS_12memory_orderE.exit109
  br label %cond.end

cond.false:                                       ; preds = %_ZNVSt3__113__atomic_baseIcLb0EE21compare_exchange_weakERccNS_12memory_orderE.exit109
  call void @__assert_func(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 21, i8* getelementptr inbounds ([63 x i8], [63 x i8]* @__PRETTY_FUNCTION__._Z7do_testIVNSt3__16atomicIcEEcEvv, i32 0, i32 0), i8* getelementptr inbounds ([43 x i8], [43 x i8]* @.str1, i32 0, i32 0))
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %38 = bitcast %"struct.std::__1::atomic"* %obj to %"struct.std::__1::__atomic_base.0"*
  store %"struct.std::__1::__atomic_base.0"* %38, %"struct.std::__1::__atomic_base.0"** %this.addr.i82, align 4
  %this1.i83 = load %"struct.std::__1::__atomic_base.0"*, %"struct.std::__1::__atomic_base.0"** %this.addr.i82
  store %"struct.std::__1::__atomic_base.0"* %this1.i83, %"struct.std::__1::__atomic_base.0"** %this.addr.i.i79, align 4
  store i32 5, i32* %__m.addr.i.i80, align 4
  %this1.i.i84 = load %"struct.std::__1::__atomic_base.0"*, %"struct.std::__1::__atomic_base.0"** %this.addr.i.i79
  %__a_.i.i85 = getelementptr inbounds %"struct.std::__1::__atomic_base.0", %"struct.std::__1::__atomic_base.0"* %this1.i.i84, i32 0, i32 0
  %39 = load i32, i32* %__m.addr.i.i80, align 4
  switch i32 %39, label %monotonic.i.i86 [
    i32 1, label %acquire.i.i87
    i32 2, label %acquire.i.i87
    i32 5, label %seqcst.i.i88
  ]

monotonic.i.i86:                                  ; preds = %cond.end
  %40 = load atomic volatile i8, i8* %__a_.i.i85 monotonic, align 1
  store i8 %40, i8* %.atomicdst.i.i81, align 1
  br label %_ZNVKSt3__113__atomic_baseIcLb0EEcvcEv.exit89

acquire.i.i87:                                    ; preds = %cond.end, %cond.end
  %41 = load atomic volatile i8, i8* %__a_.i.i85 acquire, align 1
  store i8 %41, i8* %.atomicdst.i.i81, align 1
  br label %_ZNVKSt3__113__atomic_baseIcLb0EEcvcEv.exit89

seqcst.i.i88:                                     ; preds = %cond.end
  %42 = load atomic volatile i8, i8* %__a_.i.i85 seq_cst, align 1
  store i8 %42, i8* %.atomicdst.i.i81, align 1
  br label %_ZNVKSt3__113__atomic_baseIcLb0EEcvcEv.exit89

_ZNVKSt3__113__atomic_baseIcLb0EEcvcEv.exit89:    ; preds = %seqcst.i.i88, %acquire.i.i87, %monotonic.i.i86
  %43 = load i8, i8* %.atomicdst.i.i81
  %conv3 = sext i8 %43 to i32
  %cmp4 = icmp eq i32 %conv3, 2
  br i1 %cmp4, label %cond.true5, label %cond.false6

cond.true5:                                       ; preds = %_ZNVKSt3__113__atomic_baseIcLb0EEcvcEv.exit89
  br label %cond.end7

cond.false6:                                      ; preds = %_ZNVKSt3__113__atomic_baseIcLb0EEcvcEv.exit89
  call void @__assert_func(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 22, i8* getelementptr inbounds ([63 x i8], [63 x i8]* @__PRETTY_FUNCTION__._Z7do_testIVNSt3__16atomicIcEEcEvv, i32 0, i32 0), i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str2, i32 0, i32 0))
  br label %cond.end7

cond.end7:                                        ; preds = %cond.false6, %cond.true5
  %44 = load i8, i8* %x, align 1
  %conv8 = sext i8 %44 to i32
  %cmp9 = icmp eq i32 %conv8, 3
  br i1 %cmp9, label %cond.true10, label %cond.false11

cond.true10:                                      ; preds = %cond.end7
  br label %cond.end12

cond.false11:                                     ; preds = %cond.end7
  call void @__assert_func(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 23, i8* getelementptr inbounds ([63 x i8], [63 x i8]* @__PRETTY_FUNCTION__._Z7do_testIVNSt3__16atomicIcEEcEvv, i32 0, i32 0), i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str3, i32 0, i32 0))
  br label %cond.end12

cond.end12:                                       ; preds = %cond.false11, %cond.true10
  %45 = bitcast %"struct.std::__1::atomic"* %obj to %"struct.std::__1::__atomic_base.0"*
  store %"struct.std::__1::__atomic_base.0"* %45, %"struct.std::__1::__atomic_base.0"** %this.addr.i60, align 4
  store i8* %x, i8** %__e.addr.i61, align 4
  store i8 1, i8* %__d.addr.i62, align 1
  store i32 5, i32* %__m.addr.i63, align 4
  %this1.i66 = load %"struct.std::__1::__atomic_base.0"*, %"struct.std::__1::__atomic_base.0"** %this.addr.i60
  %__a_.i67 = getelementptr inbounds %"struct.std::__1::__atomic_base.0", %"struct.std::__1::__atomic_base.0"* %this1.i66, i32 0, i32 0
  %46 = load i32, i32* %__m.addr.i63, align 4
  %47 = load i8*, i8** %__e.addr.i61, align 4
  %48 = load i8, i8* %__d.addr.i62, align 1
  store i8 %48, i8* %.atomictmp.i64
  %49 = load i32, i32* %__m.addr.i63, align 4
  switch i32 %46, label %monotonic.i69 [
    i32 1, label %acquire.i71
    i32 2, label %acquire.i71
    i32 3, label %release.i73
    i32 4, label %acqrel.i75
    i32 5, label %seqcst.i77
  ]

monotonic.i69:                                    ; preds = %cond.end12
  %50 = load i8, i8* %47, align 1
  %51 = load i8, i8* %.atomictmp.i64, align 1
  %52 = cmpxchg volatile i8* %__a_.i67, i8 %50, i8 %51 monotonic monotonic
  %a52 = extractvalue { i8, i1 } %52, 0
  store i8 %a52, i8* %47, align 1
  %53 = icmp eq i8 %a52, %50
  %frombool.i68 = zext i1 %53 to i8
  store i8 %frombool.i68, i8* %.atomicdst.i65
  br label %_ZNVSt3__113__atomic_baseIcLb0EE21compare_exchange_weakERccNS_12memory_orderE.exit

acquire.i71:                                      ; preds = %cond.end12, %cond.end12
  %54 = load i8, i8* %47, align 1
  %55 = load i8, i8* %.atomictmp.i64, align 1
  %56 = cmpxchg volatile i8* %__a_.i67, i8 %54, i8 %55 acquire acquire
  %a56 = extractvalue { i8, i1 } %56, 0
  store i8 %a56, i8* %47, align 1
  %57 = icmp eq i8 %a56, %54
  %frombool2.i70 = zext i1 %57 to i8
  store i8 %frombool2.i70, i8* %.atomicdst.i65
  br label %_ZNVSt3__113__atomic_baseIcLb0EE21compare_exchange_weakERccNS_12memory_orderE.exit

release.i73:                                      ; preds = %cond.end12
  %58 = load i8, i8* %47, align 1
  %59 = load i8, i8* %.atomictmp.i64, align 1
  %60 = cmpxchg volatile i8* %__a_.i67, i8 %58, i8 %59 release acquire
  %a60 = extractvalue { i8, i1 } %60, 0
  store i8 %a60, i8* %47, align 1
  %61 = icmp eq i8 %a60, %58
  %frombool3.i72 = zext i1 %61 to i8
  store i8 %frombool3.i72, i8* %.atomicdst.i65
  br label %_ZNVSt3__113__atomic_baseIcLb0EE21compare_exchange_weakERccNS_12memory_orderE.exit

acqrel.i75:                                       ; preds = %cond.end12
  %62 = load i8, i8* %47, align 1
  %63 = load i8, i8* %.atomictmp.i64, align 1
  %64 = cmpxchg volatile i8* %__a_.i67, i8 %62, i8 %63 acq_rel acquire
  %a64 = extractvalue { i8, i1 } %64, 0
  store i8 %a64, i8* %47, align 1
  %65 = icmp eq i8 %a64, %62
  %frombool4.i74 = zext i1 %65 to i8
  store i8 %frombool4.i74, i8* %.atomicdst.i65
  br label %_ZNVSt3__113__atomic_baseIcLb0EE21compare_exchange_weakERccNS_12memory_orderE.exit

seqcst.i77:                                       ; preds = %cond.end12
  %66 = load i8, i8* %47, align 1
  %67 = load i8, i8* %.atomictmp.i64, align 1
  %68 = cmpxchg volatile i8* %__a_.i67, i8 %66, i8 %67 seq_cst acquire
  %a68 = extractvalue { i8, i1 } %68, 0
  store i8 %a68, i8* %47, align 1
  %69 = icmp eq i8 %a68, %66
  %frombool5.i76 = zext i1 %69 to i8
  store i8 %frombool5.i76, i8* %.atomicdst.i65
  br label %_ZNVSt3__113__atomic_baseIcLb0EE21compare_exchange_weakERccNS_12memory_orderE.exit

_ZNVSt3__113__atomic_baseIcLb0EE21compare_exchange_weakERccNS_12memory_orderE.exit: ; preds = %seqcst.i77, %acqrel.i75, %release.i73, %acquire.i71, %monotonic.i69
  %70 = load i8, i8* %.atomicdst.i65
  %tobool.i78 = trunc i8 %70 to i1
  %conv14 = zext i1 %tobool.i78 to i32
  %cmp15 = icmp eq i32 %conv14, 0
  br i1 %cmp15, label %cond.true16, label %cond.false17

cond.true16:                                      ; preds = %_ZNVSt3__113__atomic_baseIcLb0EE21compare_exchange_weakERccNS_12memory_orderE.exit
  br label %cond.end18

cond.false17:                                     ; preds = %_ZNVSt3__113__atomic_baseIcLb0EE21compare_exchange_weakERccNS_12memory_orderE.exit
  call void @__assert_func(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 24, i8* getelementptr inbounds ([63 x i8], [63 x i8]* @__PRETTY_FUNCTION__._Z7do_testIVNSt3__16atomicIcEEcEvv, i32 0, i32 0), i8* getelementptr inbounds ([44 x i8], [44 x i8]* @.str4, i32 0, i32 0))
  br label %cond.end18

cond.end18:                                       ; preds = %cond.false17, %cond.true16
  %71 = bitcast %"struct.std::__1::atomic"* %obj to %"struct.std::__1::__atomic_base.0"*
  store %"struct.std::__1::__atomic_base.0"* %71, %"struct.std::__1::__atomic_base.0"** %this.addr.i52, align 4
  %this1.i53 = load %"struct.std::__1::__atomic_base.0"*, %"struct.std::__1::__atomic_base.0"** %this.addr.i52
  store %"struct.std::__1::__atomic_base.0"* %this1.i53, %"struct.std::__1::__atomic_base.0"** %this.addr.i.i49, align 4
  store i32 5, i32* %__m.addr.i.i50, align 4
  %this1.i.i54 = load %"struct.std::__1::__atomic_base.0"*, %"struct.std::__1::__atomic_base.0"** %this.addr.i.i49
  %__a_.i.i55 = getelementptr inbounds %"struct.std::__1::__atomic_base.0", %"struct.std::__1::__atomic_base.0"* %this1.i.i54, i32 0, i32 0
  %72 = load i32, i32* %__m.addr.i.i50, align 4
  switch i32 %72, label %monotonic.i.i56 [
    i32 1, label %acquire.i.i57
    i32 2, label %acquire.i.i57
    i32 5, label %seqcst.i.i58
  ]

monotonic.i.i56:                                  ; preds = %cond.end18
  %73 = load atomic volatile i8, i8* %__a_.i.i55 monotonic, align 1
  store i8 %73, i8* %.atomicdst.i.i51, align 1
  br label %_ZNVKSt3__113__atomic_baseIcLb0EEcvcEv.exit59

acquire.i.i57:                                    ; preds = %cond.end18, %cond.end18
  %74 = load atomic volatile i8, i8* %__a_.i.i55 acquire, align 1
  store i8 %74, i8* %.atomicdst.i.i51, align 1
  br label %_ZNVKSt3__113__atomic_baseIcLb0EEcvcEv.exit59

seqcst.i.i58:                                     ; preds = %cond.end18
  %75 = load atomic volatile i8, i8* %__a_.i.i55 seq_cst, align 1
  store i8 %75, i8* %.atomicdst.i.i51, align 1
  br label %_ZNVKSt3__113__atomic_baseIcLb0EEcvcEv.exit59

_ZNVKSt3__113__atomic_baseIcLb0EEcvcEv.exit59:    ; preds = %seqcst.i.i58, %acquire.i.i57, %monotonic.i.i56
  %76 = load i8, i8* %.atomicdst.i.i51
  %conv20 = sext i8 %76 to i32
  %cmp21 = icmp eq i32 %conv20, 2
  br i1 %cmp21, label %cond.true22, label %cond.false23

cond.true22:                                      ; preds = %_ZNVKSt3__113__atomic_baseIcLb0EEcvcEv.exit59
  br label %cond.end24

cond.false23:                                     ; preds = %_ZNVKSt3__113__atomic_baseIcLb0EEcvcEv.exit59
  call void @__assert_func(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 25, i8* getelementptr inbounds ([63 x i8], [63 x i8]* @__PRETTY_FUNCTION__._Z7do_testIVNSt3__16atomicIcEEcEvv, i32 0, i32 0), i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str2, i32 0, i32 0))
  br label %cond.end24

cond.end24:                                       ; preds = %cond.false23, %cond.true22
  %77 = load i8, i8* %x, align 1
  %conv25 = sext i8 %77 to i32
  %cmp26 = icmp eq i32 %conv25, 2
  br i1 %cmp26, label %cond.true27, label %cond.false28

cond.true27:                                      ; preds = %cond.end24
  br label %cond.end29

cond.false28:                                     ; preds = %cond.end24
  call void @__assert_func(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 26, i8* getelementptr inbounds ([63 x i8], [63 x i8]* @__PRETTY_FUNCTION__._Z7do_testIVNSt3__16atomicIcEEcEvv, i32 0, i32 0), i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str5, i32 0, i32 0))
  br label %cond.end29

cond.end29:                                       ; preds = %cond.false28, %cond.true27
  store i8 2, i8* %x, align 1
  %78 = bitcast %"struct.std::__1::atomic"* %obj to %"struct.std::__1::__atomic_base.0"*
  store %"struct.std::__1::__atomic_base.0"* %78, %"struct.std::__1::__atomic_base.0"** %this.addr.i46, align 4
  store i8* %x, i8** %__e.addr.i, align 4
  store i8 1, i8* %__d.addr.i47, align 1
  store i32 5, i32* %__m.addr.i, align 4
  %this1.i48 = load %"struct.std::__1::__atomic_base.0"*, %"struct.std::__1::__atomic_base.0"** %this.addr.i46
  %__a_.i = getelementptr inbounds %"struct.std::__1::__atomic_base.0", %"struct.std::__1::__atomic_base.0"* %this1.i48, i32 0, i32 0
  %79 = load i32, i32* %__m.addr.i, align 4
  %80 = load i8*, i8** %__e.addr.i, align 4
  %81 = load i8, i8* %__d.addr.i47, align 1
  store i8 %81, i8* %.atomictmp.i
  %82 = load i32, i32* %__m.addr.i, align 4
  switch i32 %79, label %monotonic.i [
    i32 1, label %acquire.i
    i32 2, label %acquire.i
    i32 3, label %release.i
    i32 4, label %acqrel.i
    i32 5, label %seqcst.i
  ]

monotonic.i:                                      ; preds = %cond.end29
  %83 = load i8, i8* %80, align 1
  %84 = load i8, i8* %.atomictmp.i, align 1
  %85 = cmpxchg volatile i8* %__a_.i, i8 %83, i8 %84 monotonic monotonic
  %a85 = extractvalue { i8, i1 } %85, 0
  store i8 %a85, i8* %80, align 1
  %86 = icmp eq i8 %a85, %83
  %frombool.i = zext i1 %86 to i8
  store i8 %frombool.i, i8* %.atomicdst.i
  br label %_ZNVSt3__113__atomic_baseIcLb0EE23compare_exchange_strongERccNS_12memory_orderE.exit

acquire.i:                                        ; preds = %cond.end29, %cond.end29
  %87 = load i8, i8* %80, align 1
  %88 = load i8, i8* %.atomictmp.i, align 1
  %89 = cmpxchg volatile i8* %__a_.i, i8 %87, i8 %88 acquire monotonic
  %a89 = extractvalue { i8, i1 } %89, 0
  store i8 %a89, i8* %80, align 1
  %90 = icmp eq i8 %a89, %87
  %frombool2.i = zext i1 %90 to i8
  store i8 %frombool2.i, i8* %.atomicdst.i
  br label %_ZNVSt3__113__atomic_baseIcLb0EE23compare_exchange_strongERccNS_12memory_orderE.exit

release.i:                                        ; preds = %cond.end29
  %91 = load i8, i8* %80, align 1
  %92 = load i8, i8* %.atomictmp.i, align 1
  %93 = cmpxchg volatile i8* %__a_.i, i8 %91, i8 %92 release acquire
  %a93 = extractvalue { i8, i1 } %93, 0
  store i8 %a93, i8* %80, align 1
  %94 = icmp eq i8 %a93, %91
  %frombool3.i = zext i1 %94 to i8
  store i8 %frombool3.i, i8* %.atomicdst.i
  br label %_ZNVSt3__113__atomic_baseIcLb0EE23compare_exchange_strongERccNS_12memory_orderE.exit

acqrel.i:                                         ; preds = %cond.end29
  %95 = load i8, i8* %80, align 1
  %96 = load i8, i8* %.atomictmp.i, align 1
  %97 = cmpxchg volatile i8* %__a_.i, i8 %95, i8 %96 acq_rel acquire
  %a97 = extractvalue { i8, i1 } %97, 0
  store i8 %a97, i8* %80, align 1
  %98 = icmp eq i8 %a97, %95
  %frombool4.i = zext i1 %98 to i8
  store i8 %frombool4.i, i8* %.atomicdst.i
  br label %_ZNVSt3__113__atomic_baseIcLb0EE23compare_exchange_strongERccNS_12memory_orderE.exit

seqcst.i:                                         ; preds = %cond.end29
  %99 = load i8, i8* %80, align 1
  %100 = load i8, i8* %.atomictmp.i, align 1
  %101 = cmpxchg volatile i8* %__a_.i, i8 %99, i8 %100 seq_cst acquire
  %a101 = extractvalue { i8, i1 } %101, 0
  store i8 %a101, i8* %80, align 1
  %102 = icmp eq i8 %a101, %99
  %frombool5.i = zext i1 %102 to i8
  store i8 %frombool5.i, i8* %.atomicdst.i
  br label %_ZNVSt3__113__atomic_baseIcLb0EE23compare_exchange_strongERccNS_12memory_orderE.exit

_ZNVSt3__113__atomic_baseIcLb0EE23compare_exchange_strongERccNS_12memory_orderE.exit: ; preds = %seqcst.i, %acqrel.i, %release.i, %acquire.i, %monotonic.i
  %103 = load i8, i8* %.atomicdst.i
  %tobool.i = trunc i8 %103 to i1
  %conv31 = zext i1 %tobool.i to i32
  %cmp32 = icmp eq i32 %conv31, 1
  br i1 %cmp32, label %cond.true33, label %cond.false34

cond.true33:                                      ; preds = %_ZNVSt3__113__atomic_baseIcLb0EE23compare_exchange_strongERccNS_12memory_orderE.exit
  br label %cond.end35

cond.false34:                                     ; preds = %_ZNVSt3__113__atomic_baseIcLb0EE23compare_exchange_strongERccNS_12memory_orderE.exit
  call void @__assert_func(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 28, i8* getelementptr inbounds ([63 x i8], [63 x i8]* @__PRETTY_FUNCTION__._Z7do_testIVNSt3__16atomicIcEEcEvv, i32 0, i32 0), i8* getelementptr inbounds ([45 x i8], [45 x i8]* @.str6, i32 0, i32 0))
  br label %cond.end35

cond.end35:                                       ; preds = %cond.false34, %cond.true33
  %104 = bitcast %"struct.std::__1::atomic"* %obj to %"struct.std::__1::__atomic_base.0"*
  store %"struct.std::__1::__atomic_base.0"* %104, %"struct.std::__1::__atomic_base.0"** %this.addr.i43, align 4
  %this1.i44 = load %"struct.std::__1::__atomic_base.0"*, %"struct.std::__1::__atomic_base.0"** %this.addr.i43
  store %"struct.std::__1::__atomic_base.0"* %this1.i44, %"struct.std::__1::__atomic_base.0"** %this.addr.i.i42, align 4
  store i32 5, i32* %__m.addr.i.i, align 4
  %this1.i.i45 = load %"struct.std::__1::__atomic_base.0"*, %"struct.std::__1::__atomic_base.0"** %this.addr.i.i42
  %__a_.i.i = getelementptr inbounds %"struct.std::__1::__atomic_base.0", %"struct.std::__1::__atomic_base.0"* %this1.i.i45, i32 0, i32 0
  %105 = load i32, i32* %__m.addr.i.i, align 4
  switch i32 %105, label %monotonic.i.i [
    i32 1, label %acquire.i.i
    i32 2, label %acquire.i.i
    i32 5, label %seqcst.i.i
  ]

monotonic.i.i:                                    ; preds = %cond.end35
  %106 = load atomic volatile i8, i8* %__a_.i.i monotonic, align 1
  store i8 %106, i8* %.atomicdst.i.i, align 1
  br label %_ZNVKSt3__113__atomic_baseIcLb0EEcvcEv.exit

acquire.i.i:                                      ; preds = %cond.end35, %cond.end35
  %107 = load atomic volatile i8, i8* %__a_.i.i acquire, align 1
  store i8 %107, i8* %.atomicdst.i.i, align 1
  br label %_ZNVKSt3__113__atomic_baseIcLb0EEcvcEv.exit

seqcst.i.i:                                       ; preds = %cond.end35
  %108 = load atomic volatile i8, i8* %__a_.i.i seq_cst, align 1
  store i8 %108, i8* %.atomicdst.i.i, align 1
  br label %_ZNVKSt3__113__atomic_baseIcLb0EEcvcEv.exit

_ZNVKSt3__113__atomic_baseIcLb0EEcvcEv.exit:      ; preds = %seqcst.i.i, %acquire.i.i, %monotonic.i.i
  %109 = load i8, i8* %.atomicdst.i.i
  %conv37 = sext i8 %109 to i32
  %cmp38 = icmp eq i32 %conv37, 1
  br i1 %cmp38, label %cond.true39, label %cond.false40

cond.true39:                                      ; preds = %_ZNVKSt3__113__atomic_baseIcLb0EEcvcEv.exit
  br label %cond.end41

cond.false40:                                     ; preds = %_ZNVKSt3__113__atomic_baseIcLb0EEcvcEv.exit
  call void @__assert_func(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 29, i8* getelementptr inbounds ([63 x i8], [63 x i8]* @__PRETTY_FUNCTION__._Z7do_testIVNSt3__16atomicIcEEcEvv, i32 0, i32 0), i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str7, i32 0, i32 0))
  br label %cond.end41

cond.end41:                                       ; preds = %cond.false40, %cond.true39
  ret void
}

declare void @__assert_func(i8*, i32, i8*, i8*)
declare i32 @printf(i8*, ...)

