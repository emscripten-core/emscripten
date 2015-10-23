; ModuleID = '/tmp/tmpVIBz29/a.out.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

@.str = private unnamed_addr constant [13 x i8] c"99\0A70\0A26\0A97\0A\00", align 1
@.str1 = private unnamed_addr constant [12 x i8] c"%u %u %u %u\00", align 1
@.str2 = private unnamed_addr constant [10 x i8] c"res = %u\0A\00", align 1

define i32 @main() nounwind {
  %jp0 = alloca i32, align 4
  %i1 = alloca i32, align 4
  %jq3 = alloca i32, align 4
  %i = alloca i32, align 4
  %cq = alloca [100 x i32], align 4
  %ye = alloca [100 x i32], align 4
  %g = alloca [100 x i32], align 4
  %z = alloca [100 x i32], align 4
  %za = alloca [100 x [100 x i32]], align 4
  %a0 = alloca [100 x i32], align 4
  store i32 99, i32* %jp0, align 4
  store i32 70, i32* %i1, align 4
  store i32 26, i32* %jq3, align 4
  store i32 97, i32* %i, align 4
  br label %.lr.ph.i

.lr.ph.i:                                         ; preds = %.lr.ph.i, %0
  %j.07.i = phi i32 [ %7, %.lr.ph.i ], [ 0, %0 ]
  %1 = and i32 %j.07.i, 1
  %2 = icmp eq i32 %1, 0
  %3 = sub i32 0, %j.07.i
  %.p.i = select i1 %2, i32 %j.07.i, i32 %3
  %4 = add i32 %.p.i, 2
  %5 = urem i32 %4, 101
  %6 = getelementptr inbounds [100 x i32], [100 x i32]* %cq, i32 0, i32 %j.07.i
  store i32 %5, i32* %6, align 4
  %7 = add i32 %j.07.i, 1
  %8 = icmp ult i32 %7, 100
  br i1 %8, label %.lr.ph.i, label %.lr.ph.i44.preheader

.lr.ph.i44.preheader:                             ; preds = %.lr.ph.i
  %9 = getelementptr inbounds [100 x i32], [100 x i32]* %a0, i32 0, i32 0
  br label %.lr.ph.i44

.lr.ph.i44:                                       ; preds = %.lr.ph.i44, %.lr.ph.i44.preheader
  %j.07.i42 = phi i32 [ %16, %.lr.ph.i44 ], [ 0, %.lr.ph.i44.preheader ]
  %10 = and i32 %j.07.i42, 1
  %11 = icmp eq i32 %10, 0
  %12 = sub i32 0, %j.07.i42
  %.p.i43 = select i1 %11, i32 %j.07.i42, i32 %12
  %13 = add i32 %.p.i43, 90
  %14 = urem i32 %13, 101
  %15 = getelementptr inbounds [100 x i32], [100 x i32]* %ye, i32 0, i32 %j.07.i42
  store i32 %14, i32* %15, align 4
  %16 = add i32 %j.07.i42, 1
  %17 = icmp ult i32 %16, 100
  br i1 %17, label %.lr.ph.i44, label %.lr.ph.i40

.lr.ph.i40:                                       ; preds = %.lr.ph.i40, %.lr.ph.i44
  %j.07.i38 = phi i32 [ %24, %.lr.ph.i40 ], [ 0, %.lr.ph.i44 ]
  %18 = and i32 %j.07.i38, 1
  %19 = icmp eq i32 %18, 0
  %20 = sub i32 0, %j.07.i38
  %.p.i39 = select i1 %19, i32 %j.07.i38, i32 %20
  %21 = add i32 %.p.i39, 73
  %22 = urem i32 %21, 101
  %23 = getelementptr inbounds [100 x i32], [100 x i32]* %g, i32 0, i32 %j.07.i38
  store i32 %22, i32* %23, align 4
  %24 = add i32 %j.07.i38, 1
  %25 = icmp ult i32 %24, 100
  br i1 %25, label %.lr.ph.i40, label %.lr.ph.i36

.lr.ph.i36:                                       ; preds = %.lr.ph.i36, %.lr.ph.i40
  %j.07.i34 = phi i32 [ %32, %.lr.ph.i36 ], [ 0, %.lr.ph.i40 ]
  %26 = and i32 %j.07.i34, 1
  %27 = icmp eq i32 %26, 0
  %28 = sub i32 0, %j.07.i34
  %.p.i35 = select i1 %27, i32 %j.07.i34, i32 %28
  %29 = add i32 %.p.i35, 54
  %30 = urem i32 %29, 101
  %31 = getelementptr inbounds [100 x i32], [100 x i32]* %z, i32 0, i32 %j.07.i34
  store i32 %30, i32* %31, align 4
  %32 = add i32 %j.07.i34, 1
  %33 = icmp ult i32 %32, 100
  br i1 %33, label %.lr.ph.i36, label %.lr.ph.i32

.lr.ph.i32:                                       ; preds = %.lr.ph.i32, %.lr.ph.i36
  %j.07.i30 = phi i32 [ %40, %.lr.ph.i32 ], [ 0, %.lr.ph.i36 ]
  %34 = and i32 %j.07.i30, 1
  %35 = icmp eq i32 %34, 0
  %36 = sub i32 0, %j.07.i30
  %.p.i31 = select i1 %35, i32 %j.07.i30, i32 %36
  %37 = add i32 %.p.i31, 66
  %38 = urem i32 %37, 101
  %39 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %za, i32 0, i32 0, i32 %j.07.i30
  store i32 %38, i32* %39, align 4
  %40 = add i32 %j.07.i30, 1
  %41 = icmp ult i32 %40, 10000
  br i1 %41, label %.lr.ph.i32, label %.lr.ph.i28

.lr.ph.i28:                                       ; preds = %.lr.ph.i28, %.lr.ph.i32
  %j.07.i26 = phi i32 [ %48, %.lr.ph.i28 ], [ 0, %.lr.ph.i32 ]
  %42 = and i32 %j.07.i26, 1
  %43 = icmp eq i32 %42, 0
  %44 = sub i32 0, %j.07.i26
  %.p.i27 = select i1 %43, i32 %j.07.i26, i32 %44
  %45 = add i32 %.p.i27, 71
  %46 = urem i32 %45, 101
  %47 = getelementptr inbounds [100 x i32], [100 x i32]* %a0, i32 0, i32 %j.07.i26
  store i32 %46, i32* %47, align 4
  %48 = add i32 %j.07.i26, 1
  %49 = icmp ult i32 %48, 100
  br i1 %49, label %.lr.ph.i28, label %init.exit29

init.exit29:                                      ; preds = %.lr.ph.i28
  %50 = call i32 (i8*, i8*, ...) @sscanf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str1, i32 0, i32 0), i32* %jp0, i32* %i1, i32* %jq3, i32* %i) nounwind
  %51 = getelementptr inbounds [100 x i32], [100 x i32]* %cq, i32 0, i32 46
  %52 = load i32, i32* %51, align 4
  %53 = getelementptr inbounds [100 x i32], [100 x i32]* %cq, i32 0, i32 20
  %54 = load i32, i32* %53, align 4
  %55 = icmp ult i32 %52, %54
  br i1 %55, label %.preheader61, label %56

