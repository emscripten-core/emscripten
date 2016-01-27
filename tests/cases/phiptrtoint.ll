; ModuleID = '/tmp/tmpJctwj0/bug.bc'
; just an asm validation check, no output
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

%"class.test::Processor" = type { i32, %"class.test::StateMachine" }
%"class.test::StateMachine" = type { { i32, i32 } }

@_ZN4test9ProcessorC1Ev = internal alias void (%"class.test::Processor"*), void (%"class.test::Processor"*)* @_ZN4test9ProcessorC2Ev
@_ZN4test9ProcessorD1Ev = internal alias void (%"class.test::Processor"*), void (%"class.test::Processor"*)* @_ZN4test9ProcessorD2Ev

define internal void @_ZN4test9ProcessorC2Ev(%"class.test::Processor"* nocapture %this) unnamed_addr nounwind align 2 {
  %1 = getelementptr inbounds %"class.test::Processor", %"class.test::Processor"* %this, i32 0, i32 0
  store i32 0, i32* %1, align 4
  %2 = getelementptr inbounds %"class.test::Processor", %"class.test::Processor"* %this, i32 0, i32 1, i32 0
  store { i32, i32 } zeroinitializer, { i32, i32 }* %2, align 4
  ret void
}

define internal void @_ZN4test9ProcessorD2Ev(%"class.test::Processor"* nocapture %this) unnamed_addr nounwind readnone align 2 {
  ret void
}

define internal zeroext i1 @_ZN4test9Processor16handleFirstStateEv(%"class.test::Processor"* nocapture %this) align 2 {
  %1 = tail call i32 @rand()
  %2 = getelementptr inbounds %"class.test::Processor", %"class.test::Processor"* %this, i32 0, i32 0
  %3 = load i32, i32* %2, align 4
  %4 = add nsw i32 %3, %1
  store i32 %4, i32* %2, align 4
  %5 = and i32 %4, 1
  %6 = icmp eq i32 %5, 0
  ret i1 %6
}

declare i32 @rand()

define internal zeroext i1 @_ZN4test9Processor15handleLastStateEv(%"class.test::Processor"* nocapture %this) align 2 {
  %1 = tail call i32 @rand()
  %2 = getelementptr inbounds %"class.test::Processor", %"class.test::Processor"* %this, i32 0, i32 0
  %3 = load i32, i32* %2, align 4
  %4 = add nsw i32 %3, %1
  store i32 %4, i32* %2, align 4
  ret i1 true
}