; <label>:56                                      ; preds = %init.exit29
  %57 = load i32, i32* %9, align 4
  %58 = getelementptr inbounds [100 x i32], [100 x i32]* %ye, i32 0, i32 24
  %59 = load i32, i32* %58, align 4
  %60 = sub i32 %59, %57
  store i32 %60, i32* %58, align 4
  br label %.preheader61

.preheader61:                                     ; preds = %56, %init.exit29
  store i32 2, i32* %jp0, align 4
  %.phi.trans.insert = getelementptr inbounds [100 x i32], [100 x i32]* %cq, i32 0, i32 1
  %.pre = load i32, i32* %.phi.trans.insert, align 4
  br label %61

.preheader58:                                     ; preds = %61
  store i32 80, i32* %jp0, align 4
  store i32 94, i32* %i1, align 4
  br label %76

; <label>:61                                      ; preds = %61, %.preheader61
  %62 = phi i32 [ %.pre, %.preheader61 ], [ %66, %61 ]
  %63 = phi i32 [ 2, %.preheader61 ], [ %71, %61 ]
  %64 = getelementptr inbounds [100 x i32], [100 x i32]* %cq, i32 0, i32 %63
  %65 = load i32, i32* %64, align 4
  %66 = add i32 %65, -4
  store i32 %66, i32* %64, align 4
  %67 = add i32 %63, -1
  %68 = getelementptr inbounds [100 x i32], [100 x i32]* %ye, i32 0, i32 %67
  %69 = load i32, i32* %68, align 4
  %70 = mul i32 %62, %69
  %71 = add i32 %63, 1
  %72 = getelementptr inbounds [100 x i32], [100 x i32]* %g, i32 0, i32 %71
  %73 = load i32, i32* %72, align 4
  %74 = sub i32 %73, %70
  store i32 %74, i32* %72, align 4
  %75 = icmp ult i32 %71, 80
  br i1 %75, label %61, label %.preheader58

.preheader55:                                     ; preds = %76
  store i32 2, i32* %i1, align 4
  store i32 44, i32* %jq3, align 4
  br label %.preheader

; <label>:76                                      ; preds = %76, %.preheader58
  %77 = phi i32 [ 94, %.preheader58 ], [ %80, %76 ]
  %78 = getelementptr inbounds [100 x i32], [100 x i32]* %ye, i32 0, i32 %77
  %79 = load i32, i32* %78, align 4
  %80 = add i32 %77, -1
  %81 = getelementptr inbounds [100 x i32], [100 x i32]* %z, i32 0, i32 %80
  store i32 %79, i32* %81, align 4
  %82 = icmp ugt i32 %80, 2
  br i1 %82, label %76, label %.preheader55

.preheader:                                       ; preds = %95, %.preheader55
  %83 = phi i32 [ 44, %.preheader55 ], [ %84, %95 ]
  %84 = add i32 %83, -1
  %85 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %za, i32 0, i32 %84, i32 %83
  %.promoted = load i32, i32* %85, align 4
  %.pre75 = load i32, i32* %9, align 4
  %.phi.trans.insert76 = getelementptr inbounds [100 x i32], [100 x i32]* %a0, i32 0, i32 1
  %.pre77 = load i32, i32* %.phi.trans.insert76, align 4
  br label %86

; <label>:86                                      ; preds = %86, %.preheader
  %87 = phi i32 [ %.pre77, %.preheader ], [ %88, %86 ]
  %88 = phi i32 [ %.pre75, %.preheader ], [ %87, %86 ]
  %89 = phi i32 [ 1, %.preheader ], [ %92, %86 ]
  %90 = phi i32 [ %.promoted, %.preheader ], [ %91, %86 ]
  %91 = mul i32 %90, %87
  %92 = add i32 %89, 1
  %93 = getelementptr inbounds [100 x i32], [100 x i32]* %a0, i32 0, i32 %92
  store i32 %88, i32* %93, align 4
  %94 = icmp ult i32 %92, 46
  br i1 %94, label %86, label %95

; <label>:95                                      ; preds = %86
  store i32 %91, i32* %85, align 4
  %96 = icmp ugt i32 %84, 1
  br i1 %96, label %.preheader, label %97

; <label>:97                                      ; preds = %95
  store i32 1, i32* %jq3, align 4
  store i32 46, i32* %i, align 4
  br label %.lr.ph.i24

.lr.ph.i24:                                       ; preds = %.lr.ph.i24, %97
  %sum.07.i22 = phi i32 [ %104, %.lr.ph.i24 ], [ 0, %97 ]
  %j.06.i23 = phi i32 [ %105, %.lr.ph.i24 ], [ 0, %97 ]
  %98 = and i32 %j.06.i23, 1
  %99 = icmp eq i32 %98, 0
  %100 = getelementptr inbounds [100 x i32], [100 x i32]* %cq, i32 0, i32 %j.06.i23
  %101 = load i32, i32* %100, align 4
  %102 = sub i32 0, %101
  %103 = select i1 %99, i32 %101, i32 %102
  %104 = add i32 %103, %sum.07.i22
  %105 = add i32 %j.06.i23, 1
  %106 = icmp ult i32 %105, 100
  br i1 %106, label %.lr.ph.i24, label %.lr.ph.i20

.lr.ph.i20:                                       ; preds = %.lr.ph.i20, %.lr.ph.i24
  %sum.07.i18 = phi i32 [ %113, %.lr.ph.i20 ], [ 0, %.lr.ph.i24 ]
  %j.06.i19 = phi i32 [ %114, %.lr.ph.i20 ], [ 0, %.lr.ph.i24 ]
  %107 = and i32 %j.06.i19, 1
  %108 = icmp eq i32 %107, 0
  %109 = getelementptr inbounds [100 x i32], [100 x i32]* %ye, i32 0, i32 %j.06.i19
  %110 = load i32, i32* %109, align 4
  %111 = sub i32 0, %110
  %112 = select i1 %108, i32 %110, i32 %111
  %113 = add i32 %112, %sum.07.i18
  %114 = add i32 %j.06.i19, 1
  %115 = icmp ult i32 %114, 100
  br i1 %115, label %.lr.ph.i20, label %.lr.ph.i16