define internal zeroext i1 @_ZN4test9Processor3runEv(%"class.test::Processor"* %this) align 2 {
  %1 = getelementptr inbounds %"class.test::Processor", %"class.test::Processor"* %this, i32 0, i32 1, i32 0
  store { i32, i32 } { i32 ptrtoint (i1 (%"class.test::Processor"*)* @_ZN4test9Processor16handleFirstStateEv to i32), i32 0 }, { i32, i32 }* %1, align 4
  %2 = bitcast %"class.test::Processor"* %this to i8*
  br label %.backedge

.backedge:                                        ; preds = %25, %..backedge_crit_edge, %0
  %3 = phi { i32, i32 } [ { i32 ptrtoint (i1 (%"class.test::Processor"*)* @_ZN4test9Processor16handleFirstStateEv to i32), i32 0 }, %0 ], [ %.pre.pre, %..backedge_crit_edge ], [ { i32 ptrtoint (i1 (%"class.test::Processor"*)* @_ZN4test9Processor15handleLastStateEv to i32), i32 0 }, %25 ]
  %.fca.0.extract = extractvalue { i32, i32 } %3, 0
  %.fca.1.extract = extractvalue { i32, i32 } %3, 1
  %4 = icmp ne i32 %.fca.0.extract, ptrtoint (i1 (%"class.test::Processor"*)* @_ZN4test9Processor15handleLastStateEv to i32)
  %5 = icmp ne i32 %.fca.0.extract, 0
  %6 = icmp ne i32 %.fca.1.extract, 0
  %7 = and i1 %5, %6
  %8 = or i1 %4, %7
  %9 = getelementptr inbounds i8, i8* %2, i32 %.fca.1.extract
  %10 = bitcast i8* %9 to %"class.test::Processor"*
  %11 = and i32 %.fca.0.extract, 1
  %12 = icmp eq i32 %11, 0
  br i1 %12, label %20, label %13

; <label>:13                                      ; preds = %.backedge
  %14 = bitcast i8* %9 to i8**
  %15 = load i8*, i8** %14, align 4
  %16 = add i32 %.fca.0.extract, -1
  %17 = getelementptr i8, i8* %15, i32 %16
  %18 = bitcast i8* %17 to i1 (%"class.test::Processor"*)**
  %19 = load i1 (%"class.test::Processor"*)*, i1 (%"class.test::Processor"*)** %18, align 4
  br label %_ZN4test12StateMachineINS_9ProcessorEE11handleStateEPS1_.exit

; <label>:20                                      ; preds = %.backedge
  %21 = inttoptr i32 %.fca.0.extract to i1 (%"class.test::Processor"*)*
  br label %_ZN4test12StateMachineINS_9ProcessorEE11handleStateEPS1_.exit

_ZN4test12StateMachineINS_9ProcessorEE11handleStateEPS1_.exit: ; preds = %20, %13
  %22 = phi i1 (%"class.test::Processor"*)* [ %19, %13 ], [ %21, %20 ]
  %23 = tail call zeroext i1 %22(%"class.test::Processor"* %10)
  br i1 %8, label %24, label %26

; <label>:24                                      ; preds = %_ZN4test12StateMachineINS_9ProcessorEE11handleStateEPS1_.exit
  br i1 %23, label %25, label %..backedge_crit_edge

..backedge_crit_edge:                             ; preds = %24
  %.pre.pre = load { i32, i32 }, { i32, i32 }* %1, align 4
  br label %.backedge

; <label>:25                                      ; preds = %24
  store { i32, i32 } { i32 ptrtoint (i1 (%"class.test::Processor"*)* @_ZN4test9Processor15handleLastStateEv to i32), i32 0 }, { i32, i32 }* %1, align 4
  br label %.backedge

; <label>:26                                      ; preds = %_ZN4test12StateMachineINS_9ProcessorEE11handleStateEPS1_.exit
  ret i1 %23
}

define internal i32 @_ZNK4test9Processor6resultEv(%"class.test::Processor"* nocapture %this) nounwind readonly align 2 {
  %1 = getelementptr inbounds %"class.test::Processor", %"class.test::Processor"* %this, i32 0, i32 0
  %2 = load i32, i32* %1, align 4
  ret i32 %2
}

define i32 @runProcess() personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %processor = alloca %"class.test::Processor", align 4
  call void @_ZN4test9ProcessorC1Ev(%"class.test::Processor"* %processor)
  %1 = invoke zeroext i1 @_ZN4test9Processor3runEv(%"class.test::Processor"* %processor)
          to label %2 unwind label %5

; <label>:2                                       ; preds = %0
  %3 = invoke i32 @_ZNK4test9Processor6resultEv(%"class.test::Processor"* %processor)
          to label %4 unwind label %5

; <label>:4                                       ; preds = %2
  call void @_ZN4test9ProcessorD1Ev(%"class.test::Processor"* %processor)
  ret i32 %3

; <label>:5                                       ; preds = %2, %0
  %6 = landingpad { i8*, i32 }
          cleanup
  invoke void @_ZN4test9ProcessorD1Ev(%"class.test::Processor"* %processor)
          to label %7 unwind label %8

; <label>:7                                       ; preds = %5
  resume { i8*, i32 } %6

; <label>:8                                       ; preds = %5
  %9 = landingpad { i8*, i32 }
          catch i8* null
  call void @_ZSt9terminatev() noreturn nounwind
  unreachable
}

declare i32 @__gxx_personality_v0(...)

declare void @_ZSt9terminatev()