.lr.ph.i16:                                       ; preds = %.lr.ph.i16, %.lr.ph.i20
  %sum.07.i14 = phi i32 [ %122, %.lr.ph.i16 ], [ 0, %.lr.ph.i20 ]
  %j.06.i15 = phi i32 [ %123, %.lr.ph.i16 ], [ 0, %.lr.ph.i20 ]
  %116 = and i32 %j.06.i15, 1
  %117 = icmp eq i32 %116, 0
  %118 = getelementptr inbounds [100 x i32], [100 x i32]* %g, i32 0, i32 %j.06.i15
  %119 = load i32, i32* %118, align 4
  %120 = sub i32 0, %119
  %121 = select i1 %117, i32 %119, i32 %120
  %122 = add i32 %121, %sum.07.i14
  %123 = add i32 %j.06.i15, 1
  %124 = icmp ult i32 %123, 100
  br i1 %124, label %.lr.ph.i16, label %.lr.ph.i12

.lr.ph.i12:                                       ; preds = %.lr.ph.i12, %.lr.ph.i16
  %sum.07.i10 = phi i32 [ %131, %.lr.ph.i12 ], [ 0, %.lr.ph.i16 ]
  %j.06.i11 = phi i32 [ %132, %.lr.ph.i12 ], [ 0, %.lr.ph.i16 ]
  %125 = and i32 %j.06.i11, 1
  %126 = icmp eq i32 %125, 0
  %127 = getelementptr inbounds [100 x i32], [100 x i32]* %z, i32 0, i32 %j.06.i11
  %128 = load i32, i32* %127, align 4
  %129 = sub i32 0, %128
  %130 = select i1 %126, i32 %128, i32 %129
  %131 = add i32 %130, %sum.07.i10
  %132 = add i32 %j.06.i11, 1
  %133 = icmp ult i32 %132, 100
  br i1 %133, label %.lr.ph.i12, label %.lr.ph.i8

.lr.ph.i8:                                        ; preds = %.lr.ph.i8, %.lr.ph.i12
  %sum.07.i6 = phi i32 [ %140, %.lr.ph.i8 ], [ 0, %.lr.ph.i12 ]
  %j.06.i7 = phi i32 [ %141, %.lr.ph.i8 ], [ 0, %.lr.ph.i12 ]
  %134 = and i32 %j.06.i7, 1
  %135 = icmp eq i32 %134, 0
  %136 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %za, i32 0, i32 0, i32 %j.06.i7
  %137 = load i32, i32* %136, align 4
  %138 = sub i32 0, %137
  %139 = select i1 %135, i32 %137, i32 %138
  %140 = add i32 %139, %sum.07.i6
  %141 = add i32 %j.06.i7, 1
  %142 = icmp ult i32 %141, 10000
  br i1 %142, label %.lr.ph.i8, label %.lr.ph.i5

.lr.ph.i5:                                        ; preds = %.lr.ph.i5, %.lr.ph.i8
  %sum.07.i = phi i32 [ %149, %.lr.ph.i5 ], [ 0, %.lr.ph.i8 ]
  %j.06.i = phi i32 [ %150, %.lr.ph.i5 ], [ 0, %.lr.ph.i8 ]
  %143 = and i32 %j.06.i, 1
  %144 = icmp eq i32 %143, 0
  %145 = getelementptr inbounds [100 x i32], [100 x i32]* %a0, i32 0, i32 %j.06.i
  %146 = load i32, i32* %145, align 4
  %147 = sub i32 0, %146
  %148 = select i1 %144, i32 %146, i32 %147
  %149 = add i32 %148, %sum.07.i
  %150 = add i32 %j.06.i, 1
  %151 = icmp ult i32 %150, 100
  br i1 %151, label %.lr.ph.i5, label %checkSum.exit

checkSum.exit:                                    ; preds = %.lr.ph.i5
  %152 = add i32 %104, 82
  %153 = add i32 %152, %113
  %154 = sub i32 %153, %122
  %155 = add i32 %154, %131
  %156 = sub i32 %155, %140
  %157 = add i32 %156, %149
  %158 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str2, i32 0, i32 0), i32 %157) nounwind
  ret i32 0
}

declare i32 @sscanf(i8* nocapture, i8* nocapture, ...) nounwind

declare i32 @printf(i8* nocapture, ...) nounwind
