; ModuleID = 'emcc-0-basebc.bc'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:128-n32-S128"
target triple = "asmjs-unknown-emscripten"

; Tests i24 constantExprs nested heavily

%struct.S0 = type { i16, i8, i32 }
%struct.malloc_state = type { i32, i32, i32, i32, i8*, %struct.malloc_chunk*, %struct.malloc_chunk*, i32, i32, i32, [66 x %struct.malloc_chunk*], [32 x %struct.malloc_tree_chunk*], i32, i32, i32, i32, %struct.malloc_segment, i8*, i32 }
%struct.malloc_chunk = type { i32, i32, %struct.malloc_chunk*, %struct.malloc_chunk* }
%struct.malloc_tree_chunk = type { i32, i32, %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk*, [2 x %struct.malloc_tree_chunk*], %struct.malloc_tree_chunk*, i32 }
%struct.malloc_segment = type { i8*, i32, %struct.malloc_segment*, i32 }
%struct.malloc_params = type { i32, i32, i32, i32, i32, i32 }
%struct._IO_FILE = type { i32, i8*, i8*, i32 (%struct._IO_FILE*)*, i8*, i8*, i8*, i8*, i32 (%struct._IO_FILE*, i8*, i32)*, i32 (%struct._IO_FILE*, i8*, i32)*, i32 (%struct._IO_FILE*, i32, i32)*, i8*, i32, %struct._IO_FILE*, %struct._IO_FILE*, i32, i32, i32, i16, i8, i8, i32, i32, i8*, i32, i8*, i8*, i8*, i32, i32 }
%"struct.std::nothrow_t" = type { i8 }
%"class.std::bad_alloc" = type { %"class.std::exception" }
%"class.std::exception" = type { i32 (...)** }
%"class.std::bad_array_new_length" = type { %"class.std::bad_alloc" }
%"class.std::bad_array_length" = type { %"class.std::bad_alloc" }
%struct.__locale_struct = type opaque
%struct.__locale_struct.0 = type opaque
%struct.__locale_struct.1 = type opaque
%union.U1 = type { i8* }
%struct.mallinfo = type { i32, i32, i32, i32, i32, i32, i32, i32, i32, i32 }
%struct._IO_FILE.3 = type { i32, i8*, i8*, {}*, i8*, i8*, i8*, i8*, i32 (%struct._IO_FILE.3*, i8*, i32)*, i32 (%struct._IO_FILE.3*, i8*, i32)*, i32 (%struct._IO_FILE.3*, i32, i32)*, i8*, i32, %struct._IO_FILE.3*, %struct._IO_FILE.3*, i32, i32, i32, i16, i8, i8, i32, i32, i8*, i32, i8*, i8*, i8*, i32, i32 }
%struct._IO_FILE.4 = type { i32, i8*, i8*, {}*, i8*, i8*, i8*, i8*, i32 (%struct._IO_FILE.4*, i8*, i32)*, i32 (%struct._IO_FILE.4*, i8*, i32)*, i32 (%struct._IO_FILE.4*, i32, i32)*, i8*, i32, %struct._IO_FILE.4*, %struct._IO_FILE.4*, i32, i32, i32, i16, i8, i8, i32, i32, i8*, i32, i8*, i8*, i8*, i32, i32 }

@.str = private unnamed_addr constant [2 x i8] c"1\00", align 1
@_ZL3g_2 = internal global i32 -8, align 4
@.str1 = private unnamed_addr constant [4 x i8] c"g_2\00", align 1
@_ZL4g_52 = internal global [4 x [6 x [6 x i8]]] [[6 x [6 x i8]] [[6 x i8] c"\00\02Q\07\07Q", [6 x i8] c"\09\09\8F\01\00\00", [6 x i8] c"\FFO\09\07\00\8F", [6 x i8] c"\D1\FF\09\02\09\00", [6 x i8] c"\FD\02\8FE\FFQ", [6 x i8] c"E\FFQ\01\D1\D1"], [6 x [6 x i8]] [[6 x i8] c"EOOE\FD\8F", [6 x i8] c"\FD\09\01\02E\D1", [6 x i8] c"\D1\02M\07EQ", [6 x i8] c"\FF\09\01\01\FD\00", [6 x i8] c"\09O\02\07\D1\8F", [6 x i8] c"\00\FF\02\02\FF\00"], [6 x [6 x i8]] [[6 x i8] c"\00\02\01E\09Q", [6 x i8] c"\07\FFM\01\00\D1", [6 x i8] c"\07O\01E\00\8F", [6 x i8] c"\00\09O\02\07\D1", [6 x i8] c"\00\02Q\07\07Q", [6 x i8] c"\09\09\8F\01\00\00"], [6 x [6 x i8]] [[6 x i8] c"\FFO\09\07\00\8F", [6 x i8] c"\D1\FF\09\02\09\00", [6 x i8] c"\FD\02\8FE\FFQ", [6 x i8] c"E\FFQ\01\D1\D1", [6 x i8] c"EOOE\FD\8F", [6 x i8] c"\FD\09\01\02E\D1"]], align 1
@.str2 = private unnamed_addr constant [14 x i8] c"g_52[i][j][k]\00", align 1
@.str3 = private unnamed_addr constant [22 x i8] c"index = [%d][%d][%d]\0A\00", align 1
@_ZL4g_53 = internal global i32 -1550080695, align 4
@.str4 = private unnamed_addr constant [5 x i8] c"g_53\00", align 1
@.str5 = private unnamed_addr constant [8 x i8] c"g_74.f0\00", align 1
@.str6 = private unnamed_addr constant [8 x i8] c"g_74.f4\00", align 1
@_ZL4g_77 = internal global i16 1, align 2
@.str7 = private unnamed_addr constant [5 x i8] c"g_77\00", align 1
@_ZL4g_79 = internal global i16 -1, align 2
@.str8 = private unnamed_addr constant [5 x i8] c"g_79\00", align 1
@_ZL4g_82 = internal global i16 -4597, align 2
@.str9 = private unnamed_addr constant [5 x i8] c"g_82\00", align 1
@_ZL4g_83 = internal global %struct.S0 { i16 -20724, i8 -72, i32 1 }, align 8
@.str10 = private unnamed_addr constant [8 x i8] c"g_83.f0\00", align 1
@.str11 = private unnamed_addr constant [8 x i8] c"g_83.f1\00", align 1
@.str12 = private unnamed_addr constant [8 x i8] c"g_83.f2\00", align 1
@_ZL4g_93 = internal global i32 -911546352, align 4
@.str13 = private unnamed_addr constant [5 x i8] c"g_93\00", align 1
@_ZL5g_103 = internal global i16 -1, align 2
@.str14 = private unnamed_addr constant [6 x i8] c"g_103\00", align 1
@_ZL5g_126 = internal global i8 6, align 1
@.str15 = private unnamed_addr constant [6 x i8] c"g_126\00", align 1
@_ZL5g_132 = internal global i32 -782244254, align 4
@.str16 = private unnamed_addr constant [6 x i8] c"g_132\00", align 1
@_ZL5g_133 = internal unnamed_addr global i8 -17, align 1
@.str17 = private unnamed_addr constant [6 x i8] c"g_133\00", align 1
@_ZL5g_135 = internal global i32 -3, align 4
@.str18 = private unnamed_addr constant [6 x i8] c"g_135\00", align 1
@_ZL5g_136 = internal global i8 -1, align 1
@.str19 = private unnamed_addr constant [6 x i8] c"g_136\00", align 1
@_ZL5g_138 = internal global i32 1083335064, align 4
@.str20 = private unnamed_addr constant [6 x i8] c"g_138\00", align 1
@_ZL5g_162 = internal global i16 17495, align 2
@.str21 = private unnamed_addr constant [6 x i8] c"g_162\00", align 1
@_ZL5g_168 = internal global i16 8, align 2
@.str22 = private unnamed_addr constant [6 x i8] c"g_168\00", align 1
@_ZL5g_170 = internal global i8 -109, align 1
@.str23 = private unnamed_addr constant [6 x i8] c"g_170\00", align 1
@_ZL5g_171 = internal global [8 x i8] c"\FF\FF\FF\FF\FF\FF\FF\FF", align 1
@.str24 = private unnamed_addr constant [9 x i8] c"g_171[i]\00", align 1
@.str25 = private unnamed_addr constant [14 x i8] c"index = [%d]\0A\00", align 1
@_ZL5g_172 = internal global i16 0, align 2
@.str26 = private unnamed_addr constant [6 x i8] c"g_172\00", align 1
@_ZL5g_257 = internal global i32 2067456055, align 4
@.str27 = private unnamed_addr constant [6 x i8] c"g_257\00", align 1
@_ZL5g_304 = internal global i8 1, align 1
@.str28 = private unnamed_addr constant [6 x i8] c"g_304\00", align 1
@_ZL5g_308 = internal global i16 -3, align 2
@.str29 = private unnamed_addr constant [6 x i8] c"g_308\00", align 1
@_ZL5g_361 = internal global i16 1, align 2
@.str30 = private unnamed_addr constant [6 x i8] c"g_361\00", align 1
@_ZL5g_400 = internal global i32 711639049, align 4
@.str31 = private unnamed_addr constant [6 x i8] c"g_400\00", align 1
@.str32 = private unnamed_addr constant [6 x i8] c"g_428\00", align 1
@_ZL5g_471 = internal global %struct.S0 { i16 0, i8 6, i32 1033957816 }, align 8
@.str33 = private unnamed_addr constant [9 x i8] c"g_471.f0\00", align 1
@.str34 = private unnamed_addr constant [9 x i8] c"g_471.f1\00", align 1
@.str35 = private unnamed_addr constant [9 x i8] c"g_471.f2\00", align 1
@_ZL5g_546 = internal global i32 1, align 4
@.str36 = private unnamed_addr constant [6 x i8] c"g_546\00", align 1
@.str37 = private unnamed_addr constant [6 x i8] c"g_687\00", align 1
@.str38 = private unnamed_addr constant [6 x i8] c"g_711\00", align 1
@_ZL5g_935 = internal unnamed_addr constant [10 x i32] [i32 -533036994, i32 -533036994, i32 1950933783, i32 -533036994, i32 -533036994, i32 1950933783, i32 -533036994, i32 -533036994, i32 1950933783, i32 -533036994], align 4
@.str39 = private unnamed_addr constant [9 x i8] c"g_935[i]\00", align 1
@_ZL6g_1010 = internal global i8 1, align 1
@.str40 = private unnamed_addr constant [7 x i8] c"g_1010\00", align 1
@_ZL6g_1025 = internal unnamed_addr global i8 -7, align 1
@.str41 = private unnamed_addr constant [7 x i8] c"g_1025\00", align 1
@_ZL6g_1136 = internal unnamed_addr global i8 -108, align 1
@.str42 = private unnamed_addr constant [7 x i8] c"g_1136\00", align 1
@.str43 = private unnamed_addr constant [7 x i8] c"g_1165\00", align 1
@.str44 = private unnamed_addr constant [7 x i8] c"g_1176\00", align 1
@_ZL6g_1198 = internal global i32 1, align 4
@.str45 = private unnamed_addr constant [7 x i8] c"g_1198\00", align 1
@_ZL6g_1442 = internal unnamed_addr constant [8 x i32] [i32 -3, i32 -1424736723, i32 -1424736723, i32 -3, i32 -1424736723, i32 -1424736723, i32 -3, i32 -1424736723], align 4
@.str46 = private unnamed_addr constant [10 x i8] c"g_1442[i]\00", align 1
@_ZL13crc32_context = internal unnamed_addr global i32 -1, align 4
@.str47 = private unnamed_addr constant [15 x i8] c"checksum = %X\0A\00", align 1
@_ZL4g_74 = internal global { i8, [3 x i8] } { i8 -4, [3 x i8] undef }, align 4
@.str48 = private unnamed_addr constant [36 x i8] c"...checksum after hashing %s : %lX\0A\00", align 1
@_ZL9crc32_tab = internal unnamed_addr global [256 x i32] zeroinitializer, align 4
@_ZZL6func_1vE6l_1011 = private unnamed_addr constant [7 x [1 x [7 x %struct.S0]]] [[1 x [7 x %struct.S0]] [[7 x %struct.S0] [%struct.S0 { i16 7185, i8 -3, i32 -2 }, %struct.S0 { i16 -25965, i8 -51, i32 7 }, %struct.S0 { i16 7185, i8 -3, i32 -2 }, %struct.S0 { i16 -4, i8 -5, i32 879602046 }, %struct.S0 { i16 32067, i8 -3, i32 -1554043524 }, %struct.S0 { i16 32067, i8 -3, i32 -1554043524 }, %struct.S0 { i16 -4, i8 -5, i32 879602046 }]], [1 x [7 x %struct.S0]] [[7 x %struct.S0] [%struct.S0 { i16 9338, i8 45, i32 -5 }, %struct.S0 { i16 -9, i8 50, i32 -865011421 }, %struct.S0 { i16 9338, i8 45, i32 -5 }, %struct.S0 { i16 10188, i8 44, i32 -40550910 }, %struct.S0 { i16 -15215, i8 -5, i32 -963383367 }, %struct.S0 { i16 -15215, i8 -5, i32 -963383367 }, %struct.S0 { i16 10188, i8 44, i32 -40550910 }]], [1 x [7 x %struct.S0]] [[7 x %struct.S0] [%struct.S0 { i16 7185, i8 -3, i32 -2 }, %struct.S0 { i16 -25965, i8 -51, i32 7 }, %struct.S0 { i16 7185, i8 -3, i32 -2 }, %struct.S0 { i16 -4, i8 -5, i32 879602046 }, %struct.S0 { i16 32067, i8 -3, i32 -1554043524 }, %struct.S0 { i16 32067, i8 -3, i32 -1554043524 }, %struct.S0 { i16 -4, i8 -5, i32 879602046 }]], [1 x [7 x %struct.S0]] [[7 x %struct.S0] [%struct.S0 { i16 9338, i8 45, i32 -5 }, %struct.S0 { i16 -9, i8 50, i32 -865011421 }, %struct.S0 { i16 9338, i8 45, i32 -5 }, %struct.S0 { i16 10188, i8 44, i32 -40550910 }, %struct.S0 { i16 -15215, i8 -5, i32 -963383367 }, %struct.S0 { i16 -15215, i8 -5, i32 -963383367 }, %struct.S0 { i16 10188, i8 44, i32 -40550910 }]], [1 x [7 x %struct.S0]] [[7 x %struct.S0] [%struct.S0 { i16 7185, i8 -3, i32 -2 }, %struct.S0 { i16 -25965, i8 -51, i32 7 }, %struct.S0 { i16 7185, i8 -3, i32 -2 }, %struct.S0 { i16 -4, i8 -5, i32 879602046 }, %struct.S0 { i16 32067, i8 -3, i32 -1554043524 }, %struct.S0 { i16 32067, i8 -3, i32 -1554043524 }, %struct.S0 { i16 -4, i8 -5, i32 879602046 }]], [1 x [7 x %struct.S0]] [[7 x %struct.S0] [%struct.S0 { i16 9338, i8 45, i32 -5 }, %struct.S0 { i16 -9, i8 50, i32 -865011421 }, %struct.S0 { i16 9338, i8 45, i32 -5 }, %struct.S0 { i16 10188, i8 44, i32 -40550910 }, %struct.S0 { i16 -15215, i8 -5, i32 -963383367 }, %struct.S0 { i16 -15215, i8 -5, i32 -963383367 }, %struct.S0 { i16 10188, i8 44, i32 -40550910 }]], [1 x [7 x %struct.S0]] [[7 x %struct.S0] [%struct.S0 { i16 7185, i8 -3, i32 -2 }, %struct.S0 { i16 -25965, i8 -51, i32 7 }, %struct.S0 { i16 7185, i8 -3, i32 -2 }, %struct.S0 { i16 -4, i8 -5, i32 879602046 }, %struct.S0 { i16 32067, i8 -3, i32 -1554043524 }, %struct.S0 { i16 -25965, i8 -51, i32 7 }, %struct.S0 { i16 32067, i8 -3, i32 -1554043524 }]]], align 8
@_ZL5g_984 = internal global i8** @_ZL5g_985, align 4
@_ZZL6func_1vE6l_1499 = private unnamed_addr constant [9 x i8] c"\98\98\98\98\98\98\98\98\98", align 1
@_ZZL6func_1vE6l_1006 = private unnamed_addr constant [10 x [7 x [3 x i32]]] [[7 x [3 x i32]] [[3 x i32] [i32 1841884517, i32 -1, i32 -1613798643], [3 x i32] [i32 2091444926, i32 -2138639918, i32 0], [3 x i32] [i32 283017736, i32 -7, i32 1698185546], [3 x i32] [i32 1628157269, i32 0, i32 1044216359], [3 x i32] [i32 1547065395, i32 -1, i32 -7], [3 x i32] [i32 8, i32 -1, i32 1], [3 x i32] [i32 1841884517, i32 0, i32 2]], [7 x [3 x i32]] [[3 x i32] [i32 7, i32 -7, i32 2087377514], [3 x i32] [i32 834421965, i32 -2138639918, i32 2], [3 x i32] [i32 1, i32 -1, i32 1044216359], [3 x i32] [i32 7, i32 -10, i32 -1], [3 x i32] [i32 7, i32 -1, i32 -328727838], [3 x i32] [i32 360433363, i32 -2138639918, i32 -1613798643], [3 x i32] [i32 1547065395, i32 -7, i32 -1]], [7 x [3 x i32]] [[3 x i32] [i32 0, i32 0, i32 -1129666350], [3 x i32] [i32 -1120699426, i32 -1, i32 -8], [3 x i32] [i32 2091444926, i32 -1, i32 2087377514], [3 x i32] [i32 7, i32 0, i32 -5], [3 x i32] [i32 679230240, i32 -7, i32 -5], [3 x i32] [i32 1, i32 -2138639918, i32 -7], [3 x i32] [i32 834421965, i32 -1, i32 -1129666350]], [7 x [3 x i32]] [[3 x i32] [i32 679230240, i32 -10, i32 -4], [3 x i32] [i32 -1, i32 -1, i32 0], [3 x i32] [i32 8, i32 -2138639918, i32 -328727838], [3 x i32] [i32 -1120699426, i32 -7, i32 -4], [3 x i32] [i32 1, i32 0, i32 -1], [3 x i32] [i32 283017736, i32 -1, i32 2], [3 x i32] [i32 360433363, i32 -1, i32 -5]], [7 x [3 x i32]] [[3 x i32] [i32 -1, i32 0, i32 0], [3 x i32] [i32 555482698, i32 -7, i32 1], [3 x i32] [i32 1, i32 -2138639918, i32 -8], [3 x i32] [i32 1, i32 -1, i32 -1], [3 x i32] [i32 555482698, i32 -10, i32 1698185546], [3 x i32] [i32 1841884517, i32 -1, i32 -1613798643], [3 x i32] [i32 2091444926, i32 -2138639918, i32 0]], [7 x [3 x i32]] [[3 x i32] [i32 283017736, i32 -7, i32 1698185546], [3 x i32] [i32 1628157269, i32 0, i32 1044216359], [3 x i32] [i32 1547065395, i32 -1, i32 -7], [3 x i32] [i32 8, i32 -1, i32 1], [3 x i32] [i32 1841884517, i32 0, i32 2], [3 x i32] [i32 7, i32 -7, i32 2087377514], [3 x i32] [i32 834421965, i32 -2138639918, i32 2]], [7 x [3 x i32]] [[3 x i32] [i32 1, i32 1023954582, i32 -1], [3 x i32] [i32 -1, i32 -1579985206, i32 8], [3 x i32] [i32 2, i32 1023954582, i32 -1425719182], [3 x i32] [i32 87909934, i32 0, i32 0], [3 x i32] [i32 0, i32 7, i32 8], [3 x i32] [i32 702414371, i32 0, i32 2], [3 x i32] [i32 -328727838, i32 -2, i32 -2]], [7 x [3 x i32]] [[3 x i32] [i32 -10, i32 -2, i32 96667756], [3 x i32] [i32 2, i32 0, i32 1924945160], [3 x i32] [i32 -1, i32 7, i32 -1402097775], [3 x i32] [i32 -1, i32 0, i32 7], [3 x i32] [i32 -4, i32 1023954582, i32 2], [3 x i32] [i32 -1, i32 -1579985206, i32 73780325], [3 x i32] [i32 0, i32 1023954582, i32 -1893771394]], [7 x [3 x i32]] [[3 x i32] [i32 4, i32 0, i32 -1425719182], [3 x i32] [i32 -328727838, i32 7, i32 73780325], [3 x i32] [i32 6, i32 0, i32 -2], [3 x i32] [i32 -1613798643, i32 -2, i32 -1986717165], [3 x i32] [i32 87909934, i32 -2, i32 -1402097775], [3 x i32] zeroinitializer, [3 x i32] [i32 320804302, i32 7, i32 -2139698281]], [7 x [3 x i32]] [[3 x i32] [i32 1698185546, i32 0, i32 -2], [3 x i32] [i32 -1, i32 1023954582, i32 -2], [3 x i32] [i32 320804302, i32 -1579985206, i32 -1878118873], [3 x i32] [i32 -5, i32 1023954582, i32 0], [3 x i32] [i32 -10, i32 0, i32 -1893771394], [3 x i32] [i32 -1613798643, i32 7, i32 -1878118873], [3 x i32] [i32 653753375, i32 0, i32 -1]]], align 4
@_ZL5g_983 = internal global i8*** @_ZL5g_984, align 4
@_ZZL6func_1vE6l_1324 = private unnamed_addr constant [7 x [3 x [10 x i8****]]] [[3 x [10 x i8****]] [[10 x i8****] [i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983], [10 x i8****] [i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983], [10 x i8****] [i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983]], [3 x [10 x i8****]] [[10 x i8****] [i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983], [10 x i8****] [i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983], [10 x i8****] [i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983]], [3 x [10 x i8****]] [[10 x i8****] [i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983], [10 x i8****] [i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983], [10 x i8****] [i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983]], [3 x [10 x i8****]] [[10 x i8****] [i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983], [10 x i8****] [i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983], [10 x i8****] [i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983]], [3 x [10 x i8****]] [[10 x i8****] [i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983], [10 x i8****] [i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983], [10 x i8****] [i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** null, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** null, i8**** @_ZL5g_983, i8**** @_ZL5g_983]], [3 x [10 x i8****]] [[10 x i8****] [i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983], [10 x i8****] [i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983], [10 x i8****] [i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983]], [3 x [10 x i8****]] [[10 x i8****] [i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983], [10 x i8****] [i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** null, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** null, i8**** @_ZL5g_983, i8**** @_ZL5g_983], [10 x i8****] [i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983, i8**** @_ZL5g_983]]], align 4
@_ZZL6func_1vE6l_1346 = private unnamed_addr constant [10 x i32] [i32 -2, i32 -2, i32 -2, i32 -2, i32 -2, i32 -2, i32 -2, i32 -2, i32 -2, i32 -2], align 4
@_ZZL6func_1vE6l_1519 = private unnamed_addr constant [10 x %struct.S0] [%struct.S0 { i16 1, i8 91, i32 1 }, %struct.S0 { i16 1, i8 91, i32 1 }, %struct.S0 { i16 1, i8 91, i32 1 }, %struct.S0 { i16 1, i8 91, i32 1 }, %struct.S0 { i16 1, i8 91, i32 1 }, %struct.S0 { i16 1, i8 91, i32 1 }, %struct.S0 { i16 1, i8 91, i32 1 }, %struct.S0 { i16 1, i8 91, i32 1 }, %struct.S0 { i16 1, i8 91, i32 1 }, %struct.S0 { i16 1, i8 91, i32 1 }], align 8
@_ZL5g_189 = internal constant %struct.S0* @_ZL4g_83, align 4
@_ZZL6func_1vE6l_1263 = private unnamed_addr constant [2 x [1 x [8 x i32]]] [[1 x [8 x i32]] [[8 x i32] [i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1]], [1 x [8 x i32]] [[8 x i32] [i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1]]], align 4
@_ZZL6func_1vE6l_1364 = private unnamed_addr constant %struct.S0 { i16 12857, i8 -1, i32 103472530 }, align 4
@_ZZL6func_1vE6l_1204 = private unnamed_addr constant [10 x i32*] [i32* @_ZL5g_138, i32* bitcast (i8* getelementptr (i8, i8* bitcast (%struct.S0* @_ZL5g_471 to i8*), i64 4) to i32*), i32* bitcast (i8* getelementptr (i8, i8* bitcast (%struct.S0* @_ZL5g_471 to i8*), i64 4) to i32*), i32* @_ZL5g_138, i32* bitcast (i8* getelementptr (i8, i8* bitcast (%struct.S0* @_ZL5g_471 to i8*), i64 4) to i32*), i32* bitcast (i8* getelementptr (i8, i8* bitcast (%struct.S0* @_ZL5g_471 to i8*), i64 4) to i32*), i32* @_ZL5g_138, i32* bitcast (i8* getelementptr (i8, i8* bitcast (%struct.S0* @_ZL5g_471 to i8*), i64 4) to i32*), i32* bitcast (i8* getelementptr (i8, i8* bitcast (%struct.S0* @_ZL5g_471 to i8*), i64 4) to i32*), i32* @_ZL5g_138], align 4
@_ZZL6func_1vE6l_1278 = private unnamed_addr constant [7 x [9 x [4 x i32]]] [[9 x [4 x i32]] [[4 x i32] [i32 0, i32 -9, i32 -608911817, i32 -1137618830], [4 x i32] [i32 -1673477626, i32 1178455578, i32 -1, i32 1], [4 x i32] [i32 1749634172, i32 -608911817, i32 -1750306944, i32 5], [4 x i32] [i32 -1, i32 -267173613, i32 1, i32 -267173613], [4 x i32] [i32 342924466, i32 -1137618830, i32 5, i32 0], [4 x i32] [i32 -1, i32 5, i32 -2032672910, i32 2], [4 x i32] [i32 -267173613, i32 1687884857, i32 -7, i32 0], [4 x i32] [i32 -267173613, i32 60247711, i32 -2032672910, i32 -829052586], [4 x i32] [i32 -1, i32 0, i32 5, i32 -9]], [9 x [4 x i32]] [[4 x i32] [i32 342924466, i32 6, i32 1, i32 -1750306944], [4 x i32] [i32 -1, i32 -2032672910, i32 -1750306944, i32 1], [4 x i32] [i32 1749634172, i32 -1, i32 -1, i32 1749634172], [4 x i32] [i32 -1673477626, i32 -920516504, i32 -608911817, i32 -5], [4 x i32] [i32 0, i32 -1, i32 1687884857, i32 0], [4 x i32] [i32 -829052586, i32 -1750306944, i32 1749634172, i32 0], [4 x i32] [i32 -1137618830, i32 -1, i32 342924466, i32 -5], [4 x i32] [i32 6, i32 -920516504, i32 -829052586, i32 1749634172], [4 x i32] [i32 1178455578, i32 -1, i32 1, i32 1]], [9 x [4 x i32]] [[4 x i32] [i32 0, i32 -2032672910, i32 -920516504, i32 -1750306944], [4 x i32] [i32 -852752877, i32 6, i32 -852752877, i32 -9], [4 x i32] [i32 1, i32 0, i32 6, i32 -829052586], [4 x i32] [i32 2, i32 60247711, i32 1, i32 0], [4 x i32] [i32 -608911817, i32 6, i32 -7, i32 -5], [4 x i32] [i32 -5, i32 0, i32 1749634172, i32 -1750306944], [4 x i32] [i32 -2032672910, i32 -1, i32 2, i32 -608911817], [4 x i32] [i32 2, i32 -608911817, i32 -1673477626, i32 0], [4 x i32] [i32 -267173613, i32 1, i32 -920516504, i32 -920516504]], [9 x [4 x i32]] [[4 x i32] [i32 -1692884230, i32 -1692884230, i32 1178455578, i32 -1], [4 x i32] [i32 1749634172, i32 1687884857, i32 0, i32 -1673477626], [4 x i32] [i32 -1, i32 -1750306944, i32 -1, i32 0], [4 x i32] [i32 1178455578, i32 -1750306944, i32 6, i32 -1673477626], [4 x i32] [i32 -1750306944, i32 1687884857, i32 1, i32 -1], [4 x i32] [i32 5, i32 -1692884230, i32 -1, i32 -920516504], [4 x i32] [i32 -1, i32 1, i32 1, i32 0], [4 x i32] [i32 -1, i32 -608911817, i32 -2032672910, i32 -608911817], [4 x i32] [i32 0, i32 -1, i32 0, i32 -1750306944]], [9 x [4 x i32]] [[4 x i32] [i32 -829052586, i32 0, i32 0, i32 -5], [4 x i32] [i32 -608911817, i32 6, i32 -1137618830, i32 -267173613], [4 x i32] [i32 -608911817, i32 -9, i32 0, i32 1178455578], [4 x i32] [i32 -829052586, i32 -267173613, i32 0, i32 1687884857], [4 x i32] [i32 0, i32 1749634172, i32 -2032672910, i32 1], [4 x i32] [i32 -1, i32 0, i32 1, i32 -7], [4 x i32] [i32 -1, i32 -1, i32 -1, i32 -1], [4 x i32] [i32 5, i32 -1673477626, i32 1, i32 0], [4 x i32] [i32 -1750306944, i32 1, i32 6, i32 342924466]], [9 x [4 x i32]] [[4 x i32] [i32 1178455578, i32 1, i32 -1, i32 342924466], [4 x i32] [i32 -1, i32 1, i32 0, i32 0], [4 x i32] [i32 1749634172, i32 -1673477626, i32 1178455578, i32 -1], [4 x i32] [i32 -1692884230, i32 -1, i32 -920516504, i32 -7], [4 x i32] [i32 -267173613, i32 0, i32 -1673477626, i32 1], [4 x i32] [i32 2, i32 1749634172, i32 2, i32 1687884857], [4 x i32] [i32 -2032672910, i32 -267173613, i32 1749634172, i32 1178455578], [4 x i32] [i32 -5, i32 -9, i32 -7, i32 -267173613], [4 x i32] [i32 1, i32 6, i32 -7, i32 -5]], [9 x [4 x i32]] [[4 x i32] [i32 -5, i32 0, i32 1749634172, i32 -1750306944], [4 x i32] [i32 -2032672910, i32 -1, i32 2, i32 -608911817], [4 x i32] [i32 2, i32 -608911817, i32 -1673477626, i32 0], [4 x i32] [i32 -267173613, i32 1, i32 -920516504, i32 -920516504], [4 x i32] [i32 -1692884230, i32 -1692884230, i32 1178455578, i32 -1], [4 x i32] [i32 1749634172, i32 1687884857, i32 0, i32 -1673477626], [4 x i32] [i32 -1, i32 -1750306944, i32 -1, i32 0], [4 x i32] [i32 1178455578, i32 -1750306944, i32 6, i32 -1673477626], [4 x i32] [i32 -1750306944, i32 1687884857, i32 1, i32 -1]]], align 4
@_ZZL6func_1vE6l_1365 = private unnamed_addr constant [8 x [3 x [5 x i16]]] [[3 x [5 x i16]] [[5 x i16] [i16 -1, i16 -10870, i16 20658, i16 0, i16 3], [5 x i16] [i16 22073, i16 24925, i16 -13292, i16 -10870, i16 -1459], [5 x i16] [i16 1, i16 138, i16 7, i16 -26288, i16 7]], [3 x [5 x i16]] [[5 x i16] [i16 1, i16 1, i16 7, i16 5, i16 24925], [5 x i16] [i16 -30784, i16 0, i16 -13292, i16 -1, i16 -26288], [5 x i16] [i16 1, i16 9699, i16 20658, i16 5708, i16 1]], [3 x [5 x i16]] [[5 x i16] [i16 22073, i16 1, i16 20658, i16 -13292, i16 7], [5 x i16] [i16 0, i16 -1459, i16 5708, i16 -1459, i16 0], [5 x i16] [i16 0, i16 22073, i16 2965, i16 1, i16 -1459]], [3 x [5 x i16]] [[5 x i16] [i16 22073, i16 5, i16 -30784, i16 2965, i16 5708], [5 x i16] [i16 2965, i16 -30784, i16 5, i16 22073, i16 -1459], [5 x i16] [i16 1, i16 2965, i16 22073, i16 0, i16 0]], [3 x [5 x i16]] [[5 x i16] [i16 -1459, i16 5708, i16 -1459, i16 0, i16 7], [5 x i16] [i16 -13292, i16 20658, i16 1, i16 22073, i16 1], [5 x i16] [i16 5708, i16 7, i16 -1, i16 2965, i16 1]], [3 x [5 x i16]] [[5 x i16] [i16 31974, i16 -10870, i16 1, i16 1, i16 5], [5 x i16] [i16 1, i16 9699, i16 -1459, i16 -1459, i16 9699], [5 x i16] [i16 1, i16 9699, i16 22073, i16 -13292, i16 3]], [3 x [5 x i16]] [[5 x i16] [i16 -30784, i16 -10870, i16 5, i16 5708, i16 138], [5 x i16] [i16 1, i16 7, i16 -30784, i16 31974, i16 -26288], [5 x i16] [i16 -30784, i16 20658, i16 2965, i16 1, i16 22073]], [3 x [5 x i16]] [[5 x i16] [i16 1, i16 5708, i16 5708, i16 1, i16 22073], [5 x i16] [i16 1, i16 2965, i16 20658, i16 -30784, i16 -26288], [5 x i16] [i16 31974, i16 -30784, i16 7, i16 1, i16 138]]], align 2
@_ZL5g_985 = internal global i8* @_ZL5g_304, align 4
@_ZZL6func_1vE6l_1320 = private unnamed_addr constant <{ <{ { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] } }>, <{ { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] } }>, <{ { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] } }>, <{ { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] } }>, <{ { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] } }>, <{ { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] } }>, <{ { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] } }> }> <{ <{ { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] } }> <{ { i8, [3 x i8] } { i8 -38, [3 x i8] undef }, { i8, [3 x i8] } { i8 -5, [3 x i8] undef }, { i8, [3 x i8] } { i8 -71, [3 x i8] undef }, { i8, [3 x i8] } { i8 19, [3 x i8] undef }, { i8, [3 x i8] } { i8 126, [3 x i8] undef }, { i8, [3 x i8] } { i8 -87, [3 x i8] undef }, { i8, [3 x i8] } { i8 1, [3 x i8] undef }, { i8, [3 x i8] } { i8 -4, [3 x i8] undef }, { i8, [3 x i8] } { i8 8, [3 x i8] undef } }>, <{ { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] } }> <{ { i8, [3 x i8] } { i8 -4, [3 x i8] undef }, { i8, [3 x i8] } { i8 -4, [3 x i8] undef }, { i8, [3 x i8] } { i8 -5, [3 x i8] undef }, { i8, [3 x i8] } { i8 8, [3 x i8] undef }, { i8, [3 x i8] } { i8 8, [3 x i8] undef }, { i8, [3 x i8] } { i8 -5, [3 x i8] undef }, { i8, [3 x i8] } { i8 -4, [3 x i8] undef }, { i8, [3 x i8] } { i8 -4, [3 x i8] undef }, { i8, [3 x i8] } { i8 -87, [3 x i8] undef } }>, <{ { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] } }> <{ { i8, [3 x i8] } { i8 3, [3 x i8] undef }, { i8, [3 x i8] } { i8 43, [3 x i8] undef }, { i8, [3 x i8] } { i8 -125, [3 x i8] undef }, { i8, [3 x i8] } { i8 1, [3 x i8] undef }, { i8, [3 x i8] } { i8 1, [3 x i8] undef }, { i8, [3 x i8] } { i8 19, [3 x i8] undef }, { i8, [3 x i8] } { i8 -87, [3 x i8] undef }, { i8, [3 x i8] } { i8 -4, [3 x i8] undef }, { i8, [3 x i8] } { i8 -1, [3 x i8] undef } }>, <{ { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] } }> <{ { i8, [3 x i8] } { i8 3, [3 x i8] undef }, { i8, [3 x i8] } { i8 1, [3 x i8] undef }, { i8, [3 x i8] } { i8 -21, [3 x i8] undef }, { i8, [3 x i8] } { i8 118, [3 x i8] undef }, { i8, [3 x i8] } { i8 1, [3 x i8] undef }, { i8, [3 x i8] } { i8 -4, [3 x i8] undef }, { i8, [3 x i8] } { i8 0, [3 x i8] undef }, { i8, [3 x i8] } { i8 -1, [3 x i8] undef }, { i8, [3 x i8] } { i8 -87, [3 x i8] undef } }>, <{ { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] } }> <{ { i8, [3 x i8] } { i8 -1, [3 x i8] undef }, { i8, [3 x i8] } { i8 8, [3 x i8] undef }, { i8, [3 x i8] } { i8 1, [3 x i8] undef }, { i8, [3 x i8] } { i8 3, [3 x i8] undef }, { i8, [3 x i8] } { i8 0, [3 x i8] undef }, { i8, [3 x i8] } { i8 0, [3 x i8] undef }, { i8, [3 x i8] } { i8 3, [3 x i8] undef }, { i8, [3 x i8] } { i8 1, [3 x i8] undef }, { i8, [3 x i8] } { i8 8, [3 x i8] undef } }>, <{ { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] } }> <{ { i8, [3 x i8] } { i8 -1, [3 x i8] undef }, { i8, [3 x i8] } { i8 -87, [3 x i8] undef }, { i8, [3 x i8] } { i8 -1, [3 x i8] undef }, { i8, [3 x i8] } { i8 0, [3 x i8] undef }, { i8, [3 x i8] } { i8 -4, [3 x i8] undef }, { i8, [3 x i8] } { i8 1, [3 x i8] undef }, { i8, [3 x i8] } { i8 118, [3 x i8] undef }, { i8, [3 x i8] } { i8 -21, [3 x i8] undef }, { i8, [3 x i8] } { i8 1, [3 x i8] undef } }>, <{ { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] } }> <{ { i8, [3 x i8] } { i8 3, [3 x i8] undef }, { i8, [3 x i8] } { i8 -1, [3 x i8] undef }, { i8, [3 x i8] } { i8 -4, [3 x i8] undef }, { i8, [3 x i8] } { i8 -87, [3 x i8] undef }, { i8, [3 x i8] } { i8 19, [3 x i8] undef }, { i8, [3 x i8] } { i8 1, [3 x i8] undef }, { i8, [3 x i8] } { i8 1, [3 x i8] undef }, { i8, [3 x i8] } { i8 -125, [3 x i8] undef }, { i8, [3 x i8] } { i8 43, [3 x i8] undef } }> }>, align 4
@_ZZL6func_1vE6l_1423 = private unnamed_addr constant %struct.S0 { i16 -14979, i8 -56, i32 1 }, align 4
@_ZZL6func_1vE6l_1460 = private unnamed_addr constant [5 x [10 x i32]] [[10 x i32] [i32 -1, i32 -1, i32 -2111769585, i32 -10, i32 3, i32 -1, i32 -7, i32 -1, i32 -1, i32 -7], [10 x i32] [i32 -88107511, i32 -1, i32 -10, i32 -10, i32 -1, i32 -88107511, i32 -392857583, i32 -1, i32 1, i32 9], [10 x i32] [i32 9, i32 -1, i32 -1, i32 1, i32 -2111769585, i32 1, i32 -88107511, i32 1, i32 -2111769585, i32 1], [10 x i32] [i32 9, i32 1, i32 9, i32 -2, i32 -7, i32 -88107511, i32 -88107511, i32 -10, i32 1, i32 -2], [10 x i32] [i32 -1, i32 -88107511, i32 -10, i32 1, i32 -2, i32 -2, i32 1, i32 -10, i32 -88107511, i32 -1]], align 4
@_ZL5g_197 = internal global [2 x [10 x i32*]] [[10 x i32*] [i32* @_ZL4g_93, i32* @_ZL4g_93, i32* @_ZL4g_93, i32* @_ZL4g_93, i32* @_ZL4g_93, i32* @_ZL4g_93, i32* @_ZL4g_93, i32* @_ZL4g_93, i32* @_ZL4g_93, i32* @_ZL4g_93], [10 x i32*] [i32* @_ZL4g_93, i32* @_ZL4g_93, i32* @_ZL4g_93, i32* @_ZL4g_93, i32* @_ZL4g_93, i32* @_ZL4g_93, i32* @_ZL4g_93, i32* @_ZL4g_93, i32* @_ZL4g_93, i32* @_ZL4g_93]], align 4
@_ZZL6func_1vE6l_1443 = private unnamed_addr constant [7 x [5 x i32]] [[5 x i32] [i32 520842546, i32 -856207067, i32 2, i32 -3, i32 -1686173722], [5 x i32] [i32 1184089122, i32 -1017155828, i32 -3, i32 -691016572, i32 -3], [5 x i32] [i32 -3, i32 -3, i32 0, i32 2, i32 8], [5 x i32] [i32 -3, i32 -10, i32 8, i32 9, i32 7], [5 x i32] [i32 1184089122, i32 -1176025092, i32 -1686173722, i32 -8, i32 -1], [5 x i32] [i32 520842546, i32 -10, i32 -10, i32 520842546, i32 1752169706], [5 x i32] [i32 -856207067, i32 -3, i32 -10, i32 8, i32 9]], align 4
@_ZZL6func_1vE6l_1461 = private unnamed_addr constant [10 x [1 x i32]] [[1 x i32] [i32 1], [1 x i32] [i32 -1], [1 x i32] [i32 1], [1 x i32] [i32 -1], [1 x i32] [i32 1], [1 x i32] [i32 -1], [1 x i32] [i32 1], [1 x i32] [i32 -1], [1 x i32] [i32 1], [1 x i32] [i32 -1]], align 4
@_ZZL6func_1vE6l_1470 = private unnamed_addr constant %struct.S0 { i16 -30104, i8 -1, i32 6 }, align 4
@_ZL6g_1353 = internal constant %struct.S0*** @_ZL6g_1354, align 4
@_ZL6g_1354 = internal global %struct.S0** @_ZL6g_1355, align 4
@_ZL6g_1355 = internal global %struct.S0* null, align 4
@_ZL6g_1393 = internal global i32** @_ZL6g_1394, align 4
@_ZL6g_1394 = internal global i32* @_ZL5g_546, align 4
@_ZL5g_931 = internal global i32* @_ZL4g_53, align 4
@_ZL6g_1162 = internal unnamed_addr global { i8, [3 x i8] } { i8 0, [3 x i8] undef }, align 4
@_ZZL7func_20iitE4l_60 = private unnamed_addr constant %struct.S0 { i16 26262, i8 73, i32 1210384164 }, align 4
@_ZZL7func_20iitE5l_757 = private unnamed_addr constant [1 x [9 x [4 x i16]]] [[9 x [4 x i16]] [[4 x i16] [i16 -1, i16 0, i16 0, i16 -1], [4 x i16] [i16 0, i16 -1, i16 0, i16 8], [4 x i16] [i16 0, i16 0, i16 -1, i16 0], [4 x i16] [i16 0, i16 8, i16 8, i16 0], [4 x i16] [i16 8, i16 0, i16 8, i16 8], [4 x i16] [i16 0, i16 0, i16 -1, i16 0], [4 x i16] [i16 0, i16 8, i16 8, i16 0], [4 x i16] [i16 8, i16 0, i16 8, i16 8], [4 x i16] [i16 0, i16 0, i16 -1, i16 0]]], align 2
@_ZZL7func_20iitE5l_876 = private unnamed_addr constant [10 x i16] [i16 5, i16 0, i16 0, i16 5, i16 -9, i16 5, i16 0, i16 0, i16 5, i16 -9], align 2
@_ZZL7func_20iitE5l_403 = private unnamed_addr constant [1 x [4 x i32*]] [[4 x i32*] [i32* @_ZL5g_135, i32* @_ZL5g_135, i32* @_ZL5g_135, i32* @_ZL5g_135]], align 4
@_ZZL7func_20iitE5l_422 = private unnamed_addr constant %struct.S0 { i16 8, i8 95, i32 -1317782521 }, align 4
@_ZZL7func_20iitE5l_516 = private unnamed_addr constant <{ { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] } }> <{ { i8, [3 x i8] } { i8 118, [3 x i8] undef }, { i8, [3 x i8] } { i8 118, [3 x i8] undef }, { i8, [3 x i8] } { i8 118, [3 x i8] undef }, { i8, [3 x i8] } { i8 118, [3 x i8] undef }, { i8, [3 x i8] } { i8 118, [3 x i8] undef }, { i8, [3 x i8] } { i8 118, [3 x i8] undef }, { i8, [3 x i8] } { i8 118, [3 x i8] undef }, { i8, [3 x i8] } { i8 118, [3 x i8] undef }, { i8, [3 x i8] } { i8 118, [3 x i8] undef } }>, align 4
@_ZZL7func_20iitE5l_519 = private unnamed_addr constant %struct.S0 { i16 -1, i8 9, i32 -1103229061 }, align 4
@_ZZL7func_20iitE5l_548 = private unnamed_addr constant [8 x [8 x i32]] [[8 x i32] [i32 6, i32 -1608903715, i32 -2, i32 -1608903715, i32 6, i32 -2, i32 2, i32 2], [8 x i32] [i32 3, i32 -1608903715, i32 -1, i32 -1, i32 -1608903715, i32 3, i32 7, i32 -1608903715], [8 x i32] [i32 2, i32 7, i32 -1, i32 2, i32 -1, i32 7, i32 2, i32 3], [8 x i32] [i32 -1608903715, i32 6, i32 -2, i32 2, i32 2, i32 -2, i32 6, i32 -1608903715], [8 x i32] [i32 3, i32 2, i32 7, i32 -1, i32 2, i32 -1, i32 7, i32 2], [8 x i32] [i32 -1608903715, i32 7, i32 3, i32 -1608903715, i32 -1, i32 -1, i32 -1608903715, i32 3], [8 x i32] [i32 2, i32 2, i32 -2, i32 6, i32 -1608903715, i32 -2, i32 -1608903715, i32 6], [8 x i32] [i32 3, i32 6, i32 3, i32 -1, i32 6, i32 7, i32 7, i32 6]], align 4
@_ZZL7func_20iitE5l_495 = private unnamed_addr constant %struct.S0 { i16 -5, i8 -1, i32 -768235975 }, align 4
@_ZZL7func_20iitE5l_467 = private unnamed_addr constant [6 x i32] [i32 1631231813, i32 1631231813, i32 1631231813, i32 1631231813, i32 1631231813, i32 1631231813], align 4
@_ZZL7func_20iitE5l_513 = private unnamed_addr constant [2 x [3 x [9 x i16*]]] [[3 x [9 x i16*]] [[9 x i16*] [i16* @_ZL4g_79, i16* @_ZL4g_79, i16* @_ZL5g_103, i16* @_ZL4g_77, i16* @_ZL5g_361, i16* @_ZL5g_172, i16* @_ZL4g_77, i16* @_ZL5g_172, i16* @_ZL5g_361], [9 x i16*] [i16* @_ZL5g_361, i16* @_ZL5g_172, i16* @_ZL5g_172, i16* @_ZL5g_361, i16* @_ZL4g_79, i16* @_ZL5g_103, i16* null, i16* @_ZL4g_77, i16* null], [9 x i16*] [i16* @_ZL4g_77, i16* null, i16* @_ZL5g_103, i16* @_ZL5g_103, i16* null, i16* @_ZL4g_77, i16* @_ZL5g_308, i16* @_ZL4g_79, i16* null]], [3 x [9 x i16*]] [[9 x i16*] [i16* @_ZL5g_308, i16* @_ZL5g_103, i16* @_ZL4g_77, i16* @_ZL4g_79, i16* @_ZL4g_79, i16* @_ZL4g_77, i16* @_ZL5g_103, i16* @_ZL5g_308, i16* @_ZL5g_172], [9 x i16*] [i16* null, i16* @_ZL5g_103, i16* @_ZL4g_82, i16* @_ZL5g_361, i16* null, i16* null, i16* @_ZL5g_361, i16* @_ZL4g_82, i16* @_ZL4g_79], [9 x i16*] [i16* @_ZL5g_172, i16* @_ZL5g_308, i16* @_ZL4g_77, i16* @_ZL5g_361, i16* null, i16* @_ZL5g_308, i16* @_ZL5g_308, i16* null, i16* @_ZL5g_361]]], align 4
@_ZZL7func_20iitE5l_589 = private unnamed_addr constant <{ { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] }, { i8, [3 x i8] } }> <{ { i8, [3 x i8] } { i8 -1, [3 x i8] undef }, { i8, [3 x i8] } { i8 -19, [3 x i8] undef }, { i8, [3 x i8] } { i8 -1, [3 x i8] undef }, { i8, [3 x i8] } { i8 -1, [3 x i8] undef }, { i8, [3 x i8] } { i8 -19, [3 x i8] undef }, { i8, [3 x i8] } { i8 -1, [3 x i8] undef } }>, align 4
@_ZZL7func_20iitE5l_598 = private unnamed_addr constant [2 x [10 x %struct.S0]] [[10 x %struct.S0] [%struct.S0 { i16 23086, i8 -45, i32 -648996750 }, %struct.S0 { i16 -6, i8 -105, i32 -1 }, %struct.S0 { i16 23086, i8 -45, i32 -648996750 }, %struct.S0 { i16 23086, i8 -45, i32 -648996750 }, %struct.S0 { i16 -6, i8 -105, i32 -1 }, %struct.S0 { i16 23086, i8 -45, i32 -648996750 }, %struct.S0 { i16 23086, i8 -45, i32 -648996750 }, %struct.S0 { i16 -6, i8 -105, i32 -1 }, %struct.S0 { i16 23086, i8 -45, i32 -648996750 }, %struct.S0 { i16 23086, i8 -45, i32 -648996750 }], [10 x %struct.S0] [%struct.S0 { i16 -6, i8 -105, i32 -1 }, %struct.S0 { i16 -6, i8 -105, i32 -1 }, %struct.S0 { i16 -25976, i8 0, i32 1 }, %struct.S0 { i16 -6, i8 -105, i32 -1 }, %struct.S0 { i16 -6, i8 -105, i32 -1 }, %struct.S0 { i16 -25976, i8 0, i32 1 }, %struct.S0 { i16 -6, i8 -105, i32 -1 }, %struct.S0 { i16 -6, i8 -105, i32 -1 }, %struct.S0 { i16 -25976, i8 0, i32 1 }, %struct.S0 { i16 -6, i8 -105, i32 -1 }]], align 4
@_ZZL7func_492S0E5l_178 = private unnamed_addr constant [3 x [5 x [9 x i16*]]] [[5 x [9 x i16*]] [[9 x i16*] [i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* @_ZL5g_162, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* @_ZL5g_168, i16* @_ZL5g_168, i16* @_ZL5g_168, i16* @_ZL5g_168, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* @_ZL5g_162], [9 x i16*] [i16* null, i16* @_ZL5g_168, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* @_ZL5g_168, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* @_ZL5g_162, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* null], [9 x i16*] [i16* @_ZL5g_162, i16* @_ZL5g_168, i16* null, i16* null, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* null, i16* null, i16* @_ZL5g_168, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0)], [9 x i16*] [i16* @_ZL5g_162, i16* @_ZL5g_168, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* @_ZL5g_162, i16* @_ZL5g_168, i16* @_ZL5g_162, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* @_ZL5g_162, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0)], [9 x i16*] [i16* null, i16* @_ZL5g_162, i16* null, i16* @_ZL5g_168, i16* @_ZL5g_162, i16* @_ZL5g_168, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* null, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0)]], [5 x [9 x i16*]] [[9 x i16*] [i16* null, i16* @_ZL5g_162, i16* null, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* @_ZL5g_168, i16* @_ZL5g_162, i16* @_ZL5g_168, i16* @_ZL5g_168, i16* @_ZL5g_168], [9 x i16*] [i16* null, i16* @_ZL5g_168, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* @_ZL5g_162, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* @_ZL5g_162, i16* @_ZL5g_162, i16* @_ZL5g_162, i16* @_ZL5g_168], [9 x i16*] [i16* null, i16* null, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* @_ZL5g_168, i16* @_ZL5g_168, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* null, i16* null, i16* null], [9 x i16*] [i16* @_ZL5g_162, i16* @_ZL5g_168, i16* null, i16* null, i16* @_ZL5g_162, i16* null, i16* @_ZL5g_168, i16* @_ZL5g_162, i16* @_ZL5g_168], [9 x i16*] [i16* @_ZL5g_162, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* @_ZL5g_168, i16* null, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* @_ZL5g_168, i16* null]], [5 x [9 x i16*]] [[9 x i16*] [i16* null, i16* @_ZL5g_168, i16* null, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* @_ZL5g_162, i16* null, i16* @_ZL5g_162, i16* @_ZL5g_162, i16* @_ZL5g_168], [9 x i16*] [i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* @_ZL5g_168, i16* @_ZL5g_168, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* null, i16* @_ZL5g_162, i16* @_ZL5g_168, i16* @_ZL5g_168], [9 x i16*] [i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* null, i16* null, i16* @_ZL5g_168, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* null, i16* null, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* @_ZL5g_168], [9 x i16*] [i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* null, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* null, i16* @_ZL5g_168, i16* @_ZL5g_168, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), i16* @_ZL5g_168], [9 x i16*] [i16* @_ZL5g_162, i16* @_ZL5g_168, i16* @_ZL5g_168, i16* @_ZL5g_168, i16* @_ZL5g_168, i16* @_ZL5g_162, i16* @_ZL5g_168, i16* @_ZL5g_168, i16* null]]], align 4
@_ZZL7func_492S0E5l_198 = private unnamed_addr constant [4 x [1 x i32**]] [[1 x i32**] [i32** bitcast (i8* getelementptr (i8, i8* bitcast ([2 x [10 x i32*]]* @_ZL5g_197 to i8*), i64 76) to i32**)], [1 x i32**] zeroinitializer, [1 x i32**] [i32** bitcast (i8* getelementptr (i8, i8* bitcast ([2 x [10 x i32*]]* @_ZL5g_197 to i8*), i64 76) to i32**)], [1 x i32**] zeroinitializer], align 4
@_ZZL7func_492S0E5l_119 = private unnamed_addr constant { i8, [3 x i8] } { i8 -3, [3 x i8] undef }, align 4
@_ZZL7func_492S0E5l_125 = private unnamed_addr constant [8 x [4 x [6 x i32]]] [[4 x [6 x i32]] [[6 x i32] [i32 1, i32 -9, i32 2, i32 585837050, i32 1, i32 9], [6 x i32] [i32 -9, i32 0, i32 -5, i32 9, i32 1, i32 585837050], [6 x i32] [i32 735221361, i32 -9, i32 -626661945, i32 0, i32 -875020938, i32 -9], [6 x i32] [i32 -875020938, i32 -1969890654, i32 -5, i32 -1, i32 -1, i32 -5]], [4 x [6 x i32]] [[6 x i32] [i32 -875020938, i32 -875020938, i32 2, i32 0, i32 0, i32 1], [6 x i32] [i32 735221361, i32 -626661945, i32 -1969890654, i32 9, i32 -1, i32 2], [6 x i32] [i32 -9, i32 735221361, i32 -1969890654, i32 585837050, i32 -875020938, i32 1], [6 x i32] [i32 1, i32 585837050, i32 2, i32 -9, i32 1, i32 -5]], [4 x [6 x i32]] [[6 x i32] [i32 -9, i32 1, i32 -5, i32 -5, i32 1, i32 -9], [6 x i32] [i32 735221361, i32 585837050, i32 -626661945, i32 1, i32 -875020938, i32 585837050], [6 x i32] [i32 -875020938, i32 735221361, i32 -5, i32 2, i32 -1, i32 9], [6 x i32] [i32 -875020938, i32 -626661945, i32 2, i32 1, i32 0, i32 0]], [4 x [6 x i32]] [[6 x i32] [i32 735221361, i32 0, i32 0, i32 2, i32 -875020938, i32 -875020938], [6 x i32] [i32 735221361, i32 0, i32 0, i32 735221361, i32 0, i32 -5], [6 x i32] [i32 9, i32 735221361, i32 -626661945, i32 -1969890654, i32 9, i32 -1], [6 x i32] [i32 735221361, i32 -5, i32 2, i32 -1, i32 9, i32 -1969890654]], [4 x [6 x i32]] [[6 x i32] [i32 1, i32 735221361, i32 459562963, i32 -5, i32 0, i32 735221361], [6 x i32] [i32 0, i32 0, i32 2, i32 -875020938, i32 -875020938, i32 2], [6 x i32] [i32 0, i32 0, i32 -626661945, i32 -5, i32 -9, i32 9], [6 x i32] [i32 1, i32 459562963, i32 0, i32 -1, i32 -875020938, i32 -626661945]], [4 x [6 x i32]] [[6 x i32] [i32 735221361, i32 1, i32 0, i32 -1969890654, i32 0, i32 9], [6 x i32] [i32 9, i32 -1969890654, i32 -626661945, i32 735221361, i32 9, i32 2], [6 x i32] [i32 735221361, i32 9, i32 2, i32 2, i32 9, i32 735221361], [6 x i32] [i32 1, i32 -1969890654, i32 459562963, i32 9, i32 0, i32 -1969890654]], [4 x [6 x i32]] [[6 x i32] [i32 0, i32 1, i32 2, i32 -626661945, i32 -875020938, i32 -1], [6 x i32] [i32 0, i32 459562963, i32 -626661945, i32 9, i32 -9, i32 -5], [6 x i32] [i32 1, i32 0, i32 0, i32 2, i32 -875020938, i32 -875020938], [6 x i32] [i32 735221361, i32 0, i32 0, i32 735221361, i32 0, i32 -5]], [4 x [6 x i32]] [[6 x i32] [i32 9, i32 735221361, i32 -626661945, i32 -1969890654, i32 9, i32 -1], [6 x i32] [i32 735221361, i32 -5, i32 2, i32 -1, i32 9, i32 -1969890654], [6 x i32] [i32 1, i32 735221361, i32 459562963, i32 -5, i32 0, i32 735221361], [6 x i32] [i32 0, i32 0, i32 2, i32 -875020938, i32 -875020938, i32 2]]], align 4
@_ZZL7func_492S0E5l_296 = private unnamed_addr constant [6 x [2 x [4 x i32]]] [[2 x [4 x i32]] [[4 x i32] [i32 -2, i32 5, i32 161729879, i32 884294967], [4 x i32] [i32 -1, i32 0, i32 -3, i32 884294967]], [2 x [4 x i32]] [[4 x i32] [i32 -1769297005, i32 5, i32 1, i32 0], [4 x i32] [i32 1136616794, i32 -1, i32 -1591181149, i32 -1]], [2 x [4 x i32]] [[4 x i32] [i32 5, i32 62548300, i32 -3, i32 -6], [4 x i32] [i32 59636063, i32 -2, i32 62548300, i32 59636063]], [2 x [4 x i32]] [[4 x i32] [i32 -2, i32 -1, i32 1, i32 1136616794], [4 x i32] [i32 -2, i32 -1769297005, i32 62548300, i32 884294967]], [2 x [4 x i32]] [[4 x i32] [i32 59636063, i32 1136616794, i32 -3, i32 -3], [4 x i32] [i32 5, i32 5, i32 -1591181149, i32 1136616794]], [2 x [4 x i32]] [[4 x i32] [i32 1136616794, i32 59636063, i32 1, i32 -1], [4 x i32] [i32 -1769297005, i32 -2, i32 -3, i32 1]]], align 4
@_ZZL7func_40iaE5l_359 = private unnamed_addr constant [2 x [6 x [10 x i32*]]] [[6 x [10 x i32*]] [[10 x i32*] [i32* @_ZL3g_2, i32* @_ZL3g_2, i32* @_ZL5g_135, i32* @_ZL4g_93, i32* @_ZL3g_2, i32* null, i32* @_ZL3g_2, i32* @_ZL5g_132, i32* @_ZL5g_132, i32* @_ZL3g_2], [10 x i32*] [i32* @_ZL5g_257, i32* @_ZL4g_93, i32* @_ZL3g_2, i32* @_ZL3g_2, i32* @_ZL4g_93, i32* @_ZL5g_257, i32* @_ZL5g_132, i32* null, i32* @_ZL3g_2, i32* @_ZL5g_132], [10 x i32*] [i32* @_ZL5g_135, i32* null, i32* null, i32* @_ZL5g_135, i32* @_ZL4g_93, i32* @_ZL3g_2, i32* @_ZL3g_2, i32* @_ZL3g_2, i32* @_ZL3g_2, i32* @_ZL5g_257], [10 x i32*] [i32* @_ZL5g_135, i32* @_ZL3g_2, i32* null, i32* @_ZL5g_135, i32* @_ZL4g_93, i32* @_ZL5g_257, i32* null, i32* null, i32* @_ZL5g_132, i32* @_ZL3g_2], [10 x i32*] [i32* null, i32* null, i32* null, i32* @_ZL5g_132, i32* @_ZL3g_2, i32* @_ZL3g_2, i32* @_ZL4g_93, i32* @_ZL3g_2, i32* @_ZL5g_132, i32* @_ZL5g_257], [10 x i32*] [i32* @_ZL5g_257, i32* @_ZL5g_132, i32* @_ZL3g_2, i32* @_ZL5g_132, i32* @_ZL3g_2, i32* @_ZL3g_2, i32* @_ZL4g_93, i32* @_ZL3g_2, i32* @_ZL3g_2, i32* @_ZL3g_2]], [6 x [10 x i32*]] [[10 x i32*] [i32* @_ZL4g_93, i32* null, i32* null, i32* @_ZL5g_135, i32* @_ZL5g_132, i32* @_ZL5g_132, i32* @_ZL5g_135, i32* null, i32* null, i32* @_ZL4g_93], [10 x i32*] [i32* @_ZL5g_257, i32* @_ZL5g_257, i32* @_ZL3g_2, i32* @_ZL5g_132, i32* @_ZL3g_2, i32* @_ZL3g_2, i32* @_ZL4g_93, i32* @_ZL5g_257, i32* null, i32* @_ZL5g_257], [10 x i32*] [i32* @_ZL5g_135, i32* @_ZL3g_2, i32* @_ZL3g_2, i32* @_ZL5g_257, i32* @_ZL3g_2, i32* @_ZL3g_2, i32* @_ZL5g_132, i32* @_ZL4g_93, i32* @_ZL4g_93, i32* @_ZL4g_93], [10 x i32*] [i32* @_ZL3g_2, i32* @_ZL4g_93, i32* @_ZL5g_132, i32* @_ZL5g_132, i32* @_ZL5g_132, i32* @_ZL4g_93, i32* @_ZL3g_2, i32* @_ZL5g_257, i32* @_ZL5g_257, i32* @_ZL3g_2], [10 x i32*] [i32* null, i32* @_ZL3g_2, i32* @_ZL3g_2, i32* @_ZL5g_132, i32* @_ZL3g_2, i32* @_ZL5g_257, i32* null, i32* @_ZL5g_135, i32* @_ZL3g_2, i32* @_ZL5g_257], [10 x i32*] [i32* @_ZL5g_132, i32* @_ZL3g_2, i32* @_ZL3g_2, i32* @_ZL4g_93, i32* @_ZL3g_2, i32* @_ZL3g_2, i32* @_ZL3g_2, i32* @_ZL3g_2, i32* @_ZL3g_2, i32* @_ZL3g_2]]], align 4
@_ZZL7func_32tiPaE5l_369 = private unnamed_addr constant %struct.S0 { i16 -4, i8 1, i32 -4 }, align 4
@_ZZL7func_32tiPaE5l_382 = private unnamed_addr constant [6 x [1 x i16*]] [[1 x i16*] [i16* @_ZL5g_308], [1 x i16*] zeroinitializer, [1 x i16*] zeroinitializer, [1 x i16*] [i16* @_ZL5g_308], [1 x i16*] zeroinitializer, [1 x i16*] zeroinitializer], align 4
@_ZZL7func_32tiPaE5l_387 = private unnamed_addr constant %struct.S0 { i16 16776, i8 55, i32 1410444512 }, align 4
@_ZZL7func_32tiPaE5l_392 = private unnamed_addr constant [10 x [8 x i8]] [[8 x i8] c"\06\83r\00r\83\06\00", [8 x i8] c"\06\C7r\00r\C7\06\00", [8 x i8] c"\06\83r\00r\83\06\00", [8 x i8] c"\06\C7r\00r\C7\06\00", [8 x i8] c"\06\83r\00r\83\06\00", [8 x i8] c"\06\C7r\00r\C7\06\00", [8 x i8] c"\06\83r\00r\83\06\00", [8 x i8] c"\06\C7r\00r\C7\06\00", [8 x i8] c"\06\83r\00r\83\06\00", [8 x i8] c"\06\C7r\00r\C7\06\00"], align 1
@_ZZL7func_26jPKaPaS1_iE5l_398 = private unnamed_addr constant [9 x [8 x i8]] [[8 x i8] c"c\9E\00\9Eco\00o", [8 x i8] c"co\00oc\9E\00\9E", [8 x i8] c"c\9E\00\9Eco\00o", [8 x i8] c"co\00oc\9E\00\9E", [8 x i8] c"c\9E\00\9Eco\00o", [8 x i8] c"co\00oc\9E\00\9E", [8 x i8] c"c\9E\00\9Eco\00o", [8 x i8] c"co\00oc\9E\00\9E", [8 x i8] c"c\9E\00\9Eco\00o"], align 1
@_ZZL6func_8ijPasS_E6l_1016 = private unnamed_addr constant [5 x [3 x i32*]] [[3 x i32*] [i32* @_ZL5g_257, i32* @_ZL4g_93, i32* @_ZL4g_93], [3 x i32*] [i32* @_ZL5g_257, i32* @_ZL4g_93, i32* @_ZL4g_93], [3 x i32*] [i32* @_ZL5g_257, i32* @_ZL4g_93, i32* @_ZL4g_93], [3 x i32*] [i32* @_ZL5g_257, i32* @_ZL4g_93, i32* @_ZL4g_93], [3 x i32*] [i32* @_ZL5g_257, i32* @_ZL4g_93, i32* @_ZL4g_93]], align 4
@_ZZL6func_5PahE6l_1024 = private unnamed_addr constant [6 x i32*] [i32* null, i32* @_ZL3g_2, i32* @_ZL3g_2, i32* null, i32* @_ZL3g_2, i32* @_ZL3g_2], align 4
@_gm_ = internal global %struct.malloc_state zeroinitializer, align 4
@mparams = internal global %struct.malloc_params zeroinitializer, align 4
@stderr = external constant %struct._IO_FILE*
@.str49 = private unnamed_addr constant [26 x i8] c"max system bytes = %10lu\0A\00", align 1
@.str150 = private unnamed_addr constant [26 x i8] c"system bytes     = %10lu\0A\00", align 1
@.str251 = private unnamed_addr constant [26 x i8] c"in use bytes     = %10lu\0A\00", align 1
@_ZSt7nothrow = external constant %"struct.std::nothrow_t"
@_ZL13__new_handler = internal global void ()* null, align 4

@_ZTVSt9bad_alloc = external unnamed_addr constant [5 x i8*]

@.str352 = private unnamed_addr constant [15 x i8] c"std::bad_alloc\00", align 1
@_ZTVSt20bad_array_new_length = external unnamed_addr constant [5 x i8*]

@.str1453 = private unnamed_addr constant [17 x i8] c"bad_array_length\00", align 1
@_ZTVSt16bad_array_length = external unnamed_addr constant [5 x i8*]
@.str2554 = private unnamed_addr constant [21 x i8] c"bad_array_new_length\00", align 1
@_ZTVN10__cxxabiv120__si_class_type_infoE = external global i8*
@_ZTSSt9bad_alloc = external constant [13 x i8]
@_ZTISt9exception = external constant i8*
@_ZTISt9bad_alloc = external unnamed_addr constant { i8*, i8*, i8* }
@_ZTSSt20bad_array_new_length = external constant [25 x i8]
@_ZTISt20bad_array_new_length = external unnamed_addr constant { i8*, i8*, i8* }
@_ZTSSt16bad_array_length = external constant [21 x i8]
@_ZTISt16bad_array_length = external unnamed_addr constant { i8*, i8*, i8* }
@.str655 = private unnamed_addr constant [9 x i8] c"infinity\00", align 1
@.str1756 = private unnamed_addr constant [4 x i8] c"nan\00", align 1
@decfloat.th = internal unnamed_addr constant [2 x i32] [i32 9007199, i32 254740991], align 4
@decfloat.p10s = internal unnamed_addr constant [8 x i32] [i32 10, i32 100, i32 1000, i32 10000, i32 100000, i32 1000000, i32 10000000, i32 100000000], align 4

declare i32 @strcmp(i8*, i8*)

; Function Attrs: nounwind
define i32 @main(i32 %argc, i8** nocapture %argv) #0 {
entry:
  %cmp = icmp eq i32 %argc, 2
  br i1 %cmp, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %entry
  %arrayidx = getelementptr inbounds i8*, i8** %argv, i32 1
  %0 = load i8*, i8** %arrayidx, align 4
  %call = tail call i32 @strcmp(i8* %0, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str, i32 0, i32 0))
  %cmp1 = icmp eq i32 %call, 0
  %. = zext i1 %cmp1 to i32
  br label %if.end

if.end:                                           ; preds = %land.lhs.true, %entry
  %print_hash_value.0 = phi i32 [ 0, %entry ], [ %., %land.lhs.true ]
  tail call fastcc void @_ZL12crc32_gentabv()
  tail call fastcc void @_ZL6func_1v()
  %1 = load i32, i32* @_ZL3g_2, align 4
  %conv = sext i32 %1 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str1, i32 0, i32 0), i32 %print_hash_value.0)
  %tobool = icmp eq i32 %print_hash_value.0, 0
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc20, %if.end
  %i.075 = phi i32 [ 0, %if.end ], [ %inc21, %for.inc20 ]
  br label %for.body9

for.body9:                                        ; preds = %for.inc.5, %for.cond4.preheader
  %j.074 = phi i32 [ 0, %for.cond4.preheader ], [ %inc18, %for.inc.5 ]
  %arrayidx12 = getelementptr inbounds [4 x [6 x [6 x i8]]], [4 x [6 x [6 x i8]]]* @_ZL4g_52, i32 0, i32 %i.075, i32 %j.074, i32 0
  %2 = load i8, i8* %arrayidx12, align 1
  %conv13 = sext i8 %2 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv13, i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str2, i32 0, i32 0), i32 %print_hash_value.0)
  br i1 %tobool, label %for.inc.1.critedge, label %if.then14

if.then14:                                        ; preds = %for.body9
  %call15 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str3, i32 0, i32 0), i32 %i.075, i32 %j.074, i32 0)
  %arrayidx12.1 = getelementptr inbounds [4 x [6 x [6 x i8]]], [4 x [6 x [6 x i8]]]* @_ZL4g_52, i32 0, i32 %i.075, i32 %j.074, i32 1
  %3 = load i8, i8* %arrayidx12.1, align 1
  %conv13.1 = sext i8 %3 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv13.1, i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str2, i32 0, i32 0), i32 %print_hash_value.0)
  %call15.1 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str3, i32 0, i32 0), i32 %i.075, i32 %j.074, i32 1)
  br label %for.inc.1

for.inc20:                                        ; preds = %for.inc.5
  %inc21 = add nsw i32 %i.075, 1
  %exitcond76 = icmp eq i32 %inc21, 4
  br i1 %exitcond76, label %for.body45, label %for.cond4.preheader

for.body45:                                       ; preds = %for.inc20
  %4 = load i32, i32* @_ZL4g_53, align 4
  %conv23 = sext i32 %4 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv23, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str4, i32 0, i32 0), i32 %print_hash_value.0)
  %5 = load i8, i8* getelementptr inbounds ({ i8, [3 x i8] }, { i8, [3 x i8] }* @_ZL4g_74, i32 0, i32 0), align 4
  %conv24 = zext i8 %5 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv24, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str5, i32 0, i32 0), i32 %print_hash_value.0)
  %6 = load i8, i8* getelementptr inbounds ({ i8, [3 x i8] }, { i8, [3 x i8] }* @_ZL4g_74, i32 0, i32 0), align 4
  %conv25 = sext i8 %6 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv25, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str6, i32 0, i32 0), i32 %print_hash_value.0)
  %7 = load i16, i16* @_ZL4g_77, align 2
  %conv26 = zext i16 %7 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv26, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str7, i32 0, i32 0), i32 %print_hash_value.0)
  %8 = load i16, i16* @_ZL4g_79, align 2
  %conv27 = zext i16 %8 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv27, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str8, i32 0, i32 0), i32 %print_hash_value.0)
  %9 = load i16, i16* @_ZL4g_82, align 2
  %conv28 = zext i16 %9 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv28, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str9, i32 0, i32 0), i32 %print_hash_value.0)
  %10 = load i16, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), align 8
  %conv29 = sext i16 %10 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv29, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str10, i32 0, i32 0), i32 %print_hash_value.0)
  %11 = load i8, i8* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 1), align 2
  %conv30 = sext i8 %11 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv30, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str11, i32 0, i32 0), i32 %print_hash_value.0)
  %12 = load i32, i32* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 2), align 4
  %conv31 = zext i32 %12 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv31, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str12, i32 0, i32 0), i32 %print_hash_value.0)
  %13 = load i32, i32* @_ZL4g_93, align 4
  %conv32 = sext i32 %13 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv32, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str13, i32 0, i32 0), i32 %print_hash_value.0)
  %14 = load i16, i16* @_ZL5g_103, align 2
  %conv33 = zext i16 %14 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv33, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str14, i32 0, i32 0), i32 %print_hash_value.0)
  %15 = load i8, i8* @_ZL5g_126, align 1
  %conv34 = sext i8 %15 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv34, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str15, i32 0, i32 0), i32 %print_hash_value.0)
  %16 = load i32, i32* @_ZL5g_132, align 4
  %conv35 = sext i32 %16 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv35, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str16, i32 0, i32 0), i32 %print_hash_value.0)
  %17 = load i8, i8* @_ZL5g_133, align 1
  %conv36 = sext i8 %17 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv36, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str17, i32 0, i32 0), i32 %print_hash_value.0)
  %18 = load i32, i32* @_ZL5g_135, align 4
  %conv37 = sext i32 %18 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv37, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str18, i32 0, i32 0), i32 %print_hash_value.0)
  %19 = load i8, i8* @_ZL5g_136, align 1
  %conv38 = sext i8 %19 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv38, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str19, i32 0, i32 0), i32 %print_hash_value.0)
  %20 = load i32, i32* @_ZL5g_138, align 4
  %conv39 = zext i32 %20 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv39, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str20, i32 0, i32 0), i32 %print_hash_value.0)
  %21 = load i16, i16* @_ZL5g_162, align 2
  %conv40 = sext i16 %21 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv40, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str21, i32 0, i32 0), i32 %print_hash_value.0)
  %22 = load i16, i16* @_ZL5g_168, align 2
  %conv41 = sext i16 %22 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv41, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str22, i32 0, i32 0), i32 %print_hash_value.0)
  %23 = load i8, i8* @_ZL5g_170, align 1
  %conv42 = zext i8 %23 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv42, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str23, i32 0, i32 0), i32 %print_hash_value.0)
  %tobool48 = icmp eq i32 %print_hash_value.0, 0
  %24 = load i8, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @_ZL5g_171, i32 0, i32 0), align 1
  %conv47 = sext i8 %24 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv47, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str24, i32 0, i32 0), i32 %print_hash_value.0)
  br i1 %tobool48, label %for.inc52.6.thread, label %if.then49.7

for.inc52.6.thread:                               ; preds = %for.body45
  %25 = load i8, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @_ZL5g_171, i32 0, i32 1), align 1
  %conv47.177 = sext i8 %25 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv47.177, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str24, i32 0, i32 0), i32 %print_hash_value.0)
  %26 = load i8, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @_ZL5g_171, i32 0, i32 2), align 1
  %conv47.278 = sext i8 %26 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv47.278, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str24, i32 0, i32 0), i32 %print_hash_value.0)
  %27 = load i8, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @_ZL5g_171, i32 0, i32 3), align 1
  %conv47.379 = sext i8 %27 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv47.379, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str24, i32 0, i32 0), i32 %print_hash_value.0)
  %28 = load i8, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @_ZL5g_171, i32 0, i32 4), align 1
  %conv47.480 = sext i8 %28 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv47.480, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str24, i32 0, i32 0), i32 %print_hash_value.0)
  %29 = load i8, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @_ZL5g_171, i32 0, i32 5), align 1
  %conv47.581 = sext i8 %29 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv47.581, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str24, i32 0, i32 0), i32 %print_hash_value.0)
  %30 = load i8, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @_ZL5g_171, i32 0, i32 6), align 1
  %conv47.682 = sext i8 %30 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv47.682, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str24, i32 0, i32 0), i32 %print_hash_value.0)
  %31 = load i8, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @_ZL5g_171, i32 0, i32 7), align 1
  %conv47.783 = sext i8 %31 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv47.783, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str24, i32 0, i32 0), i32 %print_hash_value.0)
  br label %for.body70

for.body70:                                       ; preds = %if.then49.7, %for.inc52.6.thread
  %32 = load i16, i16* @_ZL5g_172, align 2
  %conv55 = zext i16 %32 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv55, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str26, i32 0, i32 0), i32 %print_hash_value.0)
  %33 = load i32, i32* @_ZL5g_257, align 4
  %conv56 = sext i32 %33 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv56, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str27, i32 0, i32 0), i32 %print_hash_value.0)
  %34 = load i8, i8* @_ZL5g_304, align 1
  %conv57 = sext i8 %34 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv57, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str28, i32 0, i32 0), i32 %print_hash_value.0)
  %35 = load i16, i16* @_ZL5g_308, align 2
  %conv58 = zext i16 %35 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv58, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str29, i32 0, i32 0), i32 %print_hash_value.0)
  %36 = load i16, i16* @_ZL5g_361, align 2
  %conv59 = zext i16 %36 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv59, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str30, i32 0, i32 0), i32 %print_hash_value.0)
  %37 = load i32, i32* @_ZL5g_400, align 4
  %conv60 = sext i32 %37 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv60, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str31, i32 0, i32 0), i32 %print_hash_value.0)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 -1, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str32, i32 0, i32 0), i32 %print_hash_value.0)
  %38 = load i16, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL5g_471, i32 0, i32 0), align 8
  %conv62 = sext i16 %38 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv62, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str33, i32 0, i32 0), i32 %print_hash_value.0)
  %39 = load i8, i8* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL5g_471, i32 0, i32 1), align 2
  %conv63 = sext i8 %39 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv63, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str34, i32 0, i32 0), i32 %print_hash_value.0)
  %40 = load i32, i32* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL5g_471, i32 0, i32 2), align 4
  %conv64 = zext i32 %40 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv64, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str35, i32 0, i32 0), i32 %print_hash_value.0)
  %41 = load i32, i32* @_ZL5g_546, align 4
  %conv65 = sext i32 %41 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv65, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str36, i32 0, i32 0), i32 %print_hash_value.0)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 2891502546, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str37, i32 0, i32 0), i32 %print_hash_value.0)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 255, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str38, i32 0, i32 0), i32 %print_hash_value.0)
  %tobool73 = icmp eq i32 %print_hash_value.0, 0
  tail call fastcc void @_ZL15transparent_crcyPci(i64 3761930302, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str39, i32 0, i32 0), i32 %print_hash_value.0)
  br i1 %tobool73, label %for.inc77.8.thread, label %if.then74.9

for.inc77.8.thread:                               ; preds = %for.body70
  tail call fastcc void @_ZL15transparent_crcyPci(i64 3761930302, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str39, i32 0, i32 0), i32 %print_hash_value.0)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 1950933783, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str39, i32 0, i32 0), i32 %print_hash_value.0)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 3761930302, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str39, i32 0, i32 0), i32 %print_hash_value.0)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 3761930302, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str39, i32 0, i32 0), i32 %print_hash_value.0)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 1950933783, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str39, i32 0, i32 0), i32 %print_hash_value.0)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 3761930302, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str39, i32 0, i32 0), i32 %print_hash_value.0)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 3761930302, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str39, i32 0, i32 0), i32 %print_hash_value.0)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 1950933783, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str39, i32 0, i32 0), i32 %print_hash_value.0)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 3761930302, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str39, i32 0, i32 0), i32 %print_hash_value.0)
  br label %for.body88

for.body88:                                       ; preds = %if.then74.9, %for.inc77.8.thread
  %42 = load i8, i8* @_ZL6g_1010, align 1
  %conv80 = sext i8 %42 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv80, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str40, i32 0, i32 0), i32 %print_hash_value.0)
  %43 = load i8, i8* @_ZL6g_1025, align 1
  %conv81 = zext i8 %43 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv81, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str41, i32 0, i32 0), i32 %print_hash_value.0)
  %44 = load i8, i8* @_ZL6g_1136, align 1
  %conv82 = zext i8 %44 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv82, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str42, i32 0, i32 0), i32 %print_hash_value.0)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 1, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str43, i32 0, i32 0), i32 %print_hash_value.0)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 7, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str44, i32 0, i32 0), i32 %print_hash_value.0)
  %45 = load i32, i32* @_ZL6g_1198, align 4
  %conv85 = sext i32 %45 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv85, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str45, i32 0, i32 0), i32 %print_hash_value.0)
  %tobool91 = icmp eq i32 %print_hash_value.0, 0
  tail call fastcc void @_ZL15transparent_crcyPci(i64 4294967293, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str46, i32 0, i32 0), i32 %print_hash_value.0)
  br i1 %tobool91, label %for.inc95.6.thread, label %if.then92.7

for.inc95.6.thread:                               ; preds = %for.body88
  tail call fastcc void @_ZL15transparent_crcyPci(i64 2870230573, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str46, i32 0, i32 0), i32 %print_hash_value.0)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 2870230573, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str46, i32 0, i32 0), i32 %print_hash_value.0)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 4294967293, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str46, i32 0, i32 0), i32 %print_hash_value.0)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 2870230573, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str46, i32 0, i32 0), i32 %print_hash_value.0)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 2870230573, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str46, i32 0, i32 0), i32 %print_hash_value.0)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 4294967293, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str46, i32 0, i32 0), i32 %print_hash_value.0)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 2870230573, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str46, i32 0, i32 0), i32 %print_hash_value.0)
  br label %for.inc95.7

if.then92.7:                                      ; preds = %for.body88
  %call93 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str25, i32 0, i32 0), i32 0)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 2870230573, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str46, i32 0, i32 0), i32 %print_hash_value.0)
  %call93.1 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str25, i32 0, i32 0), i32 1)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 2870230573, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str46, i32 0, i32 0), i32 %print_hash_value.0)
  %call93.2 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str25, i32 0, i32 0), i32 2)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 4294967293, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str46, i32 0, i32 0), i32 %print_hash_value.0)
  %call93.3 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str25, i32 0, i32 0), i32 3)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 2870230573, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str46, i32 0, i32 0), i32 %print_hash_value.0)
  %call93.4 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str25, i32 0, i32 0), i32 4)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 2870230573, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str46, i32 0, i32 0), i32 %print_hash_value.0)
  %call93.5 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str25, i32 0, i32 0), i32 5)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 4294967293, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str46, i32 0, i32 0), i32 %print_hash_value.0)
  %call93.6 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str25, i32 0, i32 0), i32 6)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 2870230573, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str46, i32 0, i32 0), i32 %print_hash_value.0)
  %call93.7 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str25, i32 0, i32 0), i32 7)
  br label %for.inc95.7

for.inc95.7:                                      ; preds = %if.then92.7, %for.inc95.6.thread
  %46 = load i32, i32* @_ZL13crc32_context, align 4
  %xor = xor i32 %46, -1
  tail call fastcc void @_ZL17platform_main_endji(i32 %xor)
  ret i32 0

if.then74.9:                                      ; preds = %for.body70
  %call75 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str25, i32 0, i32 0), i32 0)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 3761930302, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str39, i32 0, i32 0), i32 %print_hash_value.0)
  %call75.1 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str25, i32 0, i32 0), i32 1)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 1950933783, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str39, i32 0, i32 0), i32 %print_hash_value.0)
  %call75.2 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str25, i32 0, i32 0), i32 2)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 3761930302, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str39, i32 0, i32 0), i32 %print_hash_value.0)
  %call75.3 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str25, i32 0, i32 0), i32 3)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 3761930302, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str39, i32 0, i32 0), i32 %print_hash_value.0)
  %call75.4 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str25, i32 0, i32 0), i32 4)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 1950933783, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str39, i32 0, i32 0), i32 %print_hash_value.0)
  %call75.5 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str25, i32 0, i32 0), i32 5)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 3761930302, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str39, i32 0, i32 0), i32 %print_hash_value.0)
  %call75.6 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str25, i32 0, i32 0), i32 6)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 3761930302, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str39, i32 0, i32 0), i32 %print_hash_value.0)
  %call75.7 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str25, i32 0, i32 0), i32 7)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 1950933783, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str39, i32 0, i32 0), i32 %print_hash_value.0)
  %call75.8 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str25, i32 0, i32 0), i32 8)
  tail call fastcc void @_ZL15transparent_crcyPci(i64 3761930302, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str39, i32 0, i32 0), i32 %print_hash_value.0)
  %call75.9 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str25, i32 0, i32 0), i32 9)
  br label %for.body88

if.then49.7:                                      ; preds = %for.body45
  %call50 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str25, i32 0, i32 0), i32 0)
  %47 = load i8, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @_ZL5g_171, i32 0, i32 1), align 1
  %conv47.1 = sext i8 %47 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv47.1, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str24, i32 0, i32 0), i32 %print_hash_value.0)
  %call50.1 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str25, i32 0, i32 0), i32 1)
  %48 = load i8, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @_ZL5g_171, i32 0, i32 2), align 1
  %conv47.2 = sext i8 %48 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv47.2, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str24, i32 0, i32 0), i32 %print_hash_value.0)
  %call50.2 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str25, i32 0, i32 0), i32 2)
  %49 = load i8, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @_ZL5g_171, i32 0, i32 3), align 1
  %conv47.3 = sext i8 %49 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv47.3, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str24, i32 0, i32 0), i32 %print_hash_value.0)
  %call50.3 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str25, i32 0, i32 0), i32 3)
  %50 = load i8, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @_ZL5g_171, i32 0, i32 4), align 1
  %conv47.4 = sext i8 %50 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv47.4, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str24, i32 0, i32 0), i32 %print_hash_value.0)
  %call50.4 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str25, i32 0, i32 0), i32 4)
  %51 = load i8, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @_ZL5g_171, i32 0, i32 5), align 1
  %conv47.5 = sext i8 %51 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv47.5, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str24, i32 0, i32 0), i32 %print_hash_value.0)
  %call50.5 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str25, i32 0, i32 0), i32 5)
  %52 = load i8, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @_ZL5g_171, i32 0, i32 6), align 1
  %conv47.6 = sext i8 %52 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv47.6, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str24, i32 0, i32 0), i32 %print_hash_value.0)
  %call50.6 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str25, i32 0, i32 0), i32 6)
  %53 = load i8, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @_ZL5g_171, i32 0, i32 7), align 1
  %conv47.7 = sext i8 %53 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv47.7, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str24, i32 0, i32 0), i32 %print_hash_value.0)
  %call50.7 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str25, i32 0, i32 0), i32 7)
  br label %for.body70

for.inc.1.critedge:                               ; preds = %for.body9
  %arrayidx12.1.c = getelementptr inbounds [4 x [6 x [6 x i8]]], [4 x [6 x [6 x i8]]]* @_ZL4g_52, i32 0, i32 %i.075, i32 %j.074, i32 1
  %54 = load i8, i8* %arrayidx12.1.c, align 1
  %conv13.1.c = sext i8 %54 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv13.1.c, i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str2, i32 0, i32 0), i32 %print_hash_value.0)
  br label %for.inc.1

for.inc.1:                                        ; preds = %for.inc.1.critedge, %if.then14
  %arrayidx12.2 = getelementptr inbounds [4 x [6 x [6 x i8]]], [4 x [6 x [6 x i8]]]* @_ZL4g_52, i32 0, i32 %i.075, i32 %j.074, i32 2
  %55 = load i8, i8* %arrayidx12.2, align 1
  %conv13.2 = sext i8 %55 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv13.2, i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str2, i32 0, i32 0), i32 %print_hash_value.0)
  br i1 %tobool, label %for.inc.3.critedge, label %if.then14.2

if.then14.2:                                      ; preds = %for.inc.1
  %call15.2 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str3, i32 0, i32 0), i32 %i.075, i32 %j.074, i32 2)
  %arrayidx12.3 = getelementptr inbounds [4 x [6 x [6 x i8]]], [4 x [6 x [6 x i8]]]* @_ZL4g_52, i32 0, i32 %i.075, i32 %j.074, i32 3
  %56 = load i8, i8* %arrayidx12.3, align 1
  %conv13.3 = sext i8 %56 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv13.3, i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str2, i32 0, i32 0), i32 %print_hash_value.0)
  %call15.3 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str3, i32 0, i32 0), i32 %i.075, i32 %j.074, i32 3)
  br label %for.inc.3

for.inc.3.critedge:                               ; preds = %for.inc.1
  %arrayidx12.3.c = getelementptr inbounds [4 x [6 x [6 x i8]]], [4 x [6 x [6 x i8]]]* @_ZL4g_52, i32 0, i32 %i.075, i32 %j.074, i32 3
  %57 = load i8, i8* %arrayidx12.3.c, align 1
  %conv13.3.c = sext i8 %57 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv13.3.c, i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str2, i32 0, i32 0), i32 %print_hash_value.0)
  br label %for.inc.3

for.inc.3:                                        ; preds = %for.inc.3.critedge, %if.then14.2
  %arrayidx12.4 = getelementptr inbounds [4 x [6 x [6 x i8]]], [4 x [6 x [6 x i8]]]* @_ZL4g_52, i32 0, i32 %i.075, i32 %j.074, i32 4
  %58 = load i8, i8* %arrayidx12.4, align 1
  %conv13.4 = sext i8 %58 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv13.4, i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str2, i32 0, i32 0), i32 %print_hash_value.0)
  br i1 %tobool, label %for.inc.5.critedge, label %if.then14.4

if.then14.4:                                      ; preds = %for.inc.3
  %call15.4 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str3, i32 0, i32 0), i32 %i.075, i32 %j.074, i32 4)
  %arrayidx12.5 = getelementptr inbounds [4 x [6 x [6 x i8]]], [4 x [6 x [6 x i8]]]* @_ZL4g_52, i32 0, i32 %i.075, i32 %j.074, i32 5
  %59 = load i8, i8* %arrayidx12.5, align 1
  %conv13.5 = sext i8 %59 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv13.5, i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str2, i32 0, i32 0), i32 %print_hash_value.0)
  %call15.5 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str3, i32 0, i32 0), i32 %i.075, i32 %j.074, i32 5)
  br label %for.inc.5

for.inc.5.critedge:                               ; preds = %for.inc.3
  %arrayidx12.5.c = getelementptr inbounds [4 x [6 x [6 x i8]]], [4 x [6 x [6 x i8]]]* @_ZL4g_52, i32 0, i32 %i.075, i32 %j.074, i32 5
  %60 = load i8, i8* %arrayidx12.5.c, align 1
  %conv13.5.c = sext i8 %60 to i64
  tail call fastcc void @_ZL15transparent_crcyPci(i64 %conv13.5.c, i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str2, i32 0, i32 0), i32 %print_hash_value.0)
  br label %for.inc.5

for.inc.5:                                        ; preds = %for.inc.5.critedge, %if.then14.4
  %inc18 = add nsw i32 %j.074, 1
  %exitcond = icmp eq i32 %inc18, 6
  br i1 %exitcond, label %for.inc20, label %for.body9
}

; Function Attrs: nounwind
define internal fastcc void @_ZL12crc32_gentabv() #0 {
entry:
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %entry
  %i.08 = phi i32 [ 0, %entry ], [ %inc, %for.body3 ]
  %and = and i32 %i.08, 1
  %tobool = icmp eq i32 %and, 0
  %shr = lshr i32 %i.08, 1
  %xor = xor i32 %shr, -306674912
  %crc.1 = select i1 %tobool, i32 %shr, i32 %xor
  %and.1 = and i32 %crc.1, 1
  %tobool.1 = icmp eq i32 %and.1, 0
  %shr.1 = lshr i32 %crc.1, 1
  %xor.1 = xor i32 %shr.1, -306674912
  %crc.1.1 = select i1 %tobool.1, i32 %shr.1, i32 %xor.1
  %and.2 = and i32 %crc.1.1, 1
  %tobool.2 = icmp eq i32 %and.2, 0
  %shr.2 = lshr i32 %crc.1.1, 1
  %xor.2 = xor i32 %shr.2, -306674912
  %crc.1.2 = select i1 %tobool.2, i32 %shr.2, i32 %xor.2
  %and.3 = and i32 %crc.1.2, 1
  %tobool.3 = icmp eq i32 %and.3, 0
  %shr.3 = lshr i32 %crc.1.2, 1
  %xor.3 = xor i32 %shr.3, -306674912
  %crc.1.3 = select i1 %tobool.3, i32 %shr.3, i32 %xor.3
  %and.4 = and i32 %crc.1.3, 1
  %tobool.4 = icmp eq i32 %and.4, 0
  %shr.4 = lshr i32 %crc.1.3, 1
  %xor.4 = xor i32 %shr.4, -306674912
  %crc.1.4 = select i1 %tobool.4, i32 %shr.4, i32 %xor.4
  %and.5 = and i32 %crc.1.4, 1
  %tobool.5 = icmp eq i32 %and.5, 0
  %shr.5 = lshr i32 %crc.1.4, 1
  %xor.5 = xor i32 %shr.5, -306674912
  %crc.1.5 = select i1 %tobool.5, i32 %shr.5, i32 %xor.5
  %and.6 = and i32 %crc.1.5, 1
  %tobool.6 = icmp eq i32 %and.6, 0
  %shr.6 = lshr i32 %crc.1.5, 1
  %xor.6 = xor i32 %shr.6, -306674912
  %crc.1.6 = select i1 %tobool.6, i32 %shr.6, i32 %xor.6
  %and.7 = and i32 %crc.1.6, 1
  %tobool.7 = icmp eq i32 %and.7, 0
  %shr.7 = lshr i32 %crc.1.6, 1
  %xor.7 = xor i32 %shr.7, -306674912
  %crc.1.7 = select i1 %tobool.7, i32 %shr.7, i32 %xor.7
  %arrayidx = getelementptr inbounds [256 x i32], [256 x i32]* @_ZL9crc32_tab, i32 0, i32 %i.08
  store i32 %crc.1.7, i32* %arrayidx, align 4
  %inc = add nsw i32 %i.08, 1
  %exitcond = icmp eq i32 %inc, 256
  br i1 %exitcond, label %for.end6, label %for.body3

for.end6:                                         ; preds = %for.body3
  ret void
}

; Function Attrs: nounwind
define internal fastcc void @_ZL6func_1v() #0 {
for.end97:
  %agg.tmp = alloca i64, align 8
  %tmpcast = bitcast i64* %agg.tmp to %struct.S0*
  store i32 0, i32* @_ZL3g_2, align 4
  %call = call fastcc zeroext i8 @_ZL7func_20iit(i32 0, i16 zeroext -16644)
  %0 = load i32, i32* @_ZL3g_2, align 4
  %conv98 = trunc i32 %0 to i8
  %call105 = call fastcc zeroext i16 @_ZL26safe_mul_func_uint16_t_u_utt(i16 zeroext 7, i16 zeroext 1)
  %conv106 = zext i16 %call105 to i32
  %call118 = call fastcc zeroext i8 @_ZL25safe_mul_func_uint8_t_u_uhh(i8 zeroext 1, i8 zeroext 23)
  %conv119 = zext i8 %call118 to i32
  %cmp120 = icmp uge i32 %conv106, %conv119
  %conv121 = zext i1 %cmp120 to i32
  %call122 = call fastcc zeroext i8 @_ZL28safe_rshift_func_uint8_t_u_shi(i8 zeroext %conv98, i32 %conv121)
  %cmp124 = icmp ult i8 %call, %call122
  %conv125 = zext i1 %cmp124 to i8
  store i64 -21471878022, i64* %agg.tmp, align 8
  call fastcc void @_ZL7func_14aPaS_2S0j(i8 signext %conv125, %struct.S0* byval align 4 %tmpcast)
  call fastcc void @_ZL6func_5Pah()
  %1 = load i32, i32* @_ZL6g_1198, align 4
  %tobool = icmp eq i32 %1, 0
  br i1 %tobool, label %if.else1462, label %for.end311

for.end311:                                       ; preds = %for.end97
  store i8 25, i8* @_ZL6g_1136, align 1
  %bf.load = load i24, i24* bitcast ({ i8, [3 x i8] }* @_ZL6g_1162 to i24*), align 4
  %bf.clear = and i24 %bf.load, -262144
  %bf.set = or i24 %bf.clear, zext (i1 icmp slt (i32 zext (i1 icmp sge (i32 zext (i1 icmp eq (i8* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL5g_471, i32 0, i32 1), i8* @_ZL6g_1010) to i32), i32 1) to i32), i32 710400751) to i24)
  store i24 %bf.set, i24* bitcast ({ i8, [3 x i8] }* @_ZL6g_1162 to i24*), align 4
  store i16 1, i16* @_ZL5g_162, align 2
  br label %cleanup1590

if.else1462:                                      ; preds = %for.end97
  %2 = load i8**, i8*** @_ZL5g_984, align 4
  %3 = load i8*, i8** %2, align 4
  store i8 0, i8* %3, align 1
  %call1515 = call fastcc signext i8 @_ZL24safe_div_func_int8_t_s_saa(i8 signext 0, i8 signext 1)
  %call1517 = call fastcc zeroext i8 @_ZL28safe_lshift_func_uint8_t_u_shi(i8 zeroext %call1515, i32 58)
  %conv1518 = zext i8 %call1517 to i16
  %call1519 = call fastcc zeroext i16 @_ZL29safe_lshift_func_uint16_t_u_utj(i16 zeroext %conv1518, i32 14)
  %cmp1522 = icmp eq i16 %call1519, 4
  %conv1523 = zext i1 %cmp1522 to i32
  %xor15271 = or i32 %conv1523, 4
  store i32 %xor15271, i32* @_ZL5g_135, align 4
  %4 = load i8, i8* @_ZL5g_136, align 1
  %5 = load i8, i8* @_ZL5g_133, align 1
  %and15422 = and i8 %5, %4
  %cmp1543 = icmp ne i8 %and15422, 1
  %conv1544 = zext i1 %cmp1543 to i8
  store i8 %conv1544, i8* @_ZL6g_1010, align 1
  %6 = load i32, i32* @_ZL5g_135, align 4
  %call1565 = call fastcc i32 @_ZL25safe_sub_func_int32_t_s_sii(i32 1, i32 %6)
  %conv1566 = trunc i32 %call1565 to i8
  %7 = load i8*, i8** @_ZL5g_985, align 4
  store i8 %conv1566, i8* %7, align 1
  br label %cleanup1590

cleanup1590:                                      ; preds = %if.else1462, %for.end311
  ret void
}

; Function Attrs: nounwind
define internal fastcc void @_ZL15transparent_crcyPci(i64 %val, i8* %vname, i32 %flag) #0 {
entry:
  tail call fastcc void @_ZL12crc32_8bytesy(i64 %val)
  %tobool = icmp eq i32 %flag, 0
  br i1 %tobool, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %0 = load i32, i32* @_ZL13crc32_context, align 4
  %xor = xor i32 %0, -1
  %call = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([36 x i8], [36 x i8]* @.str48, i32 0, i32 0), i8* %vname, i32 %xor)
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture, ...) #0

; Function Attrs: nounwind
define internal fastcc void @_ZL17platform_main_endji(i32 %crc) #0 {
entry:
  %call = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str47, i32 0, i32 0), i32 %crc)
  ret void
}

; Function Attrs: nounwind
define internal fastcc void @_ZL12crc32_8bytesy(i64 %val) #0 {
entry:
  %conv = trunc i64 %val to i8
  tail call fastcc void @_ZL10crc32_byteh(i8 zeroext %conv)
  %shr1 = lshr i64 %val, 8
  %conv3 = trunc i64 %shr1 to i8
  tail call fastcc void @_ZL10crc32_byteh(i8 zeroext %conv3)
  %shr4 = lshr i64 %val, 16
  %conv6 = trunc i64 %shr4 to i8
  tail call fastcc void @_ZL10crc32_byteh(i8 zeroext %conv6)
  %shr7 = lshr i64 %val, 24
  %conv9 = trunc i64 %shr7 to i8
  tail call fastcc void @_ZL10crc32_byteh(i8 zeroext %conv9)
  %shr10 = lshr i64 %val, 32
  %conv12 = trunc i64 %shr10 to i8
  tail call fastcc void @_ZL10crc32_byteh(i8 zeroext %conv12)
  %shr13 = lshr i64 %val, 40
  %conv15 = trunc i64 %shr13 to i8
  tail call fastcc void @_ZL10crc32_byteh(i8 zeroext %conv15)
  %shr16 = lshr i64 %val, 48
  %conv18 = trunc i64 %shr16 to i8
  tail call fastcc void @_ZL10crc32_byteh(i8 zeroext %conv18)
  %shr19 = lshr i64 %val, 56
  %conv21 = trunc i64 %shr19 to i8
  tail call fastcc void @_ZL10crc32_byteh(i8 zeroext %conv21)
  ret void
}

; Function Attrs: nounwind
define internal fastcc void @_ZL10crc32_byteh(i8 zeroext %b) #0 {
entry:
  %0 = load i32, i32* @_ZL13crc32_context, align 4
  %shr = lshr i32 %0, 8
  %conv = zext i8 %b to i32
  %.masked = and i32 %0, 255
  %and1 = xor i32 %.masked, %conv
  %arrayidx = getelementptr inbounds [256 x i32], [256 x i32]* @_ZL9crc32_tab, i32 0, i32 %and1
  %1 = load i32, i32* %arrayidx, align 4
  %xor2 = xor i32 %shr, %1
  store i32 %xor2, i32* @_ZL13crc32_context, align 4
  ret void
}

; Function Attrs: nounwind
declare void @llvm.memcpy.p0i8.p0i8.i32(i8* nocapture, i8* nocapture, i32, i32, i1) #1

; Function Attrs: nounwind
define internal fastcc void @_ZL6func_5Pah() #0 {
for.end14:
  %0 = load i8, i8* @_ZL6g_1025, align 1
  %inc15 = add i8 %0, 1
  store i8 %inc15, i8* @_ZL6g_1025, align 1
  %1 = load i32, i32* @_ZL5g_400, align 4
  %tobool = icmp eq i32 %1, 0
  br i1 %tobool, label %if.else, label %if.then

if.then:                                          ; preds = %for.end14
  %2 = load i64, i64* bitcast (%struct.S0* @_ZL4g_83 to i64*), align 8
  store i64 %2, i64* bitcast (%struct.S0* @_ZL5g_471 to i64*), align 8
  br label %if.end

if.else:                                          ; preds = %for.end14
  %and = and i32 %1, 23
  store i32 %and, i32* @_ZL5g_400, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

; Function Attrs: nounwind readonly
define internal fastcc i8* @_ZL6func_8ijPasS_(i16 signext %p_12) #2 {
entry:
  %0 = load i8**, i8*** @_ZL5g_984, align 4
  %1 = load i8*, i8** %0, align 4
  ret i8* %1
}

; Function Attrs: nounwind
define internal fastcc void @_ZL7func_14aPaS_2S0j(i8 signext %p_15, %struct.S0* byval nocapture align 4 %p_18) #0 {
entry:
  store i16 0, i16* @_ZL5g_172, align 2
  %0 = load i32, i32* @_ZL4g_53, align 4
  %f0 = getelementptr inbounds %struct.S0, %struct.S0* %p_18, i32 0, i32 0
  %1 = load i16, i16* %f0, align 4
  %conv1 = sext i16 %1 to i32
  %f2 = getelementptr inbounds %struct.S0, %struct.S0* %p_18, i32 0, i32 2
  %2 = load i32, i32* %f2, align 4
  %3 = load i32, i32* @_ZL4g_93, align 4
  %cmp2 = icmp ne i32 %2, %3
  %conv3 = zext i1 %cmp2 to i32
  %cmp4 = icmp sgt i32 %conv1, %conv3
  %conv5 = zext i1 %cmp4 to i32
  %cmp6 = icmp sge i32 %0, %conv5
  %conv7 = zext i1 %cmp6 to i16
  %call = tail call fastcc zeroext i16 @_ZL29safe_rshift_func_uint16_t_u_sti(i16 zeroext %conv7, i32 10)
  %conv8 = zext i16 %call to i32
  store i32 %conv8, i32* @_ZL5g_132, align 4
  ret void
}

; Function Attrs: nounwind
define internal fastcc zeroext i8 @_ZL7func_20iit(i32 %p_22, i16 zeroext %p_23) #0 {
for.end7:
  %agg.tmp = alloca %union.U1, align 4
  %agg.tmp9 = alloca %struct.S0, align 4
  %0 = load i32, i32* @_ZL3g_2, align 4
  %conv = trunc i32 %0 to i16
  %conv11 = trunc i32 %p_22 to i8
  store i8 %conv11, i8* getelementptr inbounds ([4 x [6 x [6 x i8]]], [4 x [6 x [6 x i8]]]* @_ZL4g_52, i32 0, i32 1, i32 3, i32 1), align 1
  %dec = add i16 %p_23, -1
  %l_60.sroa.0.0.idx = getelementptr inbounds %struct.S0, %struct.S0* %agg.tmp9, i32 0, i32 0
  store i16 26262, i16* %l_60.sroa.0.0.idx, align 4
  %l_60.sroa.1.2.idx = getelementptr inbounds %struct.S0, %struct.S0* %agg.tmp9, i32 0, i32 1
  store i8 73, i8* %l_60.sroa.1.2.idx, align 2
  %l_60.sroa.2.3.raw_cast = bitcast %struct.S0* %agg.tmp9 to i8*
  %l_60.sroa.2.3.raw_idx = getelementptr inbounds i8, i8* %l_60.sroa.2.3.raw_cast, i32 3
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %l_60.sroa.2.3.raw_idx, i8* getelementptr ([5 x i8], [5 x i8]* bitcast (i8* getelementptr (i8, i8* bitcast (%struct.S0* @_ZZL7func_20iitE4l_60 to i8*), i32 3) to [5 x i8]*), i32 0, i32 0), i32 5, i32 1, i1 false)
  call fastcc void @_ZL7func_492S0(%union.U1* sret %agg.tmp, %struct.S0* byval align 4 %agg.tmp9)
  %.lobit = lshr i32 %0, 31
  %1 = trunc i32 %.lobit to i8
  %call33 = call fastcc signext i16 @_ZL7func_40ia(i8 signext %1)
  %cmp35 = icmp eq i16 %call33, 3880
  %conv36 = zext i1 %cmp35 to i32
  %2 = load i16, i16* @_ZL5g_361, align 2
  %conv37 = zext i16 %2 to i32
  %xor = xor i32 %conv37, %conv36
  %conv38 = trunc i32 %xor to i16
  store i16 %conv38, i16* @_ZL5g_361, align 2
  %call43 = call fastcc signext i8 @_ZL24safe_sub_func_int8_t_s_saa(i8 signext -1)
  %conv44 = sext i8 %call43 to i32
  %call45 = call fastcc signext i16 @_ZL7func_32tiPa(i16 zeroext %conv, i32 %conv44)
  %3 = load i32, i32* @_ZL5g_400, align 4
  %and53 = and i32 %3, -98
  store i32 %and53, i32* @_ZL5g_400, align 4
  %4 = load i8, i8* getelementptr inbounds ([4 x [6 x [6 x i8]]], [4 x [6 x [6 x i8]]]* @_ZL4g_52, i32 0, i32 0, i32 4, i32 5), align 1
  %call104 = call fastcc zeroext i8 @_ZL28safe_rshift_func_uint8_t_u_shi(i8 zeroext %4, i32 1)
  %call105 = call fastcc zeroext i8 @_ZL28safe_lshift_func_uint8_t_u_shi(i8 zeroext %call104, i32 1)
  %cmp124 = icmp ne i8 %call105, 0
  %conv125 = zext i1 %cmp124 to i32
  %5 = load i16, i16* @_ZL4g_77, align 2
  %conv571 = trunc i16 %5 to i8
  %6 = load i32, i32* @_ZL4g_53, align 4
  %7 = load i8, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @_ZL5g_171, i32 0, i32 6), align 1
  %conv432 = sext i8 %7 to i32
  %xor434 = xor i32 %conv432, %6
  br label %lbl_496

lbl_496.loopexit:                                 ; preds = %for.body302
  store i16 %call312.lobit, i16* @_ZL5g_162, align 2
  store i32 %conv357, i32* @_ZL5g_400, align 4
  %phitmp38 = add i32 %l_404.0, -1
  br label %lbl_496

lbl_496:                                          ; preds = %lbl_496.loopexit, %for.end7
  %l_404.0 = phi i32 [ -1124763087, %for.end7 ], [ %phitmp38, %lbl_496.loopexit ]
  %p_23.addr.0 = phi i16 [ %dec, %for.end7 ], [ %conv332, %lbl_496.loopexit ]
  %p_22.addr.0 = phi i32 [ %p_22, %for.end7 ], [ %p_22.addr.235, %lbl_496.loopexit ]
  store i32 0, i32* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 2), align 4
  %tobool156 = icmp ne i32 %p_22.addr.0, 0
  %phitmp = zext i1 %tobool156 to i8
  %conv428 = trunc i32 %p_22.addr.0 to i8
  %8 = load i16, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL5g_471, i32 0, i32 0), align 8
  %conv438 = sext i16 %8 to i32
  %_ZL5g_400.promoted = load i32, i32* @_ZL5g_400, align 4
  %_ZL5g_138.promoted = load i32, i32* @_ZL5g_138, align 4
  br label %for.body59

for.body59:                                       ; preds = %lor.end440.3, %lbl_496
  %and12827 = phi i32 [ %_ZL5g_138.promoted, %lbl_496 ], [ %and128, %lor.end440.3 ]
  %9 = phi i32 [ %_ZL5g_400.promoted, %lbl_496 ], [ 0, %lor.end440.3 ]
  %cmp107 = icmp ult i32 %9, 255
  br i1 %cmp107, label %lor.end, label %lor.rhs

lor.rhs:                                          ; preds = %for.body59
  %conv117 = trunc i32 %9 to i8
  %call118 = call fastcc zeroext i8 @_ZL25safe_add_func_uint8_t_u_uhh(i8 zeroext -15, i8 zeroext %conv117)
  %conv119 = zext i8 %call118 to i16
  store i16 %conv119, i16* @_ZL5g_361, align 2
  br label %lor.end

lor.end:                                          ; preds = %lor.rhs, %for.body59
  %10 = load i32, i32* @_ZL4g_93, align 4
  %or127 = or i32 %9, %10
  %and128 = and i32 %and12827, %or127
  br i1 %tobool156, label %if.then, label %for.body191

if.then:                                          ; preds = %lor.end
  %call158 = call fastcc signext i8 @_ZL24safe_add_func_int8_t_s_saa(i8 signext -8, i8 signext -1)
  %conv159 = sext i8 %call158 to i16
  store i8 -1, i8* getelementptr inbounds ([4 x [6 x [6 x i8]]], [4 x [6 x [6 x i8]]]* @_ZL4g_52, i32 0, i32 1, i32 3, i32 1), align 1
  store i16 8, i16* @_ZL5g_361, align 2
  %call183 = call fastcc signext i16 @_ZL25safe_mul_func_int16_t_s_sss(i16 signext %conv159, i16 signext 1)
  %conv184 = sext i16 %call183 to i32
  %11 = load i32, i32* @_ZL4g_93, align 4
  %xor185 = xor i32 %11, %conv184
  store i32 %xor185, i32* @_ZL4g_93, align 4
  %tobool198 = icmp eq i32 %or127, 0
  br i1 %tobool198, label %for.body412, label %land.lhs.true

for.body191:                                      ; preds = %lor.end
  store i32 %or127, i32* @_ZL5g_400, align 4
  store i32 %and128, i32* @_ZL5g_138, align 4
  %conv192 = trunc i32 %or127 to i8
  br label %cleanup583

land.lhs.true:                                    ; preds = %if.then
  %12 = load i16, i16* @_ZL5g_361, align 2
  %conv201 = zext i16 %12 to i32
  %xor202 = xor i32 %conv201, 57244
  %conv203 = trunc i32 %xor202 to i16
  store i16 %conv203, i16* @_ZL5g_361, align 2
  %call204 = call fastcc zeroext i16 @_ZL26safe_mul_func_uint16_t_u_utt(i16 zeroext 0, i16 zeroext %conv203)
  %13 = load i16, i16* @_ZL5g_168, align 2
  %conv207 = sext i16 %13 to i32
  %cmp209 = icmp ne i16 %call204, 0
  %conv210 = zext i1 %cmp209 to i32
  %cmp212 = icmp slt i32 %conv210, %conv207
  %conv213 = zext i1 %cmp212 to i16
  store i16 %conv213, i16* @_ZL5g_168, align 2
  br i1 %cmp212, label %if.then214, label %for.body412

if.then214:                                       ; preds = %land.lhs.true
  store i32 %or127, i32* @_ZL5g_400, align 4
  store i32 %and128, i32* @_ZL5g_138, align 4
  %tobool295 = icmp eq i32 %or127, 0
  br i1 %tobool295, label %for.cond300.preheader, label %cleanup583

for.cond300.preheader:                            ; preds = %if.then214
  %14 = load i64, i64* bitcast (%struct.S0* @_ZL4g_83 to i64*), align 8
  %15 = load i16, i16* @_ZL5g_168, align 2
  %call312 = call fastcc signext i16 @_ZL28safe_lshift_func_int16_t_s_usj(i16 signext %15, i32 -1)
  %call312.lobit = lshr i16 %call312, 15
  %call331 = call fastcc signext i8 @_ZL24safe_div_func_int8_t_s_saa(i8 signext 1, i8 signext -7)
  %conv332 = sext i8 %call331 to i16
  %call354 = call fastcc signext i16 @_ZL28safe_lshift_func_int16_t_s_usj(i16 signext 1, i32 1631231813)
  %call356 = call fastcc signext i16 @_ZL25safe_mul_func_int16_t_s_sss(i16 signext %call354, i16 signext 0)
  %conv357 = sext i16 %call356 to i32
  %tobool358 = icmp eq i32 %l_404.0, 0
  br label %for.body302

for.cond300:                                      ; preds = %for.body302
  %sub = add nsw i32 %p_22.addr.235, -1
  %cmp301 = icmp sgt i32 %p_22.addr.235, 0
  br i1 %cmp301, label %for.body302, label %cleanup583.loopexit32

for.body302:                                      ; preds = %for.cond300, %for.cond300.preheader
  %p_22.addr.235 = phi i32 [ 3, %for.cond300.preheader ], [ %sub, %for.cond300 ]
  store i64 %14, i64* bitcast (%struct.S0* @_ZL5g_471 to i64*), align 8
  store i32 1, i32* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL5g_471, i32 0, i32 2), align 4
  br i1 %tobool358, label %for.cond300, label %lbl_496.loopexit

for.body412:                                      ; preds = %land.lhs.true, %if.then
  %16 = load i16, i16* @_ZL5g_168, align 2
  %call568 = call fastcc i32 @_ZL26safe_mod_func_uint32_t_u_ujj(i32 0, i32 -13150355)
  %cmp569 = icmp ne i32 %call568, 0
  %conv570 = zext i1 %cmp569 to i8
  %call572 = call fastcc signext i8 @_ZL24safe_mul_func_int8_t_s_saa(i8 signext %conv570, i8 signext %conv571)
  %conv433 = sext i16 %16 to i32
  %xor435 = xor i32 %xor434, %conv433
  %cmp436 = icmp slt i32 %xor435, %p_22.addr.0
  %conv437 = zext i1 %cmp436 to i32
  %cmp439 = icmp sgt i32 %conv437, %conv438
  %_ZL5g_361.promoted = load i16, i16* @_ZL5g_361, align 2
  br i1 %tobool156, label %lor.end440.2.thread, label %lor.rhs421.3

lor.end440.2.thread:                              ; preds = %for.body412
  %conv44250 = zext i16 %_ZL5g_361.promoted to i32
  %xor443.257 = xor i32 %conv44250, 1
  br label %lor.end440.3

cleanup583.loopexit:                              ; preds = %lor.end440.3
  store i32 0, i32* @_ZL5g_400, align 4
  store i32 %and128, i32* @_ZL5g_138, align 4
  br label %cleanup583

cleanup583.loopexit32:                            ; preds = %for.cond300
  store i16 %call312.lobit, i16* @_ZL5g_162, align 2
  store i32 %conv357, i32* @_ZL5g_400, align 4
  br label %cleanup583

cleanup583:                                       ; preds = %cleanup583.loopexit32, %cleanup583.loopexit, %if.then214, %for.body191
  %cleanup.dest.slot.2 = phi i1 [ false, %for.body191 ], [ true, %cleanup583.loopexit ], [ false, %cleanup583.loopexit32 ], [ true, %if.then214 ]
  %retval.6 = phi i8 [ %conv192, %for.body191 ], [ undef, %cleanup583.loopexit ], [ -100, %cleanup583.loopexit32 ], [ undef, %if.then214 ]
  %17 = load i8, i8* @_ZL5g_170, align 1
  %.retval.6 = select i1 %cleanup.dest.slot.2, i8 %17, i8 %retval.6
  ret i8 %.retval.6

lor.rhs421.3:                                     ; preds = %for.body412
  %conv441 = zext i1 %cmp439 to i32
  %conv442 = zext i16 %_ZL5g_361.promoted to i32
  %xor443 = xor i32 %conv442, %conv441
  %conv441.1 = zext i1 %cmp439 to i32
  %xor533.1 = xor i32 %xor443, %conv441.1
  %conv441.2 = zext i1 %cmp439 to i32
  %xor443.2 = xor i32 %xor533.1, %conv441.2
  %call422.3 = call fastcc signext i16 @_ZL25safe_mul_func_int16_t_s_sss(i16 signext %16, i16 signext %p_23.addr.0)
  %tobool423.3 = icmp eq i16 %call422.3, 0
  %phitmp..3 = select i1 %tobool423.3, i8 %phitmp, i8 1
  %call429.3 = call fastcc zeroext i8 @_ZL25safe_sub_func_uint8_t_u_uhh(i8 zeroext %phitmp..3, i8 zeroext %conv428)
  %conv430.3 = zext i8 %call429.3 to i32
  %xor431.3 = xor i32 %conv430.3, %conv125
  store i32 %xor431.3, i32* @_ZL5g_546, align 4
  br label %lor.end440.3

lor.end440.3:                                     ; preds = %lor.rhs421.3, %lor.end440.2.thread
  %xor443.258 = phi i32 [ %xor443.2, %lor.rhs421.3 ], [ %xor443.257, %lor.end440.2.thread ]
  %18 = phi i1 [ %cmp439, %lor.rhs421.3 ], [ true, %lor.end440.2.thread ]
  %conv441.3 = zext i1 %18 to i32
  %xor533.3 = xor i32 %xor443.258, %conv441.3
  %conv534.3 = trunc i32 %xor533.3 to i16
  %conv573 = sext i8 %call572 to i32
  store i32 %conv573, i32* @_ZL4g_93, align 4
  store i16 %conv534.3, i16* @_ZL5g_361, align 2
  store i8 0, i8* getelementptr inbounds ({ i8, [3 x i8] }, { i8, [3 x i8] }* @_ZL4g_74, i32 0, i32 0), align 4
  store i16 %16, i16* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 0), align 8
  %19 = load i32, i32* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 2), align 4
  %add581 = add i32 %19, 1
  store i32 %add581, i32* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 2), align 4
  %cmp58 = icmp eq i32 %add581, 0
  br i1 %cmp58, label %for.body59, label %cleanup583.loopexit
}

; Function Attrs: nounwind readnone
define internal fastcc zeroext i8 @_ZL28safe_rshift_func_uint8_t_u_shi(i8 zeroext %left, i32 %right) #3 {
entry:
  %0 = icmp ugt i32 %right, 31
  %conv = zext i8 %left to i32
  %shr = select i1 %0, i32 0, i32 %right
  %conv.shr = lshr i32 %conv, %shr
  %conv3 = trunc i32 %conv.shr to i8
  ret i8 %conv3
}

; Function Attrs: nounwind readnone
define internal fastcc zeroext i16 @_ZL26safe_mul_func_uint16_t_u_utt(i16 zeroext %ui1, i16 zeroext %ui2) #3 {
entry:
  %conv = zext i16 %ui1 to i32
  %conv1 = zext i16 %ui2 to i32
  %mul = mul i32 %conv1, %conv
  %conv2 = trunc i32 %mul to i16
  ret i16 %conv2
}

; Function Attrs: nounwind readnone
define internal fastcc zeroext i8 @_ZL25safe_mul_func_uint8_t_u_uhh(i8 zeroext %ui1, i8 zeroext %ui2) #3 {
entry:
  %conv = zext i8 %ui1 to i32
  %conv1 = zext i8 %ui2 to i32
  %mul = mul i32 %conv1, %conv
  %conv2 = trunc i32 %mul to i8
  ret i8 %conv2
}

; Function Attrs: nounwind readnone
define internal fastcc zeroext i8 @_ZL25safe_sub_func_uint8_t_u_uhh(i8 zeroext %ui1, i8 zeroext %ui2) #3 {
entry:
  %conv = zext i8 %ui1 to i32
  %conv1 = zext i8 %ui2 to i32
  %sub = sub nsw i32 %conv, %conv1
  %conv2 = trunc i32 %sub to i8
  ret i8 %conv2
}

; Function Attrs: nounwind readnone
define internal fastcc i32 @_ZL26safe_div_func_uint32_t_u_ujj(i32 %ui1, i32 %ui2) #3 {
entry:
  %cmp = icmp eq i32 %ui2, 0
  br i1 %cmp, label %cond.end, label %cond.false

cond.false:                                       ; preds = %entry
  %div = udiv i32 %ui1, %ui2
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %entry
  %cond = phi i32 [ %div, %cond.false ], [ %ui1, %entry ]
  ret i32 %cond
}

; Function Attrs: nounwind readnone
define internal fastcc zeroext i16 @_ZL26safe_mod_func_uint16_t_u_utt(i16 zeroext %ui1, i16 zeroext %ui2) #3 {
entry:
  %cmp = icmp eq i16 %ui2, 0
  %conv1 = zext i16 %ui1 to i32
  br i1 %cmp, label %cond.end, label %cond.false

cond.false:                                       ; preds = %entry
  %0 = urem i16 %ui1, %ui2
  %rem = zext i16 %0 to i32
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %entry
  %cond = phi i32 [ %rem, %cond.false ], [ %conv1, %entry ]
  %conv4 = trunc i32 %cond to i16
  ret i16 %conv4
}

; Function Attrs: nounwind readnone
define internal fastcc zeroext i8 @_ZL28safe_lshift_func_uint8_t_u_shi(i8 zeroext %left, i32 %right) #3 {
entry:
  %0 = icmp ugt i32 %right, 31
  br i1 %0, label %cond.true, label %lor.lhs.false2

lor.lhs.false2:                                   ; preds = %entry
  %conv = zext i8 %left to i32
  %shr = lshr i32 255, %right
  %cmp3 = icmp sgt i32 %conv, %shr
  br i1 %cmp3, label %cond.true, label %cond.false

cond.true:                                        ; preds = %lor.lhs.false2, %entry
  %conv4 = zext i8 %left to i32
  br label %cond.end

cond.false:                                       ; preds = %lor.lhs.false2
  %shl = shl i32 %conv, %right
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 [ %conv4, %cond.true ], [ %shl, %cond.false ]
  %conv6 = trunc i32 %cond to i8
  ret i8 %conv6
}

; Function Attrs: nounwind readnone
define internal fastcc i32 @_ZL25safe_sub_func_int32_t_s_sii(i32 %si1, i32 %si2) #3 {
entry:
  %xor = xor i32 %si2, %si1
  %and = and i32 %xor, -2147483648
  %xor2 = xor i32 %and, %si1
  %sub = sub nsw i32 %xor2, %si2
  %xor3 = xor i32 %sub, %si2
  %and4 = and i32 %xor3, %xor
  %cmp = icmp slt i32 %and4, 0
  %sub5 = select i1 %cmp, i32 0, i32 %si2
  %si1.sub5 = sub nsw i32 %si1, %sub5
  ret i32 %si1.sub5
}

; Function Attrs: nounwind readnone
define internal fastcc signext i16 @_ZL25safe_mul_func_int16_t_s_sss(i16 signext %si1, i16 signext %si2) #3 {
entry:
  %conv = sext i16 %si1 to i32
  %conv1 = sext i16 %si2 to i32
  %mul = mul nsw i32 %conv1, %conv
  %conv2 = trunc i32 %mul to i16
  ret i16 %conv2
}

; Function Attrs: nounwind readnone
define internal fastcc zeroext i16 @_ZL29safe_lshift_func_uint16_t_u_utj(i16 zeroext %left, i32 %right) #3 {
entry:
  %cmp = icmp ugt i32 %right, 31
  br i1 %cmp, label %cond.true, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %entry
  %conv = zext i16 %left to i32
  %shr = lshr i32 65535, %right
  %cmp1 = icmp sgt i32 %conv, %shr
  br i1 %cmp1, label %cond.true, label %cond.false

cond.true:                                        ; preds = %lor.lhs.false, %entry
  %conv2 = zext i16 %left to i32
  br label %cond.end

cond.false:                                       ; preds = %lor.lhs.false
  %shl = shl i32 %conv, %right
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 [ %conv2, %cond.true ], [ %shl, %cond.false ]
  %conv4 = trunc i32 %cond to i16
  ret i16 %conv4
}

; Function Attrs: nounwind readnone
define internal fastcc signext i16 @_ZL28safe_lshift_func_int16_t_s_usj(i16 signext %left, i32 %right) #3 {
entry:
  %conv = sext i16 %left to i32
  %cmp = icmp slt i16 %left, 0
  %cmp1 = icmp ugt i32 %right, 31
  %or.cond = or i1 %cmp, %cmp1
  %shr = lshr i32 32767, %right
  %cmp4 = icmp sgt i32 %conv, %shr
  %or.cond6 = or i1 %or.cond, %cmp4
  %shl = select i1 %or.cond6, i32 0, i32 %right
  %cond = shl i32 %conv, %shl
  %conv7 = trunc i32 %cond to i16
  ret i16 %conv7
}

; Function Attrs: nounwind readnone
define internal fastcc signext i16 @_ZL25safe_sub_func_int16_t_s_sss(i16 signext %si2) #3 {
entry:
  %conv11 = zext i16 %si2 to i32
  %sub = sub nsw i32 1, %conv11
  %conv2 = trunc i32 %sub to i16
  ret i16 %conv2
}

; Function Attrs: nounwind
define internal fastcc void @_ZL29safe_rshift_func_uint16_t_u_utj() #0 {
entry:
  unreachable
}

; Function Attrs: nounwind readnone
define internal fastcc signext i16 @_ZL25safe_add_func_int16_t_s_sss(i16 signext %si1, i16 signext %si2) #3 {
entry:
  %conv3 = zext i16 %si1 to i32
  %conv14 = zext i16 %si2 to i32
  %add = add nsw i32 %conv14, %conv3
  %conv2 = trunc i32 %add to i16
  ret i16 %conv2
}

; Function Attrs: nounwind
define internal fastcc void @_ZL26safe_sub_func_uint32_t_u_ujj() #0 {
entry:
  unreachable
}

; Function Attrs: nounwind readnone
define internal fastcc i32 @_ZL25safe_mod_func_int32_t_s_sii(i32 %si1, i32 %si2) #3 {
entry:
  %cmp = icmp eq i32 %si2, 0
  br i1 %cmp, label %cond.end, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %entry
  %cmp1 = icmp eq i32 %si1, -2147483648
  %cmp2 = icmp eq i32 %si2, -1
  %or.cond = and i1 %cmp1, %cmp2
  br i1 %or.cond, label %cond.end, label %cond.false

cond.false:                                       ; preds = %lor.lhs.false
  %rem = srem i32 %si1, %si2
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %lor.lhs.false, %entry
  %cond = phi i32 [ %rem, %cond.false ], [ %si1, %lor.lhs.false ], [ %si1, %entry ]
  ret i32 %cond
}

; Function Attrs: nounwind readnone
define internal fastcc i32 @_ZL25safe_add_func_int32_t_s_sii(i32 %si1) #3 {
entry:
  %cmp3 = icmp sgt i32 %si1, 1806657196
  %add = add nsw i32 %si1, 340826451
  %si1.add = select i1 %cmp3, i32 %si1, i32 %add
  ret i32 %si1.add
}

; Function Attrs: nounwind readnone
define internal fastcc zeroext i16 @_ZL29safe_rshift_func_uint16_t_u_sti(i16 zeroext %left, i32 %right) #3 {
entry:
  %0 = icmp ugt i32 %right, 31
  %conv = zext i16 %left to i32
  %shr = select i1 %0, i32 0, i32 %right
  %conv.shr = lshr i32 %conv, %shr
  %conv3 = trunc i32 %conv.shr to i16
  ret i16 %conv3
}

; Function Attrs: nounwind
define internal fastcc void @_ZL31safe_unary_minus_func_int16_t_ss() #0 {
entry:
  unreachable
}

; Function Attrs: nounwind readnone
define internal fastcc i32 @_ZL26safe_mod_func_uint32_t_u_ujj(i32 %ui1, i32 %ui2) #3 {
entry:
  %cmp = icmp eq i32 %ui2, 0
  br i1 %cmp, label %cond.end, label %cond.false

cond.false:                                       ; preds = %entry
  %rem = urem i32 %ui1, %ui2
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %entry
  %cond = phi i32 [ %rem, %cond.false ], [ %ui1, %entry ]
  ret i32 %cond
}

; Function Attrs: nounwind readnone
define internal fastcc zeroext i16 @_ZL26safe_add_func_uint16_t_u_utt(i16 zeroext %ui1, i16 zeroext %ui2) #3 {
entry:
  %conv = zext i16 %ui1 to i32
  %conv1 = zext i16 %ui2 to i32
  %add = add nsw i32 %conv1, %conv
  %conv2 = trunc i32 %add to i16
  ret i16 %conv2
}

; Function Attrs: nounwind readnone
define internal fastcc zeroext i8 @_ZL25safe_add_func_uint8_t_u_uhh(i8 zeroext %ui1, i8 zeroext %ui2) #3 {
entry:
  %conv = zext i8 %ui1 to i32
  %conv1 = zext i8 %ui2 to i32
  %add = add nsw i32 %conv1, %conv
  %conv2 = trunc i32 %add to i8
  ret i8 %conv2
}

; Function Attrs: nounwind
define internal fastcc void @_ZL25safe_div_func_int32_t_s_sii() #0 {
entry:
  unreachable
}

; Function Attrs: nounwind readnone
define internal fastcc signext i8 @_ZL27safe_lshift_func_int8_t_s_sai(i32 %right) #3 {
entry:
  %0 = icmp ugt i32 %right, 31
  %shr = lshr i32 127, %right
  %cmp6 = icmp slt i32 %shr, 1
  %or.cond = or i1 %0, %cmp6
  br i1 %or.cond, label %cond.end, label %cond.false

cond.false:                                       ; preds = %entry
  %shl = shl i32 1, %right
  %phitmp = trunc i32 %shl to i8
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %entry
  %cond = phi i8 [ %phitmp, %cond.false ], [ 1, %entry ]
  ret i8 %cond
}

; Function Attrs: nounwind readnone
define internal fastcc signext i8 @_ZL24safe_sub_func_int8_t_s_saa(i8 signext %si1) #3 {
entry:
  %conv1 = zext i8 %si1 to i32
  %sub = add nsw i32 %conv1, 183
  %conv2 = trunc i32 %sub to i8
  ret i8 %conv2
}

; Function Attrs: nounwind
define internal fastcc void @_ZL26safe_div_func_uint16_t_u_utt() #0 {
entry:
  unreachable
}

; Function Attrs: nounwind readnone
define internal fastcc signext i16 @_ZL28safe_rshift_func_int16_t_s_usj(i16 signext %left, i32 %right) #3 {
entry:
  %conv = sext i16 %left to i32
  %cmp = icmp slt i16 %left, 0
  %cmp1 = icmp ugt i32 %right, 31
  %or.cond = or i1 %cmp, %cmp1
  %shr = select i1 %or.cond, i32 0, i32 %right
  %cond = ashr i32 %conv, %shr
  %conv4 = trunc i32 %cond to i16
  ret i16 %conv4
}

; Function Attrs: nounwind
define internal fastcc void @_ZL28safe_lshift_func_uint8_t_u_uhj() #0 {
entry:
  unreachable
}

; Function Attrs: nounwind readnone
define internal fastcc signext i8 @_ZL24safe_mod_func_int8_t_s_saa(i8 signext %si1, i8 signext %si2) #3 {
entry:
  %conv = sext i8 %si2 to i32
  %cmp = icmp eq i8 %si2, 0
  br i1 %cmp, label %cond.true, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %entry
  %cmp2 = icmp eq i8 %si1, -128
  %cmp4 = icmp eq i8 %si2, -1
  %or.cond = and i1 %cmp2, %cmp4
  br i1 %or.cond, label %cond.true, label %cond.false

cond.true:                                        ; preds = %lor.lhs.false, %entry
  %conv5 = sext i8 %si1 to i32
  br label %cond.end

cond.false:                                       ; preds = %lor.lhs.false
  %conv1 = sext i8 %si1 to i32
  %rem = srem i32 %conv1, %conv
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 [ %conv5, %cond.true ], [ %rem, %cond.false ]
  %conv8 = trunc i32 %cond to i8
  ret i8 %conv8
}

; Function Attrs: nounwind readnone
define internal fastcc signext i8 @_ZL24safe_div_func_int8_t_s_saa(i8 signext %si1, i8 signext %si2) #3 {
entry:
  %conv = sext i8 %si2 to i32
  %cmp = icmp eq i8 %si2, 0
  br i1 %cmp, label %cond.true, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %entry
  %cmp2 = icmp eq i8 %si1, -128
  %cmp4 = icmp eq i8 %si2, -1
  %or.cond = and i1 %cmp2, %cmp4
  br i1 %or.cond, label %cond.true, label %cond.false

cond.true:                                        ; preds = %lor.lhs.false, %entry
  %conv5 = sext i8 %si1 to i32
  br label %cond.end

cond.false:                                       ; preds = %lor.lhs.false
  %conv1 = sext i8 %si1 to i32
  %div = sdiv i32 %conv1, %conv
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 [ %conv5, %cond.true ], [ %div, %cond.false ]
  %conv8 = trunc i32 %cond to i8
  ret i8 %conv8
}

; Function Attrs: nounwind readnone
define internal fastcc zeroext i16 @_ZL29safe_lshift_func_uint16_t_u_sti(i16 zeroext %left, i32 %right) #3 {
entry:
  %0 = icmp ugt i32 %right, 31
  br i1 %0, label %cond.true, label %lor.lhs.false2

lor.lhs.false2:                                   ; preds = %entry
  %conv = zext i16 %left to i32
  %shr = lshr i32 65535, %right
  %cmp3 = icmp sgt i32 %conv, %shr
  br i1 %cmp3, label %cond.true, label %cond.false

cond.true:                                        ; preds = %lor.lhs.false2, %entry
  %conv4 = zext i16 %left to i32
  br label %cond.end

cond.false:                                       ; preds = %lor.lhs.false2
  %shl = shl i32 %conv, %right
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 [ %conv4, %cond.true ], [ %shl, %cond.false ]
  %conv6 = trunc i32 %cond to i16
  ret i16 %conv6
}

; Function Attrs: nounwind
define internal fastcc signext i16 @_ZL7func_32tiPa(i16 zeroext %p_33, i32 %p_34) #0 {
return:
  store i32 1, i32* @_ZL5g_135, align 4
  %0 = load i8, i8* @_ZL5g_126, align 1
  %call8 = tail call fastcc signext i8 @_ZL24safe_mod_func_int8_t_s_saa(i8 signext %0, i8 signext 0)
  %conv9 = sext i8 %call8 to i32
  %1 = load i16, i16* @_ZL5g_168, align 2
  %conv10 = sext i16 %1 to i32
  %cmp11 = icmp eq i32 %conv9, %conv10
  %conv12 = zext i1 %cmp11 to i32
  %2 = load i8, i8* @_ZL5g_136, align 1
  %conv131 = zext i8 %2 to i32
  %or = or i32 %conv12, %conv131
  %conv14 = trunc i32 %or to i8
  store i8 %conv14, i8* @_ZL5g_136, align 1
  %3 = load i32, i32* @_ZL5g_132, align 4
  %or27 = or i32 %3, 1
  %call28 = tail call fastcc i32 @_ZL26safe_div_func_uint32_t_u_ujj(i32 -10, i32 %or27)
  %conv29 = trunc i32 %call28 to i16
  store i16 %conv29, i16* @_ZL4g_82, align 2
  %conv34 = zext i16 %p_33 to i32
  store i8 0, i8* @_ZL5g_133, align 1
  %4 = load i8, i8* getelementptr inbounds ({ i8, [3 x i8] }, { i8, [3 x i8] }* @_ZL4g_74, i32 0, i32 0), align 4
  %not.tobool68 = icmp ne i8 %4, 0
  %5 = zext i1 %not.tobool68 to i32
  %or82 = or i32 %5, %conv34
  %6 = load i8, i8* @_ZL5g_126, align 1
  %conv832 = zext i8 %6 to i32
  %xor = xor i32 %or82, %conv832
  %conv84 = trunc i32 %xor to i8
  store i8 %conv84, i8* @_ZL5g_126, align 1
  %lnot = icmp ne i8 %conv84, 0
  %xor87 = zext i1 %lnot to i32
  store i32 %xor87, i32* @_ZL5g_132, align 4
  ret i16 -20646
}

; Function Attrs: nounwind
define internal fastcc signext i16 @_ZL7func_40ia(i8 signext %p_42) #0 {
entry:
  %call = tail call fastcc signext i8 @_ZL24safe_mod_func_int8_t_s_saa(i8 signext 0, i8 signext 7)
  %conv2 = sext i8 %call to i32
  %0 = load i32, i32* @_ZL3g_2, align 4
  %conv3 = trunc i32 %0 to i8
  %call4 = tail call fastcc signext i8 @_ZL30safe_unary_minus_func_int8_t_sa(i8 signext %conv3)
  %conv5 = sext i8 %call4 to i16
  store i16 %conv5, i16* bitcast ({ i8, [3 x i8] }* @_ZL4g_74 to i16*), align 4
  %conv6 = sext i8 %call4 to i32
  %cmp8 = icmp sgt i32 %conv6, zext (i1 icmp ne (i8* getelementptr inbounds ([8 x i8], [8 x i8]* @_ZL5g_171, i32 0, i32 5), i8* @_ZL5g_304) to i32)
  %conv9 = zext i1 %cmp8 to i32
  %cmp10 = icmp ne i32 %conv2, %conv9
  %conv11 = zext i1 %cmp10 to i16
  %call12 = tail call fastcc signext i16 @_ZL28safe_lshift_func_int16_t_s_usj(i16 signext %conv11, i32 7)
  %conv13 = sext i16 %call12 to i32
  store i32 %conv13, i32* @_ZL5g_257, align 4
  %tobool = icmp eq i16 %call12, 0
  br i1 %tobool, label %land.end, label %land.rhs

land.rhs:                                         ; preds = %entry
  %1 = load i16, i16* @_ZL5g_308, align 2
  %conv17 = zext i16 %1 to i32
  %xor = xor i32 %conv17, %conv13
  %conv18 = trunc i32 %xor to i16
  store i16 %conv18, i16* @_ZL5g_308, align 2
  %lnot = icmp eq i16 %conv18, 0
  %conv20 = zext i1 %lnot to i32
  %2 = load i16, i16* @_ZL4g_79, align 2
  %conv21 = zext i16 %2 to i32
  %xor22 = xor i32 %conv20, %conv21
  %conv23 = trunc i32 %xor22 to i16
  store i16 %conv23, i16* @_ZL4g_79, align 2
  %tobool24 = icmp ne i16 %conv23, 0
  %phitmp = zext i1 %tobool24 to i16
  br label %land.end

land.end:                                         ; preds = %land.rhs, %entry
  %3 = phi i16 [ 0, %entry ], [ %phitmp, %land.rhs ]
  store i16 %3, i16* bitcast ({ i8, [3 x i8] }* @_ZL4g_74 to i16*), align 4
  %conv31 = sext i8 %p_42 to i16
  ret i16 %conv31
}

; Function Attrs: nounwind
define internal fastcc void @_ZL7func_492S0(%union.U1* noalias nocapture sret %agg.result, %struct.S0* byval nocapture align 4 %p_50) #0 {
entry:
  %l_131 = alloca i32, align 4
  %l_129 = alloca i32, align 4
  store i32 1179615507, i32* %l_131, align 4
  store i32 3, i32* @_ZL4g_53, align 4
  %f1 = getelementptr inbounds %struct.S0, %struct.S0* %p_50, i32 0, i32 1
  %f0 = getelementptr inbounds %struct.S0, %struct.S0* %p_50, i32 0, i32 0
  %f2 = getelementptr inbounds %struct.S0, %struct.S0* %p_50, i32 0, i32 2
  br label %for.end86

for.end86:                                        ; preds = %for.inc1370, %entry
  store i32 1795078696, i32* %l_129, align 4
  store i8 0, i8* %f1, align 2
  br label %for.body90

for.body90:                                       ; preds = %for.inc1329, %for.end86
  %0 = load i16, i16* %f0, align 4
  %conv110 = sext i16 %0 to i32
  %call111 = call fastcc i32 @_ZL26safe_div_func_uint32_t_u_ujj(i32 0, i32 %conv110)
  %1 = load i32, i32* @_ZL4g_53, align 4
  %call112 = call fastcc i32 @_ZL25safe_mod_func_int32_t_s_sii(i32 %call111, i32 %1)
  %cmp113 = icmp ult i32 %call112, -1508745334
  %conv114 = zext i1 %cmp113 to i32
  %2 = load i16, i16* @_ZL4g_77, align 2
  %conv116 = zext i16 %2 to i32
  %xor117 = xor i32 %conv116, %conv114
  %conv118 = trunc i32 %xor117 to i16
  store i16 %conv118, i16* @_ZL4g_77, align 2
  store i16 %conv118, i16* @_ZL4g_79, align 2
  %and120 = and i32 %xor117, 1
  %3 = load i32, i32* @_ZL4g_53, align 4
  %cmp121 = icmp eq i32 %and120, %3
  %conv122 = zext i1 %cmp121 to i16
  store i16 %conv122, i16* @_ZL4g_82, align 2
  %4 = load i32, i32* %f2, align 4
  %conv127 = trunc i32 %4 to i8
  %call128 = call fastcc zeroext i8 @_ZL25safe_sub_func_uint8_t_u_uhh(i8 zeroext -5, i8 zeroext %conv127)
  %tobool129 = icmp eq i8 %call128, 0
  br i1 %tobool129, label %cleanup1367, label %land.lhs.true

land.lhs.true:                                    ; preds = %for.body90
  %5 = load i8, i8* %f1, align 2
  %tobool131 = icmp eq i8 %5, 0
  br i1 %tobool131, label %cleanup1367, label %for.end139

for.end139:                                       ; preds = %land.lhs.true
  %6 = load i8, i8* getelementptr inbounds ([4 x [6 x [6 x i8]]], [4 x [6 x [6 x i8]]]* @_ZL4g_52, i32 0, i32 1, i32 3, i32 1), align 1
  %tobool140 = icmp eq i8 %6, 0
  br i1 %tobool140, label %for.end582, label %for.inc1370

for.end582:                                       ; preds = %for.end139
  %7 = load i8, i8* %f1, align 2
  %conv554 = sext i8 %7 to i32
  %add = add nsw i32 %conv554, 1
  %8 = load i32, i32* @_ZL4g_53, align 4
  %add555 = add nsw i32 %8, 1
  %arrayidx559 = getelementptr inbounds [4 x [6 x [6 x i8]]], [4 x [6 x [6 x i8]]]* @_ZL4g_52, i32 0, i32 %conv554, i32 %add555, i32 %add
  %9 = load i8, i8* %arrayidx559, align 1
  %conv560 = sext i8 %9 to i32
  %10 = load i32, i32* %f2, align 4
  %xor561 = xor i32 %conv560, %and120
  %or563 = or i32 %xor561, %10
  %xor561.1 = xor i32 %conv560, %or563
  %or563.1 = or i32 %xor561.1, %10
  %xor561.2 = xor i32 %conv560, %or563.1
  %or563.2 = or i32 %xor561.2, %10
  %xor561.3 = xor i32 %conv560, %or563.2
  %or563.3 = or i32 %xor561.3, %10
  store i32 %or563.3, i32* @_ZL4g_93, align 4
  store i16 0, i16* %f0, align 4
  %11 = load i32, i32* getelementptr inbounds (%struct.S0, %struct.S0* @_ZL4g_83, i32 0, i32 2), align 4
  %conv637 = trunc i32 %11 to i16
  %call638 = call fastcc signext i16 @_ZL25safe_sub_func_int16_t_s_sss(i16 signext %conv637)
  %tobool639 = icmp eq i16 %call638, 0
  br i1 %tobool639, label %if.else1214, label %if.then640

if.then640:                                       ; preds = %for.end582
  %12 = load i32, i32* @_ZL5g_138, align 4
  %inc733 = add i32 %12, 1
  store i32 %inc733, i32* @_ZL5g_138, align 4
  %13 = load i8, i8* getelementptr inbounds ([4 x [6 x [6 x i8]]], [4 x [6 x [6 x i8]]]* @_ZL4g_52, i32 0, i32 0, i32 1, i32 2), align 1
  %tobool734 = icmp eq i8 %13, 0
  br i1 %tobool734, label %if.end1219.loopexit, label %for.inc1329

if.else1214:                                      ; preds = %for.end582
  %14 = load i8, i8* %f1, align 2
  %tobool1216 = icmp eq i8 %14, 0
  br i1 %tobool1216, label %if.end1219, label %for.inc1370

if.end1219.loopexit:                              ; preds = %if.then640
  store i16 -28, i16* @_ZL4g_77, align 2
  br label %if.end1219

if.end1219:                                       ; preds = %if.end1219.loopexit, %if.else1214
  store i32 0, i32* %l_129, align 4
  store i32 1, i32* %l_131, align 4
  %15 = load i8, i8* %f1, align 2
  %tobool1257 = icmp eq i8 %15, 0
  br i1 %tobool1257, label %lor.lhs.false, label %for.end1334.loopexit742

lor.lhs.false:                                    ; preds = %if.end1219
  %16 = load i16, i16* %f0, align 4
  %tobool1259 = icmp eq i16 %16, 0
  br i1 %tobool1259, label %if.else1312, label %for.end1334.loopexit742

if.else1312:                                      ; preds = %lor.lhs.false
  store i32* %l_129, i32** getelementptr inbounds ([2 x [10 x i32*]], [2 x [10 x i32*]]* @_ZL5g_197, i32 0, i32 1, i32 9), align 4
  br label %for.inc1370

for.inc1329:                                      ; preds = %if.then640
  %17 = load i8, i8* %f1, align 2
  %conv1331700 = zext i8 %17 to i32
  %add1332 = add nsw i32 %conv1331700, 1
  %conv1333 = trunc i32 %add1332 to i8
  store i8 %conv1333, i8* %f1, align 2
  %cmp89 = icmp slt i8 %conv1333, 4
  br i1 %cmp89, label %for.body90, label %for.inc1370

for.end1334.loopexit742:                          ; preds = %lor.lhs.false, %if.end1219
  store i8 13, i8* @_ZL5g_170, align 1
  br label %for.inc1370

cleanup1367:                                      ; preds = %land.lhs.true, %for.body90
  %18 = load i32, i32* @_ZL4g_93, align 4
  %and635 = and i32 %18, -6
  store i32 %and635, i32* @_ZL4g_93, align 4
  %19 = load i8*, i8** bitcast ({ i8, [3 x i8] }* @_ZZL7func_492S0E5l_119 to i8**), align 4
  %20 = getelementptr inbounds %union.U1, %union.U1* %agg.result, i32 0, i32 0
  store i8* %19, i8** %20, align 4
  br label %cleanup1381

for.inc1370:                                      ; preds = %for.end1334.loopexit742, %for.inc1329, %if.else1312, %if.else1214, %for.end139
  store i32* %l_131, i32** getelementptr inbounds ([2 x [10 x i32*]], [2 x [10 x i32*]]* @_ZL5g_197, i32 0, i32 1, i32 7), align 4
  store i8 -1, i8* @_ZL5g_136, align 1
  %21 = load i32, i32* @_ZL4g_53, align 4
  %sub1371 = add nsw i32 %21, -1
  store i32 %sub1371, i32* @_ZL4g_53, align 4
  %cmp = icmp sgt i32 %21, 0
  br i1 %cmp, label %for.end86, label %for.end1372

for.end1372:                                      ; preds = %for.inc1370
  store i8 0, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @_ZL5g_171, i32 0, i32 6), align 1
  %22 = load i8*, i8** bitcast ({ i8, [3 x i8] }* @_ZL4g_74 to i8**), align 4
  %23 = getelementptr inbounds %union.U1, %union.U1* %agg.result, i32 0, i32 0
  store i8* %22, i8** %23, align 4
  br label %cleanup1381

cleanup1381:                                      ; preds = %for.end1372, %cleanup1367
  ret void
}

; Function Attrs: nounwind readnone
define internal fastcc signext i8 @_ZL24safe_add_func_int8_t_s_saa(i8 signext %si1, i8 signext %si2) #3 {
entry:
  %conv3 = zext i8 %si1 to i32
  %conv14 = zext i8 %si2 to i32
  %add = add nsw i32 %conv14, %conv3
  %conv2 = trunc i32 %add to i8
  ret i8 %conv2
}

; Function Attrs: nounwind readnone
define internal fastcc signext i8 @_ZL24safe_mul_func_int8_t_s_saa(i8 signext %si1, i8 signext %si2) #3 {
entry:
  %conv = sext i8 %si1 to i32
  %conv1 = sext i8 %si2 to i32
  %mul = mul nsw i32 %conv1, %conv
  %conv2 = trunc i32 %mul to i8
  ret i8 %conv2
}

; Function Attrs: nounwind readnone
define internal fastcc signext i16 @_ZL28safe_rshift_func_int16_t_s_ssi(i16 signext %left) #3 {
entry:
  %conv = sext i16 %left to i32
  %left.lobit = lshr i16 %left, 15
  %0 = zext i16 %left.lobit to i32
  %.not = xor i32 %0, 1
  %cond = ashr i32 %conv, %.not
  %conv6 = trunc i32 %cond to i16
  ret i16 %conv6
}

; Function Attrs: nounwind readnone
define internal fastcc signext i8 @_ZL30safe_unary_minus_func_int8_t_sa(i8 signext %si) #3 {
entry:
  %conv2 = zext i8 %si to i32
  %sub = sub nsw i32 0, %conv2
  %conv1 = trunc i32 %sub to i8
  ret i8 %conv1
}

; Function Attrs: nounwind readnone
define internal fastcc i32 @_ZL31safe_unary_minus_func_int32_t_si(i32 %si) #3 {
entry:
  %sub = sub nsw i32 0, %si
  ret i32 %sub
}

; Function Attrs: nounwind readnone
define internal fastcc i32 @_ZL26safe_add_func_uint32_t_u_ujj(i32 %ui1, i32 %ui2) #3 {
entry:
  %add = add i32 %ui2, %ui1
  ret i32 %add
}

; Function Attrs: nounwind
define weak i8* @malloc(i32 %bytes) #0 {
entry:
  %cmp = icmp ult i32 %bytes, 245
  br i1 %cmp, label %if.then, label %if.else137

if.then:                                          ; preds = %entry
  %cmp1 = icmp ult i32 %bytes, 11
  br i1 %cmp1, label %cond.end, label %cond.false

cond.false:                                       ; preds = %if.then
  %add2 = add i32 %bytes, 11
  %and = and i32 %add2, -8
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %if.then
  %cond = phi i32 [ %and, %cond.false ], [ 16, %if.then ]
  %shr = lshr exact i32 %cond, 3
  %0 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  %shr3 = lshr i32 %0, %shr
  %and4 = and i32 %shr3, 3
  %cmp5 = icmp eq i32 %and4, 0
  br i1 %cmp5, label %if.else28, label %if.then6

if.then6:                                         ; preds = %cond.end
  %neg = and i32 %shr3, 1
  %and7 = xor i32 %neg, 1
  %add8 = add i32 %and7, %shr
  %shl = shl nsw i32 %add8, 1
  %arrayidx = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %shl
  %1 = bitcast %struct.malloc_chunk** %arrayidx to %struct.malloc_chunk*
  %arrayidx.sum = add i32 %shl, 2
  %2 = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %arrayidx.sum
  %3 = load %struct.malloc_chunk*, %struct.malloc_chunk** %2, align 4
  %fd9 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %3, i32 0, i32 2
  %4 = load %struct.malloc_chunk*, %struct.malloc_chunk** %fd9, align 4
  %cmp10 = icmp eq %struct.malloc_chunk* %1, %4
  br i1 %cmp10, label %if.then11, label %if.else

if.then11:                                        ; preds = %if.then6
  %shl12 = shl i32 1, %add8
  %neg13 = xor i32 %shl12, -1
  %and14 = and i32 %0, %neg13
  store i32 %and14, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  br label %if.end21

if.else:                                          ; preds = %if.then6
  %5 = bitcast %struct.malloc_chunk* %4 to i8*
  %6 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp15 = icmp ult i8* %5, %6
  br i1 %cmp15, label %if.else20, label %land.rhs

land.rhs:                                         ; preds = %if.else
  %bk = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %4, i32 0, i32 3
  %7 = load %struct.malloc_chunk*, %struct.malloc_chunk** %bk, align 4
  %cmp16 = icmp eq %struct.malloc_chunk* %7, %3
  br i1 %cmp16, label %if.then17, label %if.else20

if.then17:                                        ; preds = %land.rhs
  store %struct.malloc_chunk* %1, %struct.malloc_chunk** %bk, align 4
  store %struct.malloc_chunk* %4, %struct.malloc_chunk** %2, align 4
  br label %if.end21

if.else20:                                        ; preds = %land.rhs, %if.else
  tail call void @abort() #6
  unreachable

if.end21:                                         ; preds = %if.then17, %if.then11
  %shl22 = shl i32 %add8, 3
  %or23 = or i32 %shl22, 3
  %head = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %3, i32 0, i32 1
  store i32 %or23, i32* %head, align 4
  %8 = bitcast %struct.malloc_chunk* %3 to i8*
  %add.ptr.sum104 = or i32 %shl22, 4
  %head25 = getelementptr inbounds i8, i8* %8, i32 %add.ptr.sum104
  %9 = bitcast i8* %head25 to i32*
  %10 = load i32, i32* %9, align 4
  %or26 = or i32 %10, 1
  store i32 %or26, i32* %9, align 4
  %11 = bitcast %struct.malloc_chunk** %fd9 to i8*
  br label %postaction

if.else28:                                        ; preds = %cond.end
  %12 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 2), align 4
  %cmp29 = icmp ugt i32 %cond, %12
  br i1 %cmp29, label %if.then30, label %if.end154

if.then30:                                        ; preds = %if.else28
  %cmp31 = icmp eq i32 %shr3, 0
  br i1 %cmp31, label %if.else127, label %if.then32

if.then32:                                        ; preds = %if.then30
  %shl35 = shl i32 %shr3, %shr
  %shl37 = shl i32 2, %shr
  %sub = sub i32 0, %shl37
  %or40 = or i32 %shl37, %sub
  %and41 = and i32 %shl35, %or40
  %sub42 = sub i32 0, %and41
  %and43 = and i32 %and41, %sub42
  %sub44 = add i32 %and43, -1
  %shr45 = lshr i32 %sub44, 12
  %and46 = and i32 %shr45, 16
  %shr47 = lshr i32 %sub44, %and46
  %shr48 = lshr i32 %shr47, 5
  %and49 = and i32 %shr48, 8
  %add50 = or i32 %and49, %and46
  %shr51 = lshr i32 %shr47, %and49
  %shr52 = lshr i32 %shr51, 2
  %and53 = and i32 %shr52, 4
  %add54 = or i32 %add50, %and53
  %shr55 = lshr i32 %shr51, %and53
  %shr56 = lshr i32 %shr55, 1
  %and57 = and i32 %shr56, 2
  %add58 = or i32 %add54, %and57
  %shr59 = lshr i32 %shr55, %and57
  %shr60 = lshr i32 %shr59, 1
  %and61 = and i32 %shr60, 1
  %add62 = or i32 %add58, %and61
  %shr63 = lshr i32 %shr59, %and61
  %add64 = add i32 %add62, %shr63
  %shl65 = shl i32 %add64, 1
  %arrayidx66 = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %shl65
  %13 = bitcast %struct.malloc_chunk** %arrayidx66 to %struct.malloc_chunk*
  %arrayidx66.sum = add i32 %shl65, 2
  %14 = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %arrayidx66.sum
  %15 = load %struct.malloc_chunk*, %struct.malloc_chunk** %14, align 4
  %fd69 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %15, i32 0, i32 2
  %16 = load %struct.malloc_chunk*, %struct.malloc_chunk** %fd69, align 4
  %cmp70 = icmp eq %struct.malloc_chunk* %13, %16
  br i1 %cmp70, label %if.then71, label %if.else75

if.then71:                                        ; preds = %if.then32
  %shl72 = shl i32 1, %add64
  %neg73 = xor i32 %shl72, -1
  %and74 = and i32 %0, %neg73
  store i32 %and74, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  br label %if.end89

if.else75:                                        ; preds = %if.then32
  %17 = bitcast %struct.malloc_chunk* %16 to i8*
  %18 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp76 = icmp ult i8* %17, %18
  br i1 %cmp76, label %if.else87, label %land.rhs77

land.rhs77:                                       ; preds = %if.else75
  %bk78 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %16, i32 0, i32 3
  %19 = load %struct.malloc_chunk*, %struct.malloc_chunk** %bk78, align 4
  %cmp79 = icmp eq %struct.malloc_chunk* %19, %15
  br i1 %cmp79, label %if.then84, label %if.else87

if.then84:                                        ; preds = %land.rhs77
  store %struct.malloc_chunk* %13, %struct.malloc_chunk** %bk78, align 4
  store %struct.malloc_chunk* %16, %struct.malloc_chunk** %14, align 4
  br label %if.end89

if.else87:                                        ; preds = %land.rhs77, %if.else75
  tail call void @abort() #6
  unreachable

if.end89:                                         ; preds = %if.then84, %if.then71
  %shl90 = shl i32 %add64, 3
  %sub91 = sub i32 %shl90, %cond
  %or93 = or i32 %cond, 3
  %head94 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %15, i32 0, i32 1
  store i32 %or93, i32* %head94, align 4
  %20 = bitcast %struct.malloc_chunk* %15 to i8*
  %add.ptr95 = getelementptr inbounds i8, i8* %20, i32 %cond
  %21 = bitcast i8* %add.ptr95 to %struct.malloc_chunk*
  %or96 = or i32 %sub91, 1
  %add.ptr95.sum102 = or i32 %cond, 4
  %head97 = getelementptr inbounds i8, i8* %20, i32 %add.ptr95.sum102
  %22 = bitcast i8* %head97 to i32*
  store i32 %or96, i32* %22, align 4
  %add.ptr98 = getelementptr inbounds i8, i8* %20, i32 %shl90
  %prev_foot = bitcast i8* %add.ptr98 to i32*
  store i32 %sub91, i32* %prev_foot, align 4
  %23 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 2), align 4
  %cmp99 = icmp eq i32 %23, 0
  br i1 %cmp99, label %if.end125, label %if.then100

if.then100:                                       ; preds = %if.end89
  %24 = load %struct.malloc_chunk*, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 5), align 4
  %shr101 = lshr i32 %23, 3
  %shl102 = shl nuw nsw i32 %shr101, 1
  %arrayidx103 = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %shl102
  %25 = bitcast %struct.malloc_chunk** %arrayidx103 to %struct.malloc_chunk*
  %26 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  %shl105 = shl i32 1, %shr101
  %and106 = and i32 %26, %shl105
  %tobool107 = icmp eq i32 %and106, 0
  br i1 %tobool107, label %if.then108, label %if.else111

if.then108:                                       ; preds = %if.then100
  %or110 = or i32 %26, %shl105
  store i32 %or110, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  %arrayidx103.sum.pre = add i32 %shl102, 2
  %.pre = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %arrayidx103.sum.pre
  br label %if.end120

if.else111:                                       ; preds = %if.then100
  %arrayidx103.sum103 = add i32 %shl102, 2
  %27 = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %arrayidx103.sum103
  %28 = load %struct.malloc_chunk*, %struct.malloc_chunk** %27, align 4
  %29 = bitcast %struct.malloc_chunk* %28 to i8*
  %30 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp113 = icmp ult i8* %29, %30
  br i1 %cmp113, label %if.else118, label %if.end120

if.else118:                                       ; preds = %if.else111
  tail call void @abort() #6
  unreachable

if.end120:                                        ; preds = %if.else111, %if.then108
  %.pre-phi = phi %struct.malloc_chunk** [ %27, %if.else111 ], [ %.pre, %if.then108 ]
  %F104.0 = phi %struct.malloc_chunk* [ %28, %if.else111 ], [ %25, %if.then108 ]
  store %struct.malloc_chunk* %24, %struct.malloc_chunk** %.pre-phi, align 4
  %bk122 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %F104.0, i32 0, i32 3
  store %struct.malloc_chunk* %24, %struct.malloc_chunk** %bk122, align 4
  %fd123 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %24, i32 0, i32 2
  store %struct.malloc_chunk* %F104.0, %struct.malloc_chunk** %fd123, align 4
  %bk124 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %24, i32 0, i32 3
  store %struct.malloc_chunk* %25, %struct.malloc_chunk** %bk124, align 4
  br label %if.end125

if.end125:                                        ; preds = %if.end120, %if.end89
  store i32 %sub91, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 2), align 4
  store %struct.malloc_chunk* %21, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 5), align 4
  %31 = bitcast %struct.malloc_chunk** %fd69 to i8*
  br label %postaction

if.else127:                                       ; preds = %if.then30
  %32 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 1), align 4
  %cmp128 = icmp eq i32 %32, 0
  br i1 %cmp128, label %if.end154, label %land.lhs.true

land.lhs.true:                                    ; preds = %if.else127
  %sub.i = sub i32 0, %32
  %and.i = and i32 %32, %sub.i
  %sub2.i = add i32 %and.i, -1
  %shr.i = lshr i32 %sub2.i, 12
  %and3.i = and i32 %shr.i, 16
  %shr4.i = lshr i32 %sub2.i, %and3.i
  %shr5.i = lshr i32 %shr4.i, 5
  %and6.i = and i32 %shr5.i, 8
  %add.i = or i32 %and6.i, %and3.i
  %shr7.i = lshr i32 %shr4.i, %and6.i
  %shr8.i = lshr i32 %shr7.i, 2
  %and9.i = and i32 %shr8.i, 4
  %add10.i = or i32 %add.i, %and9.i
  %shr11.i = lshr i32 %shr7.i, %and9.i
  %shr12.i = lshr i32 %shr11.i, 1
  %and13.i = and i32 %shr12.i, 2
  %add14.i = or i32 %add10.i, %and13.i
  %shr15.i = lshr i32 %shr11.i, %and13.i
  %shr16.i = lshr i32 %shr15.i, 1
  %and17.i = and i32 %shr16.i, 1
  %add18.i = or i32 %add14.i, %and17.i
  %shr19.i = lshr i32 %shr15.i, %and17.i
  %add20.i = add i32 %add18.i, %shr19.i
  %arrayidx.i = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 11, i32 %add20.i
  %33 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx.i, align 4
  %head.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %33, i32 0, i32 1
  %34 = load i32, i32* %head.i, align 4
  %and21.i = and i32 %34, -8
  %sub22.i = sub i32 %and21.i, %cond
  br label %while.cond.i

while.cond.i:                                     ; preds = %while.body.i, %land.lhs.true
  %rsize.0.i = phi i32 [ %sub22.i, %land.lhs.true ], [ %sub31.rsize.0.i, %while.body.i ]
  %v.0.i = phi %struct.malloc_tree_chunk* [ %33, %land.lhs.true ], [ %cond.v.0.i, %while.body.i ]
  %t.0.i = phi %struct.malloc_tree_chunk* [ %33, %land.lhs.true ], [ %cond6.i, %while.body.i ]
  %arrayidx23.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %t.0.i, i32 0, i32 4, i32 0
  %35 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx23.i, align 4
  %cmp.i = icmp eq %struct.malloc_tree_chunk* %35, null
  br i1 %cmp.i, label %cond.end.i, label %while.body.i

cond.end.i:                                       ; preds = %while.cond.i
  %arrayidx27.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %t.0.i, i32 0, i32 4, i32 1
  %36 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx27.i, align 4
  %cmp28.i = icmp eq %struct.malloc_tree_chunk* %36, null
  br i1 %cmp28.i, label %while.end.i, label %while.body.i

while.body.i:                                     ; preds = %cond.end.i, %while.cond.i
  %cond6.i = phi %struct.malloc_tree_chunk* [ %36, %cond.end.i ], [ %35, %while.cond.i ]
  %head29.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %cond6.i, i32 0, i32 1
  %37 = load i32, i32* %head29.i, align 4
  %and30.i = and i32 %37, -8
  %sub31.i = sub i32 %and30.i, %cond
  %cmp32.i = icmp ult i32 %sub31.i, %rsize.0.i
  %sub31.rsize.0.i = select i1 %cmp32.i, i32 %sub31.i, i32 %rsize.0.i
  %cond.v.0.i = select i1 %cmp32.i, %struct.malloc_tree_chunk* %cond6.i, %struct.malloc_tree_chunk* %v.0.i
  br label %while.cond.i

while.end.i:                                      ; preds = %cond.end.i
  %38 = bitcast %struct.malloc_tree_chunk* %v.0.i to i8*
  %39 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp33.i = icmp ult i8* %38, %39
  br i1 %cmp33.i, label %if.end227.i, label %if.then34.i

if.then34.i:                                      ; preds = %while.end.i
  %add.ptr.i = getelementptr inbounds i8, i8* %38, i32 %cond
  %40 = bitcast i8* %add.ptr.i to %struct.malloc_chunk*
  %cmp35.i = icmp ult i8* %38, %add.ptr.i
  br i1 %cmp35.i, label %if.then39.i, label %if.end227.i

if.then39.i:                                      ; preds = %if.then34.i
  %parent.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %v.0.i, i32 0, i32 5
  %41 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %parent.i, align 4
  %bk.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %v.0.i, i32 0, i32 3
  %42 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %bk.i, align 4
  %cmp40.i = icmp eq %struct.malloc_tree_chunk* %42, %v.0.i
  br i1 %cmp40.i, label %if.else59.i, label %if.then42.i

if.then42.i:                                      ; preds = %if.then39.i
  %fd.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %v.0.i, i32 0, i32 2
  %43 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %fd.i, align 4
  %44 = bitcast %struct.malloc_tree_chunk* %43 to i8*
  %cmp45.i = icmp ult i8* %44, %39
  br i1 %cmp45.i, label %if.else.i, label %land.lhs.true.i

land.lhs.true.i:                                  ; preds = %if.then42.i
  %bk47.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %43, i32 0, i32 3
  %45 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %bk47.i, align 4
  %cmp48.i = icmp eq %struct.malloc_tree_chunk* %45, %v.0.i
  br i1 %cmp48.i, label %land.rhs.i, label %if.else.i

land.rhs.i:                                       ; preds = %land.lhs.true.i
  %fd50.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %42, i32 0, i32 2
  %46 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %fd50.i, align 4
  %cmp51.i = icmp eq %struct.malloc_tree_chunk* %46, %v.0.i
  br i1 %cmp51.i, label %if.then55.i, label %if.else.i

if.then55.i:                                      ; preds = %land.rhs.i
  store %struct.malloc_tree_chunk* %42, %struct.malloc_tree_chunk** %bk47.i, align 4
  store %struct.malloc_tree_chunk* %43, %struct.malloc_tree_chunk** %fd50.i, align 4
  br label %if.end89.i

if.else.i:                                        ; preds = %land.rhs.i, %land.lhs.true.i, %if.then42.i
  tail call void @abort() #6
  unreachable

if.else59.i:                                      ; preds = %if.then39.i
  %arrayidx61.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %v.0.i, i32 0, i32 4, i32 1
  %47 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx61.i, align 4
  %cmp62.i = icmp eq %struct.malloc_tree_chunk* %47, null
  br i1 %cmp62.i, label %lor.lhs.false.i, label %while.cond69.i

lor.lhs.false.i:                                  ; preds = %if.else59.i
  %arrayidx65.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %v.0.i, i32 0, i32 4, i32 0
  %48 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx65.i, align 4
  %cmp66.i = icmp eq %struct.malloc_tree_chunk* %48, null
  br i1 %cmp66.i, label %if.end89.i, label %while.cond69.i

while.cond69.i:                                   ; preds = %lor.rhs.i, %while.cond69.i, %lor.lhs.false.i, %if.else59.i
  %RP.0.i = phi %struct.malloc_tree_chunk** [ %arrayidx65.i, %lor.lhs.false.i ], [ %arrayidx61.i, %if.else59.i ], [ %arrayidx71.i, %while.cond69.i ], [ %arrayidx75.i, %lor.rhs.i ]
  %R.0.i = phi %struct.malloc_tree_chunk* [ %48, %lor.lhs.false.i ], [ %47, %if.else59.i ], [ %49, %while.cond69.i ], [ %50, %lor.rhs.i ]
  %arrayidx71.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.0.i, i32 0, i32 4, i32 1
  %49 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx71.i, align 4
  %cmp72.i = icmp eq %struct.malloc_tree_chunk* %49, null
  br i1 %cmp72.i, label %lor.rhs.i, label %while.cond69.i

lor.rhs.i:                                        ; preds = %while.cond69.i
  %arrayidx75.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.0.i, i32 0, i32 4, i32 0
  %50 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx75.i, align 4
  %cmp76.i = icmp eq %struct.malloc_tree_chunk* %50, null
  br i1 %cmp76.i, label %while.end79.i, label %while.cond69.i

while.end79.i:                                    ; preds = %lor.rhs.i
  %51 = bitcast %struct.malloc_tree_chunk** %RP.0.i to i8*
  %cmp81.i = icmp ult i8* %51, %39
  br i1 %cmp81.i, label %if.else86.i, label %if.then85.i

if.then85.i:                                      ; preds = %while.end79.i
  store %struct.malloc_tree_chunk* null, %struct.malloc_tree_chunk** %RP.0.i, align 4
  br label %if.end89.i

if.else86.i:                                      ; preds = %while.end79.i
  tail call void @abort() #6
  unreachable

if.end89.i:                                       ; preds = %if.then85.i, %lor.lhs.false.i, %if.then55.i
  %R.1.i = phi %struct.malloc_tree_chunk* [ %42, %if.then55.i ], [ %R.0.i, %if.then85.i ], [ null, %lor.lhs.false.i ]
  %cmp90.i = icmp eq %struct.malloc_tree_chunk* %41, null
  br i1 %cmp90.i, label %if.end173.i, label %if.then92.i

if.then92.i:                                      ; preds = %if.end89.i
  %index.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %v.0.i, i32 0, i32 6
  %52 = load i32, i32* %index.i, align 4
  %arrayidx94.i = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 11, i32 %52
  %53 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx94.i, align 4
  %cmp95.i = icmp eq %struct.malloc_tree_chunk* %v.0.i, %53
  br i1 %cmp95.i, label %if.then97.i, label %if.else105.i

if.then97.i:                                      ; preds = %if.then92.i
  store %struct.malloc_tree_chunk* %R.1.i, %struct.malloc_tree_chunk** %arrayidx94.i, align 4
  %cond4.i = icmp eq %struct.malloc_tree_chunk* %R.1.i, null
  br i1 %cond4.i, label %if.end125.thread.i, label %if.then128.i

if.end125.thread.i:                               ; preds = %if.then97.i
  %shl.i = shl i32 1, %52
  %neg.i = xor i32 %shl.i, -1
  %54 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 1), align 4
  %and103.i = and i32 %54, %neg.i
  store i32 %and103.i, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 1), align 4
  br label %if.end173.i

if.else105.i:                                     ; preds = %if.then92.i
  %55 = bitcast %struct.malloc_tree_chunk* %41 to i8*
  %56 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp107.i = icmp ult i8* %55, %56
  br i1 %cmp107.i, label %if.else123.i, label %if.then111.i

if.then111.i:                                     ; preds = %if.else105.i
  %arrayidx113.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %41, i32 0, i32 4, i32 0
  %57 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx113.i, align 4
  %cmp114.i = icmp eq %struct.malloc_tree_chunk* %57, %v.0.i
  br i1 %cmp114.i, label %if.then116.i, label %if.else119.i

if.then116.i:                                     ; preds = %if.then111.i
  store %struct.malloc_tree_chunk* %R.1.i, %struct.malloc_tree_chunk** %arrayidx113.i, align 4
  br label %if.end125.i

if.else119.i:                                     ; preds = %if.then111.i
  %arrayidx121.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %41, i32 0, i32 4, i32 1
  store %struct.malloc_tree_chunk* %R.1.i, %struct.malloc_tree_chunk** %arrayidx121.i, align 4
  br label %if.end125.i

if.else123.i:                                     ; preds = %if.else105.i
  tail call void @abort() #6
  unreachable

if.end125.i:                                      ; preds = %if.else119.i, %if.then116.i
  %cmp126.i = icmp eq %struct.malloc_tree_chunk* %R.1.i, null
  br i1 %cmp126.i, label %if.end173.i, label %if.then128.i

if.then128.i:                                     ; preds = %if.end125.i, %if.then97.i
  %58 = bitcast %struct.malloc_tree_chunk* %R.1.i to i8*
  %59 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp130.i = icmp ult i8* %58, %59
  br i1 %cmp130.i, label %if.else170.i, label %if.then134.i

if.then134.i:                                     ; preds = %if.then128.i
  %parent135.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.1.i, i32 0, i32 5
  store %struct.malloc_tree_chunk* %41, %struct.malloc_tree_chunk** %parent135.i, align 4
  %arrayidx137.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %v.0.i, i32 0, i32 4, i32 0
  %60 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx137.i, align 4
  %cmp138.i = icmp eq %struct.malloc_tree_chunk* %60, null
  br i1 %cmp138.i, label %if.end152.i, label %if.then140.i

if.then140.i:                                     ; preds = %if.then134.i
  %61 = bitcast %struct.malloc_tree_chunk* %60 to i8*
  %62 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp142.i = icmp ult i8* %61, %62
  br i1 %cmp142.i, label %if.else150.i, label %if.then146.i

if.then146.i:                                     ; preds = %if.then140.i
  %arrayidx148.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.1.i, i32 0, i32 4, i32 0
  store %struct.malloc_tree_chunk* %60, %struct.malloc_tree_chunk** %arrayidx148.i, align 4
  %parent149.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %60, i32 0, i32 5
  store %struct.malloc_tree_chunk* %R.1.i, %struct.malloc_tree_chunk** %parent149.i, align 4
  br label %if.end152.i

if.else150.i:                                     ; preds = %if.then140.i
  tail call void @abort() #6
  unreachable

if.end152.i:                                      ; preds = %if.then146.i, %if.then134.i
  %arrayidx154.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %v.0.i, i32 0, i32 4, i32 1
  %63 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx154.i, align 4
  %cmp155.i = icmp eq %struct.malloc_tree_chunk* %63, null
  br i1 %cmp155.i, label %if.end173.i, label %if.then157.i

if.then157.i:                                     ; preds = %if.end152.i
  %64 = bitcast %struct.malloc_tree_chunk* %63 to i8*
  %65 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp159.i = icmp ult i8* %64, %65
  br i1 %cmp159.i, label %if.else167.i, label %if.then163.i

if.then163.i:                                     ; preds = %if.then157.i
  %arrayidx165.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.1.i, i32 0, i32 4, i32 1
  store %struct.malloc_tree_chunk* %63, %struct.malloc_tree_chunk** %arrayidx165.i, align 4
  %parent166.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %63, i32 0, i32 5
  store %struct.malloc_tree_chunk* %R.1.i, %struct.malloc_tree_chunk** %parent166.i, align 4
  br label %if.end173.i

if.else167.i:                                     ; preds = %if.then157.i
  tail call void @abort() #6
  unreachable

if.else170.i:                                     ; preds = %if.then128.i
  tail call void @abort() #6
  unreachable

if.end173.i:                                      ; preds = %if.then163.i, %if.end152.i, %if.end125.i, %if.end125.thread.i, %if.end89.i
  %cmp174.i = icmp ult i32 %rsize.0.i, 16
  br i1 %cmp174.i, label %if.then176.i, label %if.else184.i

if.then176.i:                                     ; preds = %if.end173.i
  %add177.i = add i32 %rsize.0.i, %cond
  %or178.i = or i32 %add177.i, 3
  %head179.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %v.0.i, i32 0, i32 1
  store i32 %or178.i, i32* %head179.i, align 4
  %add.ptr181.sum.i = add i32 %add177.i, 4
  %head182.i = getelementptr inbounds i8, i8* %38, i32 %add.ptr181.sum.i
  %66 = bitcast i8* %head182.i to i32*
  %67 = load i32, i32* %66, align 4
  %or183.i = or i32 %67, 1
  store i32 %or183.i, i32* %66, align 4
  br label %tmalloc_small.exit

if.else184.i:                                     ; preds = %if.end173.i
  %or186.i = or i32 %cond, 3
  %head187.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %v.0.i, i32 0, i32 1
  store i32 %or186.i, i32* %head187.i, align 4
  %or188.i = or i32 %rsize.0.i, 1
  %add.ptr.sum.i173 = or i32 %cond, 4
  %head189.i = getelementptr inbounds i8, i8* %38, i32 %add.ptr.sum.i173
  %68 = bitcast i8* %head189.i to i32*
  store i32 %or188.i, i32* %68, align 4
  %add.ptr.sum1.i = add i32 %rsize.0.i, %cond
  %add.ptr190.i = getelementptr inbounds i8, i8* %38, i32 %add.ptr.sum1.i
  %prev_foot.i = bitcast i8* %add.ptr190.i to i32*
  store i32 %rsize.0.i, i32* %prev_foot.i, align 4
  %69 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 2), align 4
  %cmp191.i = icmp eq i32 %69, 0
  br i1 %cmp191.i, label %if.end221.i, label %if.then193.i

if.then193.i:                                     ; preds = %if.else184.i
  %70 = load %struct.malloc_chunk*, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 5), align 4
  %shr194.i = lshr i32 %69, 3
  %shl195.i = shl nuw nsw i32 %shr194.i, 1
  %arrayidx196.i = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %shl195.i
  %71 = bitcast %struct.malloc_chunk** %arrayidx196.i to %struct.malloc_chunk*
  %72 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  %shl198.i = shl i32 1, %shr194.i
  %and199.i = and i32 %72, %shl198.i
  %tobool200.i = icmp eq i32 %and199.i, 0
  br i1 %tobool200.i, label %if.then201.i, label %if.else205.i

if.then201.i:                                     ; preds = %if.then193.i
  %or204.i = or i32 %72, %shl198.i
  store i32 %or204.i, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  %arrayidx196.sum.pre.i = add i32 %shl195.i, 2
  %.pre.i = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %arrayidx196.sum.pre.i
  br label %if.end216.i

if.else205.i:                                     ; preds = %if.then193.i
  %arrayidx196.sum2.i = add i32 %shl195.i, 2
  %73 = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %arrayidx196.sum2.i
  %74 = load %struct.malloc_chunk*, %struct.malloc_chunk** %73, align 4
  %75 = bitcast %struct.malloc_chunk* %74 to i8*
  %76 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp208.i = icmp ult i8* %75, %76
  br i1 %cmp208.i, label %if.else214.i, label %if.end216.i

if.else214.i:                                     ; preds = %if.else205.i
  tail call void @abort() #6
  unreachable

if.end216.i:                                      ; preds = %if.else205.i, %if.then201.i
  %.pre-phi.i = phi %struct.malloc_chunk** [ %73, %if.else205.i ], [ %.pre.i, %if.then201.i ]
  %F197.0.i = phi %struct.malloc_chunk* [ %74, %if.else205.i ], [ %71, %if.then201.i ]
  store %struct.malloc_chunk* %70, %struct.malloc_chunk** %.pre-phi.i, align 4
  %bk218.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %F197.0.i, i32 0, i32 3
  store %struct.malloc_chunk* %70, %struct.malloc_chunk** %bk218.i, align 4
  %fd219.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %70, i32 0, i32 2
  store %struct.malloc_chunk* %F197.0.i, %struct.malloc_chunk** %fd219.i, align 4
  %bk220.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %70, i32 0, i32 3
  store %struct.malloc_chunk* %71, %struct.malloc_chunk** %bk220.i, align 4
  br label %if.end221.i

if.end221.i:                                      ; preds = %if.end216.i, %if.else184.i
  store i32 %rsize.0.i, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 2), align 4
  store %struct.malloc_chunk* %40, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 5), align 4
  br label %tmalloc_small.exit

if.end227.i:                                      ; preds = %if.then34.i, %while.end.i
  tail call void @abort() #6
  unreachable

tmalloc_small.exit:                               ; preds = %if.end221.i, %if.then176.i
  %add.ptr225.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %v.0.i, i32 0, i32 2
  %77 = bitcast %struct.malloc_tree_chunk** %add.ptr225.i to i8*
  br label %postaction

if.else137:                                       ; preds = %entry
  %cmp138 = icmp ugt i32 %bytes, -65
  br i1 %cmp138, label %if.end154, label %if.else141

if.else141:                                       ; preds = %if.else137
  %add143 = add i32 %bytes, 11
  %and144 = and i32 %add143, -8
  %78 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 1), align 4
  %cmp145 = icmp eq i32 %78, 0
  br i1 %cmp145, label %if.end154, label %land.lhs.true147

land.lhs.true147:                                 ; preds = %if.else141
  %sub.i105 = sub i32 0, %and144
  %shr.i106 = lshr i32 %add143, 8
  %cmp.i107 = icmp eq i32 %shr.i106, 0
  br i1 %cmp.i107, label %if.end23.i, label %if.else.i108

if.else.i108:                                     ; preds = %land.lhs.true147
  %cmp1.i = icmp ugt i32 %and144, 16777215
  br i1 %cmp1.i, label %if.end23.i, label %if.else3.i

if.else3.i:                                       ; preds = %if.else.i108
  %sub4.i = add i32 %shr.i106, 1048320
  %shr5.i109 = lshr i32 %sub4.i, 16
  %and.i110 = and i32 %shr5.i109, 8
  %shl.i111 = shl i32 %shr.i106, %and.i110
  %sub6.i = add i32 %shl.i111, 520192
  %shr7.i112 = lshr i32 %sub6.i, 16
  %and8.i = and i32 %shr7.i112, 4
  %add.i113 = or i32 %and8.i, %and.i110
  %shl9.i = shl i32 %shl.i111, %and8.i
  %sub10.i = add i32 %shl9.i, 245760
  %shr11.i114 = lshr i32 %sub10.i, 16
  %and12.i = and i32 %shr11.i114, 2
  %add13.i = or i32 %add.i113, %and12.i
  %sub14.i = sub i32 14, %add13.i
  %shl15.i = shl i32 %shl9.i, %and12.i
  %shr16.i115 = lshr i32 %shl15.i, 15
  %add17.i = add i32 %sub14.i, %shr16.i115
  %shl18.i = shl nsw i32 %add17.i, 1
  %add19.i = add i32 %add17.i, 7
  %shr20.i = lshr i32 %and144, %add19.i
  %and21.i116 = and i32 %shr20.i, 1
  %add22.i = or i32 %and21.i116, %shl18.i
  br label %if.end23.i

if.end23.i:                                       ; preds = %if.else3.i, %if.else.i108, %land.lhs.true147
  %idx.0.i = phi i32 [ %add22.i, %if.else3.i ], [ 0, %land.lhs.true147 ], [ 31, %if.else.i108 ]
  %arrayidx.i117 = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 11, i32 %idx.0.i
  %79 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx.i117, align 4
  %cmp24.i = icmp eq %struct.malloc_tree_chunk* %79, null
  br i1 %cmp24.i, label %if.end53.i, label %if.then25.i

if.then25.i:                                      ; preds = %if.end23.i
  %cmp26.i = icmp eq i32 %idx.0.i, 31
  br i1 %cmp26.i, label %cond.end.i118, label %cond.false.i

cond.false.i:                                     ; preds = %if.then25.i
  %shr27.i = lshr i32 %idx.0.i, 1
  %sub30.i = sub i32 25, %shr27.i
  br label %cond.end.i118

cond.end.i118:                                    ; preds = %cond.false.i, %if.then25.i
  %cond.i = phi i32 [ %sub30.i, %cond.false.i ], [ 0, %if.then25.i ]
  %shl31.i = shl i32 %and144, %cond.i
  br label %for.cond.i

for.cond.i:                                       ; preds = %if.end39.i, %cond.end.i118
  %rst.0.i = phi %struct.malloc_tree_chunk* [ null, %cond.end.i118 ], [ %rst.1.i, %if.end39.i ]
  %sizebits.0.i = phi i32 [ %shl31.i, %cond.end.i118 ], [ %shl52.i, %if.end39.i ]
  %t.0.i119 = phi %struct.malloc_tree_chunk* [ %79, %cond.end.i118 ], [ %82, %if.end39.i ]
  %rsize.0.i120 = phi i32 [ %sub.i105, %cond.end.i118 ], [ %rsize.1.i, %if.end39.i ]
  %v.0.i121 = phi %struct.malloc_tree_chunk* [ null, %cond.end.i118 ], [ %v.1.i, %if.end39.i ]
  %head.i122 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %t.0.i119, i32 0, i32 1
  %80 = load i32, i32* %head.i122, align 4
  %and32.i = and i32 %80, -8
  %sub33.i = sub i32 %and32.i, %and144
  %cmp34.i = icmp ult i32 %sub33.i, %rsize.0.i120
  br i1 %cmp34.i, label %if.then35.i, label %if.end39.i

if.then35.i:                                      ; preds = %for.cond.i
  %cmp36.i = icmp eq i32 %and32.i, %and144
  br i1 %cmp36.i, label %if.end53.i, label %if.end39.i

if.end39.i:                                       ; preds = %if.then35.i, %for.cond.i
  %rsize.1.i = phi i32 [ %sub33.i, %if.then35.i ], [ %rsize.0.i120, %for.cond.i ]
  %v.1.i = phi %struct.malloc_tree_chunk* [ %t.0.i119, %if.then35.i ], [ %v.0.i121, %for.cond.i ]
  %arrayidx40.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %t.0.i119, i32 0, i32 4, i32 1
  %81 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx40.i, align 4
  %shr41.i = lshr i32 %sizebits.0.i, 31
  %arrayidx44.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %t.0.i119, i32 0, i32 4, i32 %shr41.i
  %82 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx44.i, align 4
  %cmp45.i123 = icmp eq %struct.malloc_tree_chunk* %81, null
  %cmp46.i = icmp eq %struct.malloc_tree_chunk* %81, %82
  %or.cond.i = or i1 %cmp45.i123, %cmp46.i
  %rst.1.i = select i1 %or.cond.i, %struct.malloc_tree_chunk* %rst.0.i, %struct.malloc_tree_chunk* %81
  %cmp49.i = icmp eq %struct.malloc_tree_chunk* %82, null
  %shl52.i = shl i32 %sizebits.0.i, 1
  br i1 %cmp49.i, label %if.end53.i, label %for.cond.i

if.end53.i:                                       ; preds = %if.end39.i, %if.then35.i, %if.end23.i
  %t.1.i = phi %struct.malloc_tree_chunk* [ null, %if.end23.i ], [ %rst.1.i, %if.end39.i ], [ %t.0.i119, %if.then35.i ]
  %rsize.2.i = phi i32 [ %sub.i105, %if.end23.i ], [ %rsize.1.i, %if.end39.i ], [ %sub33.i, %if.then35.i ]
  %v.2.i = phi %struct.malloc_tree_chunk* [ null, %if.end23.i ], [ %v.1.i, %if.end39.i ], [ %t.0.i119, %if.then35.i ]
  %cmp54.i = icmp eq %struct.malloc_tree_chunk* %t.1.i, null
  %cmp56.i = icmp eq %struct.malloc_tree_chunk* %v.2.i, null
  %or.cond16.i = and i1 %cmp54.i, %cmp56.i
  br i1 %or.cond16.i, label %if.then57.i, label %while.cond.preheader.i

if.then57.i:                                      ; preds = %if.end53.i
  %shl59.i = shl i32 2, %idx.0.i
  %sub62.i = sub i32 0, %shl59.i
  %or.i = or i32 %shl59.i, %sub62.i
  %and63.i = and i32 %78, %or.i
  %cmp64.i = icmp eq i32 %and63.i, 0
  br i1 %cmp64.i, label %if.end154, label %if.then65.i

if.then65.i:                                      ; preds = %if.then57.i
  %sub66.i = sub i32 0, %and63.i
  %and67.i = and i32 %and63.i, %sub66.i
  %sub69.i = add i32 %and67.i, -1
  %shr71.i = lshr i32 %sub69.i, 12
  %and72.i = and i32 %shr71.i, 16
  %shr74.i = lshr i32 %sub69.i, %and72.i
  %shr75.i = lshr i32 %shr74.i, 5
  %and76.i = and i32 %shr75.i, 8
  %add77.i = or i32 %and76.i, %and72.i
  %shr78.i = lshr i32 %shr74.i, %and76.i
  %shr79.i = lshr i32 %shr78.i, 2
  %and80.i = and i32 %shr79.i, 4
  %add81.i = or i32 %add77.i, %and80.i
  %shr82.i = lshr i32 %shr78.i, %and80.i
  %shr83.i = lshr i32 %shr82.i, 1
  %and84.i = and i32 %shr83.i, 2
  %add85.i = or i32 %add81.i, %and84.i
  %shr86.i = lshr i32 %shr82.i, %and84.i
  %shr87.i = lshr i32 %shr86.i, 1
  %and88.i = and i32 %shr87.i, 1
  %add89.i = or i32 %add85.i, %and88.i
  %shr90.i = lshr i32 %shr86.i, %and88.i
  %add91.i = add i32 %add89.i, %shr90.i
  %arrayidx93.i = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 11, i32 %add91.i
  %83 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx93.i, align 4
  br label %while.cond.preheader.i

while.cond.preheader.i:                           ; preds = %if.then65.i, %if.end53.i
  %t.2.ph.i = phi %struct.malloc_tree_chunk* [ %t.1.i, %if.end53.i ], [ %83, %if.then65.i ]
  %cmp9626.i = icmp eq %struct.malloc_tree_chunk* %t.2.ph.i, null
  br i1 %cmp9626.i, label %while.end.i125, label %while.body.i124

while.body.i124:                                  ; preds = %while.cond.backedge.i, %while.body.i124, %while.cond.preheader.i
  %v.329.i = phi %struct.malloc_tree_chunk* [ %v.2.i, %while.cond.preheader.i ], [ %t.2.v.3.i, %while.body.i124 ], [ %t.2.v.3.i, %while.cond.backedge.i ]
  %rsize.328.i = phi i32 [ %rsize.2.i, %while.cond.preheader.i ], [ %sub100.rsize.3.i, %while.body.i124 ], [ %sub100.rsize.3.i, %while.cond.backedge.i ]
  %t.227.i = phi %struct.malloc_tree_chunk* [ %t.2.ph.i, %while.cond.preheader.i ], [ %85, %while.body.i124 ], [ %86, %while.cond.backedge.i ]
  %head98.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %t.227.i, i32 0, i32 1
  %84 = load i32, i32* %head98.i, align 4
  %and99.i = and i32 %84, -8
  %sub100.i = sub i32 %and99.i, %and144
  %cmp101.i = icmp ult i32 %sub100.i, %rsize.328.i
  %sub100.rsize.3.i = select i1 %cmp101.i, i32 %sub100.i, i32 %rsize.328.i
  %t.2.v.3.i = select i1 %cmp101.i, %struct.malloc_tree_chunk* %t.227.i, %struct.malloc_tree_chunk* %v.329.i
  %arrayidx105.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %t.227.i, i32 0, i32 4, i32 0
  %85 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx105.i, align 4
  %cmp106.i = icmp eq %struct.malloc_tree_chunk* %85, null
  br i1 %cmp106.i, label %while.cond.backedge.i, label %while.body.i124

while.cond.backedge.i:                            ; preds = %while.body.i124
  %arrayidx112.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %t.227.i, i32 0, i32 4, i32 1
  %86 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx112.i, align 4
  %cmp96.i = icmp eq %struct.malloc_tree_chunk* %86, null
  br i1 %cmp96.i, label %while.end.i125, label %while.body.i124

while.end.i125:                                   ; preds = %while.cond.backedge.i, %while.cond.preheader.i
  %v.3.lcssa.i = phi %struct.malloc_tree_chunk* [ %v.2.i, %while.cond.preheader.i ], [ %t.2.v.3.i, %while.cond.backedge.i ]
  %rsize.3.lcssa.i = phi i32 [ %rsize.2.i, %while.cond.preheader.i ], [ %sub100.rsize.3.i, %while.cond.backedge.i ]
  %cmp115.i = icmp eq %struct.malloc_tree_chunk* %v.3.lcssa.i, null
  br i1 %cmp115.i, label %if.end154, label %land.lhs.true116.i

land.lhs.true116.i:                               ; preds = %while.end.i125
  %87 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 2), align 4
  %sub117.i = sub i32 %87, %and144
  %cmp118.i = icmp ult i32 %rsize.3.lcssa.i, %sub117.i
  br i1 %cmp118.i, label %if.then119.i, label %if.end154

if.then119.i:                                     ; preds = %land.lhs.true116.i
  %88 = bitcast %struct.malloc_tree_chunk* %v.3.lcssa.i to i8*
  %89 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp120.i = icmp ult i8* %88, %89
  br i1 %cmp120.i, label %if.end438.i, label %if.then121.i

if.then121.i:                                     ; preds = %if.then119.i
  %add.ptr.i126 = getelementptr inbounds i8, i8* %88, i32 %and144
  %90 = bitcast i8* %add.ptr.i126 to %struct.malloc_chunk*
  %cmp122.i = icmp ult i8* %88, %add.ptr.i126
  br i1 %cmp122.i, label %if.then126.i, label %if.end438.i

if.then126.i:                                     ; preds = %if.then121.i
  %parent.i127 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %v.3.lcssa.i, i32 0, i32 5
  %91 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %parent.i127, align 4
  %bk.i128 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %v.3.lcssa.i, i32 0, i32 3
  %92 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %bk.i128, align 4
  %cmp127.i = icmp eq %struct.malloc_tree_chunk* %92, %v.3.lcssa.i
  br i1 %cmp127.i, label %if.else148.i, label %if.then129.i

if.then129.i:                                     ; preds = %if.then126.i
  %fd.i129 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %v.3.lcssa.i, i32 0, i32 2
  %93 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %fd.i129, align 4
  %94 = bitcast %struct.malloc_tree_chunk* %93 to i8*
  %cmp132.i = icmp ult i8* %94, %89
  br i1 %cmp132.i, label %if.else146.i, label %land.lhs.true134.i

land.lhs.true134.i:                               ; preds = %if.then129.i
  %bk135.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %93, i32 0, i32 3
  %95 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %bk135.i, align 4
  %cmp136.i = icmp eq %struct.malloc_tree_chunk* %95, %v.3.lcssa.i
  br i1 %cmp136.i, label %land.rhs.i130, label %if.else146.i

land.rhs.i130:                                    ; preds = %land.lhs.true134.i
  %fd138.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %92, i32 0, i32 2
  %96 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %fd138.i, align 4
  %cmp139.i = icmp eq %struct.malloc_tree_chunk* %96, %v.3.lcssa.i
  br i1 %cmp139.i, label %if.then143.i, label %if.else146.i

if.then143.i:                                     ; preds = %land.rhs.i130
  store %struct.malloc_tree_chunk* %92, %struct.malloc_tree_chunk** %bk135.i, align 4
  store %struct.malloc_tree_chunk* %93, %struct.malloc_tree_chunk** %fd138.i, align 4
  br label %if.end178.i

if.else146.i:                                     ; preds = %land.rhs.i130, %land.lhs.true134.i, %if.then129.i
  tail call void @abort() #6
  unreachable

if.else148.i:                                     ; preds = %if.then126.i
  %arrayidx150.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %v.3.lcssa.i, i32 0, i32 4, i32 1
  %97 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx150.i, align 4
  %cmp151.i = icmp eq %struct.malloc_tree_chunk* %97, null
  br i1 %cmp151.i, label %lor.lhs.false.i133, label %while.cond158.i

lor.lhs.false.i133:                               ; preds = %if.else148.i
  %arrayidx154.i131 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %v.3.lcssa.i, i32 0, i32 4, i32 0
  %98 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx154.i131, align 4
  %cmp155.i132 = icmp eq %struct.malloc_tree_chunk* %98, null
  br i1 %cmp155.i132, label %if.end178.i, label %while.cond158.i

while.cond158.i:                                  ; preds = %lor.rhs.i136, %while.cond158.i, %lor.lhs.false.i133, %if.else148.i
  %RP.0.i134 = phi %struct.malloc_tree_chunk** [ %arrayidx154.i131, %lor.lhs.false.i133 ], [ %arrayidx150.i, %if.else148.i ], [ %arrayidx160.i, %while.cond158.i ], [ %arrayidx164.i, %lor.rhs.i136 ]
  %R.0.i135 = phi %struct.malloc_tree_chunk* [ %98, %lor.lhs.false.i133 ], [ %97, %if.else148.i ], [ %99, %while.cond158.i ], [ %100, %lor.rhs.i136 ]
  %arrayidx160.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.0.i135, i32 0, i32 4, i32 1
  %99 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx160.i, align 4
  %cmp161.i = icmp eq %struct.malloc_tree_chunk* %99, null
  br i1 %cmp161.i, label %lor.rhs.i136, label %while.cond158.i

lor.rhs.i136:                                     ; preds = %while.cond158.i
  %arrayidx164.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.0.i135, i32 0, i32 4, i32 0
  %100 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx164.i, align 4
  %cmp165.i = icmp eq %struct.malloc_tree_chunk* %100, null
  br i1 %cmp165.i, label %while.end168.i, label %while.cond158.i

while.end168.i:                                   ; preds = %lor.rhs.i136
  %101 = bitcast %struct.malloc_tree_chunk** %RP.0.i134 to i8*
  %cmp170.i = icmp ult i8* %101, %89
  br i1 %cmp170.i, label %if.else175.i, label %if.then174.i

if.then174.i:                                     ; preds = %while.end168.i
  store %struct.malloc_tree_chunk* null, %struct.malloc_tree_chunk** %RP.0.i134, align 4
  br label %if.end178.i

if.else175.i:                                     ; preds = %while.end168.i
  tail call void @abort() #6
  unreachable

if.end178.i:                                      ; preds = %if.then174.i, %lor.lhs.false.i133, %if.then143.i
  %R.1.i137 = phi %struct.malloc_tree_chunk* [ %92, %if.then143.i ], [ %R.0.i135, %if.then174.i ], [ null, %lor.lhs.false.i133 ]
  %cmp179.i = icmp eq %struct.malloc_tree_chunk* %91, null
  br i1 %cmp179.i, label %if.end263.i, label %if.then181.i

if.then181.i:                                     ; preds = %if.end178.i
  %index.i138 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %v.3.lcssa.i, i32 0, i32 6
  %102 = load i32, i32* %index.i138, align 4
  %arrayidx183.i = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 11, i32 %102
  %103 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx183.i, align 4
  %cmp184.i = icmp eq %struct.malloc_tree_chunk* %v.3.lcssa.i, %103
  br i1 %cmp184.i, label %if.then186.i, label %if.else195.i

if.then186.i:                                     ; preds = %if.then181.i
  store %struct.malloc_tree_chunk* %R.1.i137, %struct.malloc_tree_chunk** %arrayidx183.i, align 4
  %cond18.i = icmp eq %struct.malloc_tree_chunk* %R.1.i137, null
  br i1 %cond18.i, label %if.end215.thread.i, label %if.then218.i

if.end215.thread.i:                               ; preds = %if.then186.i
  %shl191.i = shl i32 1, %102
  %neg.i139 = xor i32 %shl191.i, -1
  %104 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 1), align 4
  %and193.i = and i32 %104, %neg.i139
  store i32 %and193.i, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 1), align 4
  br label %if.end263.i

if.else195.i:                                     ; preds = %if.then181.i
  %105 = bitcast %struct.malloc_tree_chunk* %91 to i8*
  %106 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp197.i = icmp ult i8* %105, %106
  br i1 %cmp197.i, label %if.else213.i, label %if.then201.i140

if.then201.i140:                                  ; preds = %if.else195.i
  %arrayidx203.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %91, i32 0, i32 4, i32 0
  %107 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx203.i, align 4
  %cmp204.i = icmp eq %struct.malloc_tree_chunk* %107, %v.3.lcssa.i
  br i1 %cmp204.i, label %if.then206.i, label %if.else209.i

if.then206.i:                                     ; preds = %if.then201.i140
  store %struct.malloc_tree_chunk* %R.1.i137, %struct.malloc_tree_chunk** %arrayidx203.i, align 4
  br label %if.end215.i

if.else209.i:                                     ; preds = %if.then201.i140
  %arrayidx211.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %91, i32 0, i32 4, i32 1
  store %struct.malloc_tree_chunk* %R.1.i137, %struct.malloc_tree_chunk** %arrayidx211.i, align 4
  br label %if.end215.i

if.else213.i:                                     ; preds = %if.else195.i
  tail call void @abort() #6
  unreachable

if.end215.i:                                      ; preds = %if.else209.i, %if.then206.i
  %cmp216.i = icmp eq %struct.malloc_tree_chunk* %R.1.i137, null
  br i1 %cmp216.i, label %if.end263.i, label %if.then218.i

if.then218.i:                                     ; preds = %if.end215.i, %if.then186.i
  %108 = bitcast %struct.malloc_tree_chunk* %R.1.i137 to i8*
  %109 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp220.i = icmp ult i8* %108, %109
  br i1 %cmp220.i, label %if.else260.i, label %if.then224.i

if.then224.i:                                     ; preds = %if.then218.i
  %parent225.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.1.i137, i32 0, i32 5
  store %struct.malloc_tree_chunk* %91, %struct.malloc_tree_chunk** %parent225.i, align 4
  %arrayidx227.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %v.3.lcssa.i, i32 0, i32 4, i32 0
  %110 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx227.i, align 4
  %cmp228.i = icmp eq %struct.malloc_tree_chunk* %110, null
  br i1 %cmp228.i, label %if.end242.i, label %if.then230.i

if.then230.i:                                     ; preds = %if.then224.i
  %111 = bitcast %struct.malloc_tree_chunk* %110 to i8*
  %112 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp232.i = icmp ult i8* %111, %112
  br i1 %cmp232.i, label %if.else240.i, label %if.then236.i

if.then236.i:                                     ; preds = %if.then230.i
  %arrayidx238.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.1.i137, i32 0, i32 4, i32 0
  store %struct.malloc_tree_chunk* %110, %struct.malloc_tree_chunk** %arrayidx238.i, align 4
  %parent239.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %110, i32 0, i32 5
  store %struct.malloc_tree_chunk* %R.1.i137, %struct.malloc_tree_chunk** %parent239.i, align 4
  br label %if.end242.i

if.else240.i:                                     ; preds = %if.then230.i
  tail call void @abort() #6
  unreachable

if.end242.i:                                      ; preds = %if.then236.i, %if.then224.i
  %arrayidx244.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %v.3.lcssa.i, i32 0, i32 4, i32 1
  %113 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx244.i, align 4
  %cmp245.i = icmp eq %struct.malloc_tree_chunk* %113, null
  br i1 %cmp245.i, label %if.end263.i, label %if.then247.i

if.then247.i:                                     ; preds = %if.end242.i
  %114 = bitcast %struct.malloc_tree_chunk* %113 to i8*
  %115 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp249.i = icmp ult i8* %114, %115
  br i1 %cmp249.i, label %if.else257.i, label %if.then253.i

if.then253.i:                                     ; preds = %if.then247.i
  %arrayidx255.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.1.i137, i32 0, i32 4, i32 1
  store %struct.malloc_tree_chunk* %113, %struct.malloc_tree_chunk** %arrayidx255.i, align 4
  %parent256.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %113, i32 0, i32 5
  store %struct.malloc_tree_chunk* %R.1.i137, %struct.malloc_tree_chunk** %parent256.i, align 4
  br label %if.end263.i

if.else257.i:                                     ; preds = %if.then247.i
  tail call void @abort() #6
  unreachable

if.else260.i:                                     ; preds = %if.then218.i
  tail call void @abort() #6
  unreachable

if.end263.i:                                      ; preds = %if.then253.i, %if.end242.i, %if.end215.i, %if.end215.thread.i, %if.end178.i
  %cmp264.i = icmp ult i32 %rsize.3.lcssa.i, 16
  br i1 %cmp264.i, label %if.then266.i, label %if.else275.i

if.then266.i:                                     ; preds = %if.end263.i
  %add267.i = add i32 %rsize.3.lcssa.i, %and144
  %or269.i = or i32 %add267.i, 3
  %head270.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %v.3.lcssa.i, i32 0, i32 1
  store i32 %or269.i, i32* %head270.i, align 4
  %add.ptr272.sum.i = add i32 %add267.i, 4
  %head273.i = getelementptr inbounds i8, i8* %88, i32 %add.ptr272.sum.i
  %116 = bitcast i8* %head273.i to i32*
  %117 = load i32, i32* %116, align 4
  %or274.i = or i32 %117, 1
  store i32 %or274.i, i32* %116, align 4
  br label %tmalloc_large.exit

if.else275.i:                                     ; preds = %if.end263.i
  %or277.i = or i32 %and144, 3
  %head278.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %v.3.lcssa.i, i32 0, i32 1
  store i32 %or277.i, i32* %head278.i, align 4
  %or279.i = or i32 %rsize.3.lcssa.i, 1
  %add.ptr.sum.i141172 = or i32 %and144, 4
  %head280.i = getelementptr inbounds i8, i8* %88, i32 %add.ptr.sum.i141172
  %118 = bitcast i8* %head280.i to i32*
  store i32 %or279.i, i32* %118, align 4
  %add.ptr.sum1.i142 = add i32 %rsize.3.lcssa.i, %and144
  %add.ptr281.i = getelementptr inbounds i8, i8* %88, i32 %add.ptr.sum1.i142
  %prev_foot.i143 = bitcast i8* %add.ptr281.i to i32*
  store i32 %rsize.3.lcssa.i, i32* %prev_foot.i143, align 4
  %shr282.i = lshr i32 %rsize.3.lcssa.i, 3
  %cmp283.i = icmp ult i32 %rsize.3.lcssa.i, 256
  br i1 %cmp283.i, label %if.then285.i, label %if.else313.i

if.then285.i:                                     ; preds = %if.else275.i
  %shl287.i = shl nuw nsw i32 %shr282.i, 1
  %arrayidx288.i = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %shl287.i
  %119 = bitcast %struct.malloc_chunk** %arrayidx288.i to %struct.malloc_chunk*
  %120 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  %shl290.i = shl i32 1, %shr282.i
  %and291.i = and i32 %120, %shl290.i
  %tobool292.i = icmp eq i32 %and291.i, 0
  br i1 %tobool292.i, label %if.then293.i, label %if.else297.i

if.then293.i:                                     ; preds = %if.then285.i
  %or296.i = or i32 %120, %shl290.i
  store i32 %or296.i, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  %arrayidx288.sum.pre.i = add i32 %shl287.i, 2
  %.pre.i144 = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %arrayidx288.sum.pre.i
  br label %if.end308.i

if.else297.i:                                     ; preds = %if.then285.i
  %arrayidx288.sum15.i = add i32 %shl287.i, 2
  %121 = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %arrayidx288.sum15.i
  %122 = load %struct.malloc_chunk*, %struct.malloc_chunk** %121, align 4
  %123 = bitcast %struct.malloc_chunk* %122 to i8*
  %124 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp300.i = icmp ult i8* %123, %124
  br i1 %cmp300.i, label %if.else306.i, label %if.end308.i

if.else306.i:                                     ; preds = %if.else297.i
  tail call void @abort() #6
  unreachable

if.end308.i:                                      ; preds = %if.else297.i, %if.then293.i
  %.pre-phi.i145 = phi %struct.malloc_chunk** [ %121, %if.else297.i ], [ %.pre.i144, %if.then293.i ]
  %F289.0.i = phi %struct.malloc_chunk* [ %122, %if.else297.i ], [ %119, %if.then293.i ]
  store %struct.malloc_chunk* %90, %struct.malloc_chunk** %.pre-phi.i145, align 4
  %bk310.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %F289.0.i, i32 0, i32 3
  store %struct.malloc_chunk* %90, %struct.malloc_chunk** %bk310.i, align 4
  %add.ptr.sum13.i = add i32 %and144, 8
  %fd311.i = getelementptr inbounds i8, i8* %88, i32 %add.ptr.sum13.i
  %125 = bitcast i8* %fd311.i to %struct.malloc_chunk**
  store %struct.malloc_chunk* %F289.0.i, %struct.malloc_chunk** %125, align 4
  %add.ptr.sum14.i = add i32 %and144, 12
  %bk312.i = getelementptr inbounds i8, i8* %88, i32 %add.ptr.sum14.i
  %126 = bitcast i8* %bk312.i to %struct.malloc_chunk**
  store %struct.malloc_chunk* %119, %struct.malloc_chunk** %126, align 4
  br label %tmalloc_large.exit

if.else313.i:                                     ; preds = %if.else275.i
  %127 = bitcast i8* %add.ptr.i126 to %struct.malloc_tree_chunk*
  %shr317.i = lshr i32 %rsize.3.lcssa.i, 8
  %cmp318.i = icmp eq i32 %shr317.i, 0
  br i1 %cmp318.i, label %if.end352.i, label %if.else321.i

if.else321.i:                                     ; preds = %if.else313.i
  %cmp322.i = icmp ugt i32 %rsize.3.lcssa.i, 16777215
  br i1 %cmp322.i, label %if.end352.i, label %if.else325.i

if.else325.i:                                     ; preds = %if.else321.i
  %sub328.i = add i32 %shr317.i, 1048320
  %shr329.i = lshr i32 %sub328.i, 16
  %and330.i = and i32 %shr329.i, 8
  %shl332.i = shl i32 %shr317.i, %and330.i
  %sub333.i = add i32 %shl332.i, 520192
  %shr334.i = lshr i32 %sub333.i, 16
  %and335.i = and i32 %shr334.i, 4
  %add336.i = or i32 %and335.i, %and330.i
  %shl337.i = shl i32 %shl332.i, %and335.i
  %sub338.i = add i32 %shl337.i, 245760
  %shr339.i = lshr i32 %sub338.i, 16
  %and340.i = and i32 %shr339.i, 2
  %add341.i = or i32 %add336.i, %and340.i
  %sub342.i = sub i32 14, %add341.i
  %shl343.i = shl i32 %shl337.i, %and340.i
  %shr344.i = lshr i32 %shl343.i, 15
  %add345.i = add i32 %sub342.i, %shr344.i
  %shl346.i = shl nsw i32 %add345.i, 1
  %add347.i = add i32 %add345.i, 7
  %shr348.i = lshr i32 %rsize.3.lcssa.i, %add347.i
  %and349.i = and i32 %shr348.i, 1
  %add350.i = or i32 %and349.i, %shl346.i
  br label %if.end352.i

if.end352.i:                                      ; preds = %if.else325.i, %if.else321.i, %if.else313.i
  %I315.0.i = phi i32 [ %add350.i, %if.else325.i ], [ 0, %if.else313.i ], [ 31, %if.else321.i ]
  %arrayidx354.i = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 11, i32 %I315.0.i
  %add.ptr.sum2.i = add i32 %and144, 28
  %index355.i = getelementptr inbounds i8, i8* %88, i32 %add.ptr.sum2.i
  %128 = bitcast i8* %index355.i to i32*
  store i32 %I315.0.i, i32* %128, align 4
  %add.ptr.sum3.i = add i32 %and144, 16
  %child356.i = getelementptr inbounds i8, i8* %88, i32 %add.ptr.sum3.i
  %child356.sum.i = add i32 %and144, 20
  %arrayidx357.i = getelementptr inbounds i8, i8* %88, i32 %child356.sum.i
  %129 = bitcast i8* %arrayidx357.i to %struct.malloc_tree_chunk**
  store %struct.malloc_tree_chunk* null, %struct.malloc_tree_chunk** %129, align 4
  %arrayidx359.i = bitcast i8* %child356.i to %struct.malloc_tree_chunk**
  store %struct.malloc_tree_chunk* null, %struct.malloc_tree_chunk** %arrayidx359.i, align 4
  %130 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 1), align 4
  %shl361.i = shl i32 1, %I315.0.i
  %and362.i = and i32 %130, %shl361.i
  %tobool363.i = icmp eq i32 %and362.i, 0
  br i1 %tobool363.i, label %if.then364.i, label %if.else371.i

if.then364.i:                                     ; preds = %if.end352.i
  %or367.i = or i32 %130, %shl361.i
  store i32 %or367.i, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 1), align 4
  store %struct.malloc_tree_chunk* %127, %struct.malloc_tree_chunk** %arrayidx354.i, align 4
  %131 = bitcast %struct.malloc_tree_chunk** %arrayidx354.i to %struct.malloc_tree_chunk*
  %add.ptr.sum4.i = add i32 %and144, 24
  %parent368.i = getelementptr inbounds i8, i8* %88, i32 %add.ptr.sum4.i
  %132 = bitcast i8* %parent368.i to %struct.malloc_tree_chunk**
  store %struct.malloc_tree_chunk* %131, %struct.malloc_tree_chunk** %132, align 4
  %add.ptr.sum5.i = add i32 %and144, 12
  %bk369.i = getelementptr inbounds i8, i8* %88, i32 %add.ptr.sum5.i
  %133 = bitcast i8* %bk369.i to %struct.malloc_tree_chunk**
  store %struct.malloc_tree_chunk* %127, %struct.malloc_tree_chunk** %133, align 4
  %add.ptr.sum6.i = add i32 %and144, 8
  %fd370.i = getelementptr inbounds i8, i8* %88, i32 %add.ptr.sum6.i
  %134 = bitcast i8* %fd370.i to %struct.malloc_tree_chunk**
  store %struct.malloc_tree_chunk* %127, %struct.malloc_tree_chunk** %134, align 4
  br label %tmalloc_large.exit

if.else371.i:                                     ; preds = %if.end352.i
  %135 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx354.i, align 4
  %cmp373.i = icmp eq i32 %I315.0.i, 31
  br i1 %cmp373.i, label %cond.end381.i, label %cond.false376.i

cond.false376.i:                                  ; preds = %if.else371.i
  %shr377.i = lshr i32 %I315.0.i, 1
  %sub380.i = sub i32 25, %shr377.i
  br label %cond.end381.i

cond.end381.i:                                    ; preds = %cond.false376.i, %if.else371.i
  %cond382.i = phi i32 [ %sub380.i, %cond.false376.i ], [ 0, %if.else371.i ]
  %head38520.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %135, i32 0, i32 1
  %136 = load i32, i32* %head38520.i, align 4
  %and38621.i = and i32 %136, -8
  %cmp38722.i = icmp eq i32 %and38621.i, %rsize.3.lcssa.i
  br i1 %cmp38722.i, label %if.else410.i, label %if.then389.lr.ph.i

if.then389.lr.ph.i:                               ; preds = %cond.end381.i
  %shl383.i = shl i32 %rsize.3.lcssa.i, %cond382.i
  br label %if.then389.i

for.cond384.i:                                    ; preds = %if.then389.i
  %shl394.i = shl i32 %K372.024.i, 1
  %head385.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %138, i32 0, i32 1
  %137 = load i32, i32* %head385.i, align 4
  %and386.i = and i32 %137, -8
  %cmp387.i = icmp eq i32 %and386.i, %rsize.3.lcssa.i
  br i1 %cmp387.i, label %if.else410.i, label %if.then389.i

if.then389.i:                                     ; preds = %for.cond384.i, %if.then389.lr.ph.i
  %K372.024.i = phi i32 [ %shl383.i, %if.then389.lr.ph.i ], [ %shl394.i, %for.cond384.i ]
  %T.023.i = phi %struct.malloc_tree_chunk* [ %135, %if.then389.lr.ph.i ], [ %138, %for.cond384.i ]
  %shr390.i = lshr i32 %K372.024.i, 31
  %arrayidx393.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %T.023.i, i32 0, i32 4, i32 %shr390.i
  %138 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx393.i, align 4
  %cmp395.i = icmp eq %struct.malloc_tree_chunk* %138, null
  br i1 %cmp395.i, label %if.else398.i, label %for.cond384.i

if.else398.i:                                     ; preds = %if.then389.i
  %139 = bitcast %struct.malloc_tree_chunk** %arrayidx393.i to i8*
  %140 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp400.i = icmp ult i8* %139, %140
  br i1 %cmp400.i, label %if.else408.i, label %if.then404.i

if.then404.i:                                     ; preds = %if.else398.i
  store %struct.malloc_tree_chunk* %127, %struct.malloc_tree_chunk** %arrayidx393.i, align 4
  %add.ptr.sum10.i = add i32 %and144, 24
  %parent405.i = getelementptr inbounds i8, i8* %88, i32 %add.ptr.sum10.i
  %141 = bitcast i8* %parent405.i to %struct.malloc_tree_chunk**
  store %struct.malloc_tree_chunk* %T.023.i, %struct.malloc_tree_chunk** %141, align 4
  %add.ptr.sum11.i = add i32 %and144, 12
  %bk406.i = getelementptr inbounds i8, i8* %88, i32 %add.ptr.sum11.i
  %142 = bitcast i8* %bk406.i to %struct.malloc_tree_chunk**
  store %struct.malloc_tree_chunk* %127, %struct.malloc_tree_chunk** %142, align 4
  %add.ptr.sum12.i = add i32 %and144, 8
  %fd407.i = getelementptr inbounds i8, i8* %88, i32 %add.ptr.sum12.i
  %143 = bitcast i8* %fd407.i to %struct.malloc_tree_chunk**
  store %struct.malloc_tree_chunk* %127, %struct.malloc_tree_chunk** %143, align 4
  br label %tmalloc_large.exit

if.else408.i:                                     ; preds = %if.else398.i
  tail call void @abort() #6
  unreachable

if.else410.i:                                     ; preds = %for.cond384.i, %cond.end381.i
  %T.0.lcssa.i = phi %struct.malloc_tree_chunk* [ %135, %cond.end381.i ], [ %138, %for.cond384.i ]
  %fd412.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %T.0.lcssa.i, i32 0, i32 2
  %144 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %fd412.i, align 4
  %145 = bitcast %struct.malloc_tree_chunk* %T.0.lcssa.i to i8*
  %146 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp414.i = icmp ult i8* %145, %146
  br i1 %cmp414.i, label %if.else430.i, label %land.rhs416.i

land.rhs416.i:                                    ; preds = %if.else410.i
  %147 = bitcast %struct.malloc_tree_chunk* %144 to i8*
  %cmp418.i = icmp ult i8* %147, %146
  br i1 %cmp418.i, label %if.else430.i, label %if.then424.i

if.then424.i:                                     ; preds = %land.rhs416.i
  %bk425.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %144, i32 0, i32 3
  store %struct.malloc_tree_chunk* %127, %struct.malloc_tree_chunk** %bk425.i, align 4
  store %struct.malloc_tree_chunk* %127, %struct.malloc_tree_chunk** %fd412.i, align 4
  %add.ptr.sum7.i = add i32 %and144, 8
  %fd427.i = getelementptr inbounds i8, i8* %88, i32 %add.ptr.sum7.i
  %148 = bitcast i8* %fd427.i to %struct.malloc_tree_chunk**
  store %struct.malloc_tree_chunk* %144, %struct.malloc_tree_chunk** %148, align 4
  %add.ptr.sum8.i = add i32 %and144, 12
  %bk428.i = getelementptr inbounds i8, i8* %88, i32 %add.ptr.sum8.i
  %149 = bitcast i8* %bk428.i to %struct.malloc_tree_chunk**
  store %struct.malloc_tree_chunk* %T.0.lcssa.i, %struct.malloc_tree_chunk** %149, align 4
  %add.ptr.sum9.i = add i32 %and144, 24
  %parent429.i = getelementptr inbounds i8, i8* %88, i32 %add.ptr.sum9.i
  %150 = bitcast i8* %parent429.i to %struct.malloc_tree_chunk**
  store %struct.malloc_tree_chunk* null, %struct.malloc_tree_chunk** %150, align 4
  br label %tmalloc_large.exit

if.else430.i:                                     ; preds = %land.rhs416.i, %if.else410.i
  tail call void @abort() #6
  unreachable

if.end438.i:                                      ; preds = %if.then121.i, %if.then119.i
  tail call void @abort() #6
  unreachable

tmalloc_large.exit:                               ; preds = %if.then424.i, %if.then404.i, %if.then364.i, %if.end308.i, %if.then266.i
  %add.ptr436.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %v.3.lcssa.i, i32 0, i32 2
  %151 = bitcast %struct.malloc_tree_chunk** %add.ptr436.i to i8*
  br label %postaction

if.end154:                                        ; preds = %land.lhs.true116.i, %while.end.i125, %if.then57.i, %if.else141, %if.else137, %if.else127, %if.else28
  %nb.0 = phi i32 [ %cond, %if.else127 ], [ %cond, %if.else28 ], [ -1, %if.else137 ], [ %and144, %if.else141 ], [ %and144, %while.end.i125 ], [ %and144, %land.lhs.true116.i ], [ %and144, %if.then57.i ]
  %152 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 2), align 4
  %cmp155 = icmp ugt i32 %nb.0, %152
  br i1 %cmp155, label %if.else182, label %if.then157

if.then157:                                       ; preds = %if.end154
  %sub159 = sub i32 %152, %nb.0
  %153 = load %struct.malloc_chunk*, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 5), align 4
  %cmp161 = icmp ugt i32 %sub159, 15
  br i1 %cmp161, label %if.then163, label %if.else173

if.then163:                                       ; preds = %if.then157
  %154 = bitcast %struct.malloc_chunk* %153 to i8*
  %add.ptr165 = getelementptr inbounds i8, i8* %154, i32 %nb.0
  %155 = bitcast i8* %add.ptr165 to %struct.malloc_chunk*
  store %struct.malloc_chunk* %155, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 5), align 4
  store i32 %sub159, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 2), align 4
  %or166 = or i32 %sub159, 1
  %add.ptr165.sum = add i32 %nb.0, 4
  %head167 = getelementptr inbounds i8, i8* %154, i32 %add.ptr165.sum
  %156 = bitcast i8* %head167 to i32*
  store i32 %or166, i32* %156, align 4
  %add.ptr168 = getelementptr inbounds i8, i8* %154, i32 %152
  %prev_foot169 = bitcast i8* %add.ptr168 to i32*
  store i32 %sub159, i32* %prev_foot169, align 4
  %or171 = or i32 %nb.0, 3
  %head172 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %153, i32 0, i32 1
  store i32 %or171, i32* %head172, align 4
  br label %if.end180

if.else173:                                       ; preds = %if.then157
  store i32 0, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 2), align 4
  store %struct.malloc_chunk* null, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 5), align 4
  %or175 = or i32 %152, 3
  %head176 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %153, i32 0, i32 1
  store i32 %or175, i32* %head176, align 4
  %157 = bitcast %struct.malloc_chunk* %153 to i8*
  %add.ptr177.sum = add i32 %152, 4
  %head178 = getelementptr inbounds i8, i8* %157, i32 %add.ptr177.sum
  %158 = bitcast i8* %head178 to i32*
  %159 = load i32, i32* %158, align 4
  %or179 = or i32 %159, 1
  store i32 %or179, i32* %158, align 4
  br label %if.end180

if.end180:                                        ; preds = %if.else173, %if.then163
  %add.ptr181 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %153, i32 0, i32 2
  %160 = bitcast %struct.malloc_chunk** %add.ptr181 to i8*
  br label %postaction

if.else182:                                       ; preds = %if.end154
  %161 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 3), align 4
  %cmp183 = icmp ult i32 %nb.0, %161
  br i1 %cmp183, label %if.then185, label %if.end198

if.then185:                                       ; preds = %if.else182
  %sub187 = sub i32 %161, %nb.0
  store i32 %sub187, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 3), align 4
  %162 = load %struct.malloc_chunk*, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 6), align 4
  %163 = bitcast %struct.malloc_chunk* %162 to i8*
  %add.ptr190 = getelementptr inbounds i8, i8* %163, i32 %nb.0
  %164 = bitcast i8* %add.ptr190 to %struct.malloc_chunk*
  store %struct.malloc_chunk* %164, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 6), align 4
  %or191 = or i32 %sub187, 1
  %add.ptr190.sum = add i32 %nb.0, 4
  %head192 = getelementptr inbounds i8, i8* %163, i32 %add.ptr190.sum
  %165 = bitcast i8* %head192 to i32*
  store i32 %or191, i32* %165, align 4
  %or194 = or i32 %nb.0, 3
  %head195 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %162, i32 0, i32 1
  store i32 %or194, i32* %head195, align 4
  %add.ptr196 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %162, i32 0, i32 2
  %166 = bitcast %struct.malloc_chunk** %add.ptr196 to i8*
  br label %postaction

if.end198:                                        ; preds = %if.else182
  %167 = load i32, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 0), align 4
  %cmp.i146 = icmp eq i32 %167, 0
  br i1 %cmp.i146, label %if.then.i.i, label %if.end8.i

if.then.i.i:                                      ; preds = %if.end198
  %call.i.i = tail call i32 @sysconf(i32 30) #1
  %sub.i.i = add i32 %call.i.i, -1
  %and.i.i = and i32 %sub.i.i, %call.i.i
  %cmp1.i.i = icmp eq i32 %and.i.i, 0
  br i1 %cmp1.i.i, label %init_mparams.exit.i, label %if.then5.i.i

if.then5.i.i:                                     ; preds = %if.then.i.i
  tail call void @abort() #6
  unreachable

init_mparams.exit.i:                              ; preds = %if.then.i.i
  store i32 %call.i.i, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 2), align 4
  store i32 %call.i.i, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 1), align 4
  store i32 -1, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 3), align 4
  store i32 -1, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 4), align 4
  store i32 0, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 5), align 4
  store i32 0, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 15), align 4
  %call6.i.i = tail call i32 @time(i32* null) #1
  %xor.i.i = and i32 %call6.i.i, -16
  %and7.i.i = xor i32 %xor.i.i, 1431655768
  store volatile i32 %and7.i.i, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 0), align 4
  br label %if.end8.i

if.end8.i:                                        ; preds = %init_mparams.exit.i, %if.end198
  %add.i147 = add i32 %nb.0, 48
  %168 = load i32, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 2), align 4
  %sub.i148 = add i32 %nb.0, 47
  %add9.i = add i32 %168, %sub.i148
  %neg.i149 = sub i32 0, %168
  %and11.i = and i32 %add9.i, %neg.i149
  %cmp12.i = icmp ugt i32 %and11.i, %nb.0
  br i1 %cmp12.i, label %if.end14.i, label %postaction

if.end14.i:                                       ; preds = %if.end8.i
  %169 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 14), align 4
  %cmp15.i = icmp eq i32 %169, 0
  br i1 %cmp15.i, label %if.end24.i, label %if.then16.i

if.then16.i:                                      ; preds = %if.end14.i
  %170 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 12), align 4
  %add17.i150 = add i32 %170, %and11.i
  %cmp19.i = icmp ule i32 %add17.i150, %170
  %cmp21.i = icmp ugt i32 %add17.i150, %169
  %or.cond1.i = or i1 %cmp19.i, %cmp21.i
  br i1 %or.cond1.i, label %postaction, label %if.end24.i

if.end24.i:                                       ; preds = %if.then16.i, %if.end14.i
  %171 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 15), align 4
  %and26.i = and i32 %171, 4
  %tobool27.i = icmp eq i32 %and26.i, 0
  br i1 %tobool27.i, label %if.then28.i, label %if.end121.i

if.then28.i:                                      ; preds = %if.end24.i
  %172 = load %struct.malloc_chunk*, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 6), align 4
  %cmp29.i = icmp eq %struct.malloc_chunk* %172, null
  br i1 %cmp29.i, label %if.then33.i, label %cond.false.i151

cond.false.i151:                                  ; preds = %if.then28.i
  %173 = bitcast %struct.malloc_chunk* %172 to i8*
  br label %for.cond.i.i

for.cond.i.i:                                     ; preds = %if.end.i10.i, %cond.false.i151
  %sp.0.i.i = phi %struct.malloc_segment* [ getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 16), %cond.false.i151 ], [ %176, %if.end.i10.i ]
  %base.i.i = getelementptr inbounds %struct.malloc_segment, %struct.malloc_segment* %sp.0.i.i, i32 0, i32 0
  %174 = load i8*, i8** %base.i.i, align 4
  %cmp.i9.i = icmp ugt i8* %174, %173
  br i1 %cmp.i9.i, label %if.end.i10.i, label %land.lhs.true.i.i

land.lhs.true.i.i:                                ; preds = %for.cond.i.i
  %size.i.i = getelementptr inbounds %struct.malloc_segment, %struct.malloc_segment* %sp.0.i.i, i32 0, i32 1
  %175 = load i32, i32* %size.i.i, align 4
  %add.ptr.i.i = getelementptr inbounds i8, i8* %174, i32 %175
  %cmp2.i.i = icmp ugt i8* %add.ptr.i.i, %173
  br i1 %cmp2.i.i, label %cond.end.i153, label %if.end.i10.i

if.end.i10.i:                                     ; preds = %land.lhs.true.i.i, %for.cond.i.i
  %next.i.i = getelementptr inbounds %struct.malloc_segment, %struct.malloc_segment* %sp.0.i.i, i32 0, i32 2
  %176 = load %struct.malloc_segment*, %struct.malloc_segment** %next.i.i, align 4
  %cmp3.i.i = icmp eq %struct.malloc_segment* %176, null
  br i1 %cmp3.i.i, label %if.then33.i, label %for.cond.i.i

cond.end.i153:                                    ; preds = %land.lhs.true.i.i
  %cmp32.i152 = icmp eq %struct.malloc_segment* %sp.0.i.i, null
  br i1 %cmp32.i152, label %if.then33.i, label %if.else.i159

if.then33.i:                                      ; preds = %cond.end.i153, %if.end.i10.i, %if.then28.i
  %call34.i = tail call i8* @sbrk(i32 0) #1
  %cmp35.i154 = icmp eq i8* %call34.i, inttoptr (i32 -1 to i8*)
  br i1 %cmp35.i154, label %if.else117.i, label %if.then36.i

if.then36.i:                                      ; preds = %if.then33.i
  %177 = ptrtoint i8* %call34.i to i32
  %178 = load i32, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 1), align 4
  %sub38.i = add i32 %178, -1
  %and39.i = and i32 %sub38.i, %177
  %cmp40.i155 = icmp eq i32 %and39.i, 0
  br i1 %cmp40.i155, label %if.end49.i, label %if.then41.i

if.then41.i:                                      ; preds = %if.then36.i
  %add43.i = add i32 %sub38.i, %177
  %neg45.i = sub i32 0, %178
  %and46.i = and i32 %add43.i, %neg45.i
  %sub47.i = sub i32 %and11.i, %177
  %add48.i = add i32 %sub47.i, %and46.i
  br label %if.end49.i

if.end49.i:                                       ; preds = %if.then41.i, %if.then36.i
  %ssize.0.i = phi i32 [ %and11.i, %if.then36.i ], [ %add48.i, %if.then41.i ]
  %179 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 12), align 4
  %add51.i = add i32 %179, %ssize.0.i
  %cmp52.i = icmp ugt i32 %ssize.0.i, %nb.0
  %cmp54.i156 = icmp ult i32 %ssize.0.i, 2147483647
  %or.cond.i157 = and i1 %cmp52.i, %cmp54.i156
  br i1 %or.cond.i157, label %land.lhs.true55.i, label %if.else117.i

land.lhs.true55.i:                                ; preds = %if.end49.i
  %180 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 14), align 4
  %cmp57.i = icmp eq i32 %180, 0
  br i1 %cmp57.i, label %land.lhs.true64.i, label %lor.lhs.false58.i

lor.lhs.false58.i:                                ; preds = %land.lhs.true55.i
  %cmp60.i = icmp ule i32 %add51.i, %179
  %cmp63.i = icmp ugt i32 %add51.i, %180
  %or.cond2.i = or i1 %cmp60.i, %cmp63.i
  br i1 %or.cond2.i, label %if.else117.i, label %land.lhs.true64.i

land.lhs.true64.i:                                ; preds = %lor.lhs.false58.i, %land.lhs.true55.i
  %call65.i = tail call i8* @sbrk(i32 %ssize.0.i) #1
  %cmp66.i158 = icmp eq i8* %call65.i, %call34.i
  %ssize.0..i = select i1 %cmp66.i158, i32 %ssize.0.i, i32 0
  %call34..i = select i1 %cmp66.i158, i8* %call34.i, i8* inttoptr (i32 -1 to i8*)
  br label %if.end85.i

if.else.i159:                                     ; preds = %cond.end.i153
  %181 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 3), align 4
  %add74.i = sub i32 %add9.i, %181
  %and77.i = and i32 %add74.i, %neg.i149
  %cmp78.i = icmp ult i32 %and77.i, 2147483647
  br i1 %cmp78.i, label %land.lhs.true79.i, label %if.else117.i

land.lhs.true79.i:                                ; preds = %if.else.i159
  %call80.i = tail call i8* @sbrk(i32 %and77.i) #1
  %182 = load i8*, i8** %base.i.i, align 4
  %183 = load i32, i32* %size.i.i, align 4
  %add.ptr.i160 = getelementptr inbounds i8, i8* %182, i32 %183
  %cmp82.i = icmp eq i8* %call80.i, %add.ptr.i160
  %and77..i = select i1 %cmp82.i, i32 %and77.i, i32 0
  %call80..i = select i1 %cmp82.i, i8* %call80.i, i8* inttoptr (i32 -1 to i8*)
  br label %if.end85.i

if.end85.i:                                       ; preds = %land.lhs.true79.i, %land.lhs.true64.i
  %ssize.1.i = phi i32 [ %ssize.0.i, %land.lhs.true64.i ], [ %and77.i, %land.lhs.true79.i ]
  %br.0.i = phi i8* [ %call65.i, %land.lhs.true64.i ], [ %call80.i, %land.lhs.true79.i ]
  %tsize.0.i = phi i32 [ %ssize.0..i, %land.lhs.true64.i ], [ %and77..i, %land.lhs.true79.i ]
  %tbase.0.i = phi i8* [ %call34..i, %land.lhs.true64.i ], [ %call80..i, %land.lhs.true79.i ]
  %sub109.i = sub i32 0, %ssize.1.i
  %cmp86.i = icmp eq i8* %tbase.0.i, inttoptr (i32 -1 to i8*)
  br i1 %cmp86.i, label %if.then87.i, label %if.then145.i

if.then87.i:                                      ; preds = %if.end85.i
  %cmp88.i = icmp ne i8* %br.0.i, inttoptr (i32 -1 to i8*)
  %cmp90.i161 = icmp ult i32 %ssize.1.i, 2147483647
  %or.cond3.i = and i1 %cmp88.i, %cmp90.i161
  %cmp93.i = icmp ult i32 %ssize.1.i, %add.i147
  %or.cond4.i = and i1 %or.cond3.i, %cmp93.i
  br i1 %or.cond4.i, label %if.then94.i, label %if.end114.i

if.then94.i:                                      ; preds = %if.then87.i
  %184 = load i32, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 2), align 4
  %sub96.i = sub i32 %sub.i148, %ssize.1.i
  %add98.i = add i32 %sub96.i, %184
  %neg100.i = sub i32 0, %184
  %and101.i = and i32 %add98.i, %neg100.i
  %cmp102.i = icmp ult i32 %and101.i, 2147483647
  br i1 %cmp102.i, label %if.then103.i, label %if.end114.i

if.then103.i:                                     ; preds = %if.then94.i
  %call104.i = tail call i8* @sbrk(i32 %and101.i) #1
  %cmp105.i = icmp eq i8* %call104.i, inttoptr (i32 -1 to i8*)
  br i1 %cmp105.i, label %if.else108.i, label %if.then106.i

if.then106.i:                                     ; preds = %if.then103.i
  %add107.i = add i32 %and101.i, %ssize.1.i
  br label %if.end114.i

if.else108.i:                                     ; preds = %if.then103.i
  %call110.i = tail call i8* @sbrk(i32 %sub109.i) #1
  br label %if.else117.i

if.end114.i:                                      ; preds = %if.then106.i, %if.then94.i, %if.then87.i
  %ssize.2.i = phi i32 [ %add107.i, %if.then106.i ], [ %ssize.1.i, %if.then94.i ], [ %ssize.1.i, %if.then87.i ]
  %cmp115.i162 = icmp eq i8* %br.0.i, inttoptr (i32 -1 to i8*)
  br i1 %cmp115.i162, label %if.else117.i, label %if.then145.i

if.else117.i:                                     ; preds = %if.end114.i, %if.else108.i, %if.else.i159, %lor.lhs.false58.i, %if.end49.i, %if.then33.i
  %tsize.0748284.i = phi i32 [ %tsize.0.i, %if.else108.i ], [ 0, %if.else.i159 ], [ 0, %if.then33.i ], [ 0, %if.end49.i ], [ 0, %lor.lhs.false58.i ], [ %tsize.0.i, %if.end114.i ]
  %185 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 15), align 4
  %or.i163 = or i32 %185, 4
  store i32 %or.i163, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 15), align 4
  br label %if.end121.i

if.end121.i:                                      ; preds = %if.else117.i, %if.end24.i
  %tsize.1.i = phi i32 [ 0, %if.end24.i ], [ %tsize.0748284.i, %if.else117.i ]
  %cmp124.i = icmp ult i32 %and11.i, 2147483647
  br i1 %cmp124.i, label %if.then125.i, label %if.end264.i

if.then125.i:                                     ; preds = %if.end121.i
  %call128.i = tail call i8* @sbrk(i32 %and11.i) #1
  %call129.i = tail call i8* @sbrk(i32 0) #1
  %notlhs.i = icmp ne i8* %call128.i, inttoptr (i32 -1 to i8*)
  %notrhs.i = icmp ne i8* %call129.i, inttoptr (i32 -1 to i8*)
  %or.cond6.not.i = and i1 %notrhs.i, %notlhs.i
  %cmp134.i = icmp ult i8* %call128.i, %call129.i
  %or.cond7.i = and i1 %or.cond6.not.i, %cmp134.i
  br i1 %or.cond7.i, label %if.end143.i, label %if.end264.i

if.end143.i:                                      ; preds = %if.then125.i
  %sub.ptr.lhs.cast.i = ptrtoint i8* %call129.i to i32
  %sub.ptr.rhs.cast.i = ptrtoint i8* %call128.i to i32
  %sub.ptr.sub.i = sub i32 %sub.ptr.lhs.cast.i, %sub.ptr.rhs.cast.i
  %add137.i = add i32 %nb.0, 40
  %cmp138.i164 = icmp ugt i32 %sub.ptr.sub.i, %add137.i
  %sub.ptr.sub.tsize.1.i = select i1 %cmp138.i164, i32 %sub.ptr.sub.i, i32 %tsize.1.i
  br i1 %cmp138.i164, label %if.then145.i, label %if.end264.i

if.then145.i:                                     ; preds = %if.end143.i, %if.end114.i, %if.end85.i
  %tbase.291.i = phi i8* [ %tbase.0.i, %if.end85.i ], [ %br.0.i, %if.end114.i ], [ %call128.i, %if.end143.i ]
  %tsize.290.i = phi i32 [ %tsize.0.i, %if.end85.i ], [ %ssize.2.i, %if.end114.i ], [ %sub.ptr.sub.tsize.1.i, %if.end143.i ]
  %186 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 12), align 4
  %add147.i = add i32 %186, %tsize.290.i
  store i32 %add147.i, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 12), align 4
  %187 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 13), align 4
  %cmp148.i = icmp ugt i32 %add147.i, %187
  br i1 %cmp148.i, label %if.then149.i, label %if.end152.i165

if.then149.i:                                     ; preds = %if.then145.i
  store i32 %add147.i, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 13), align 4
  br label %if.end152.i165

if.end152.i165:                                   ; preds = %if.then149.i, %if.then145.i
  %188 = load %struct.malloc_chunk*, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 6), align 4
  %cmp154.i = icmp eq %struct.malloc_chunk* %188, null
  br i1 %cmp154.i, label %if.then155.i, label %land.rhs.i167

if.then155.i:                                     ; preds = %if.end152.i165
  %189 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp156.i = icmp eq i8* %189, null
  %cmp159.i166 = icmp ult i8* %tbase.291.i, %189
  %or.cond8.i = or i1 %cmp156.i, %cmp159.i166
  br i1 %or.cond8.i, label %if.then160.i, label %if.end162.i

if.then160.i:                                     ; preds = %if.then155.i
  store i8* %tbase.291.i, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  br label %if.end162.i

if.end162.i:                                      ; preds = %if.then160.i, %if.then155.i
  store i8* %tbase.291.i, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 16, i32 0), align 4
  store i32 %tsize.290.i, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 16, i32 1), align 4
  store i32 0, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 16, i32 3), align 4
  %190 = load i32, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 0), align 4
  store i32 %190, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 9), align 4
  store i32 -1, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 8), align 4
  br label %for.body.i.i

for.body.i.i:                                     ; preds = %for.body.i.i, %if.end162.i
  %i.02.i.i = phi i32 [ 0, %if.end162.i ], [ %inc.i.i, %for.body.i.i ]
  %shl.i.i = shl i32 %i.02.i.i, 1
  %arrayidx.i.i = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %shl.i.i
  %191 = bitcast %struct.malloc_chunk** %arrayidx.i.i to %struct.malloc_chunk*
  %arrayidx.sum.i.i = add i32 %shl.i.i, 3
  %192 = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %arrayidx.sum.i.i
  store %struct.malloc_chunk* %191, %struct.malloc_chunk** %192, align 4
  %arrayidx.sum1.i.i = add i32 %shl.i.i, 2
  %193 = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %arrayidx.sum1.i.i
  store %struct.malloc_chunk* %191, %struct.malloc_chunk** %193, align 4
  %inc.i.i = add i32 %i.02.i.i, 1
  %exitcond.i.i = icmp eq i32 %inc.i.i, 32
  br i1 %exitcond.i.i, label %init_bins.exit.i, label %for.body.i.i

init_bins.exit.i:                                 ; preds = %for.body.i.i
  %sub169.i = add i32 %tsize.290.i, -40
  %add.ptr.i11.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 8
  %194 = ptrtoint i8* %add.ptr.i11.i to i32
  %and.i12.i = and i32 %194, 7
  %cmp.i13.i = icmp eq i32 %and.i12.i, 0
  br i1 %cmp.i13.i, label %init_top.exit.i, label %cond.false.i.i

cond.false.i.i:                                   ; preds = %init_bins.exit.i
  %195 = sub i32 0, %194
  %and3.i.i = and i32 %195, 7
  br label %init_top.exit.i

init_top.exit.i:                                  ; preds = %cond.false.i.i, %init_bins.exit.i
  %cond.i.i = phi i32 [ %and3.i.i, %cond.false.i.i ], [ 0, %init_bins.exit.i ]
  %add.ptr4.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %cond.i.i
  %196 = bitcast i8* %add.ptr4.i.i to %struct.malloc_chunk*
  %sub5.i.i = sub i32 %sub169.i, %cond.i.i
  store %struct.malloc_chunk* %196, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 6), align 4
  store i32 %sub5.i.i, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 3), align 4
  %or.i.i = or i32 %sub5.i.i, 1
  %add.ptr4.sum.i.i = add i32 %cond.i.i, 4
  %head.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr4.sum.i.i
  %197 = bitcast i8* %head.i.i to i32*
  store i32 %or.i.i, i32* %197, align 4
  %add.ptr6.sum.i.i = add i32 %tsize.290.i, -36
  %head7.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr6.sum.i.i
  %198 = bitcast i8* %head7.i.i to i32*
  store i32 40, i32* %198, align 4
  %199 = load i32, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 4), align 4
  store i32 %199, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 7), align 4
  br label %if.end248.i

land.rhs.i167:                                    ; preds = %while.body.i168, %if.end152.i165
  %sp.0109.i = phi %struct.malloc_segment* [ %202, %while.body.i168 ], [ getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 16), %if.end152.i165 ]
  %base184.i = getelementptr inbounds %struct.malloc_segment, %struct.malloc_segment* %sp.0109.i, i32 0, i32 0
  %200 = load i8*, i8** %base184.i, align 4
  %size185.i = getelementptr inbounds %struct.malloc_segment, %struct.malloc_segment* %sp.0109.i, i32 0, i32 1
  %201 = load i32, i32* %size185.i, align 4
  %add.ptr186.i = getelementptr inbounds i8, i8* %200, i32 %201
  %cmp187.i = icmp eq i8* %tbase.291.i, %add.ptr186.i
  br i1 %cmp187.i, label %land.lhs.true189.i, label %while.body.i168

while.body.i168:                                  ; preds = %land.rhs.i167
  %next.i = getelementptr inbounds %struct.malloc_segment, %struct.malloc_segment* %sp.0109.i, i32 0, i32 2
  %202 = load %struct.malloc_segment*, %struct.malloc_segment** %next.i, align 4
  %cmp183.i = icmp eq %struct.malloc_segment* %202, null
  br i1 %cmp183.i, label %if.else213.i169, label %land.rhs.i167

land.lhs.true189.i:                               ; preds = %land.rhs.i167
  %sflags190.i = getelementptr inbounds %struct.malloc_segment, %struct.malloc_segment* %sp.0109.i, i32 0, i32 3
  %203 = load i32, i32* %sflags190.i, align 4
  %and191.i = and i32 %203, 8
  %tobool192.i = icmp eq i32 %and191.i, 0
  br i1 %tobool192.i, label %land.lhs.true197.i, label %if.else213.i169

land.lhs.true197.i:                               ; preds = %land.lhs.true189.i
  %204 = bitcast %struct.malloc_chunk* %188 to i8*
  %cmp200.i = icmp uge i8* %204, %200
  %cmp206.i = icmp ult i8* %204, %tbase.291.i
  %or.cond93.i = and i1 %cmp200.i, %cmp206.i
  br i1 %or.cond93.i, label %if.then207.i, label %if.else213.i169

if.then207.i:                                     ; preds = %land.lhs.true197.i
  %add209.i = add i32 %201, %tsize.290.i
  store i32 %add209.i, i32* %size185.i, align 4
  %205 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 3), align 4
  %add212.i = add i32 %205, %tsize.290.i
  %add.ptr.i22.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %188, i32 0, i32 2
  %206 = ptrtoint %struct.malloc_chunk** %add.ptr.i22.i to i32
  %and.i23.i = and i32 %206, 7
  %cmp.i24.i = icmp eq i32 %and.i23.i, 0
  br i1 %cmp.i24.i, label %init_top.exit36.i, label %cond.false.i26.i

cond.false.i26.i:                                 ; preds = %if.then207.i
  %207 = sub i32 0, %206
  %and3.i25.i = and i32 %207, 7
  br label %init_top.exit36.i

init_top.exit36.i:                                ; preds = %cond.false.i26.i, %if.then207.i
  %cond.i27.i = phi i32 [ %and3.i25.i, %cond.false.i26.i ], [ 0, %if.then207.i ]
  %add.ptr4.i28.i = getelementptr inbounds i8, i8* %204, i32 %cond.i27.i
  %208 = bitcast i8* %add.ptr4.i28.i to %struct.malloc_chunk*
  %sub5.i29.i = sub i32 %add212.i, %cond.i27.i
  store %struct.malloc_chunk* %208, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 6), align 4
  store i32 %sub5.i29.i, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 3), align 4
  %or.i30.i = or i32 %sub5.i29.i, 1
  %add.ptr4.sum.i31.i = add i32 %cond.i27.i, 4
  %head.i32.i = getelementptr inbounds i8, i8* %204, i32 %add.ptr4.sum.i31.i
  %209 = bitcast i8* %head.i32.i to i32*
  store i32 %or.i30.i, i32* %209, align 4
  %add.ptr6.sum.i33.i = add i32 %add212.i, 4
  %head7.i34.i = getelementptr inbounds i8, i8* %204, i32 %add.ptr6.sum.i33.i
  %210 = bitcast i8* %head7.i34.i to i32*
  store i32 40, i32* %210, align 4
  %211 = load i32, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 4), align 4
  store i32 %211, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 7), align 4
  br label %if.end248.i

if.else213.i169:                                  ; preds = %land.lhs.true197.i, %land.lhs.true189.i, %while.body.i168
  %212 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp215.i = icmp ult i8* %tbase.291.i, %212
  br i1 %cmp215.i, label %if.then216.i, label %while.cond220.preheader.i

if.then216.i:                                     ; preds = %if.else213.i169
  store i8* %tbase.291.i, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  br label %while.cond220.preheader.i

while.cond220.preheader.i:                        ; preds = %if.then216.i, %if.else213.i169
  %add.ptr224.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %tsize.290.i
  br label %land.rhs222.i

land.rhs222.i:                                    ; preds = %while.body227.i, %while.cond220.preheader.i
  %sp.1105.i = phi %struct.malloc_segment* [ getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 16), %while.cond220.preheader.i ], [ %214, %while.body227.i ]
  %base223.i = getelementptr inbounds %struct.malloc_segment, %struct.malloc_segment* %sp.1105.i, i32 0, i32 0
  %213 = load i8*, i8** %base223.i, align 4
  %cmp225.i = icmp eq i8* %213, %add.ptr224.i
  br i1 %cmp225.i, label %land.lhs.true231.i, label %while.body227.i

while.body227.i:                                  ; preds = %land.rhs222.i
  %next228.i = getelementptr inbounds %struct.malloc_segment, %struct.malloc_segment* %sp.1105.i, i32 0, i32 2
  %214 = load %struct.malloc_segment*, %struct.malloc_segment** %next228.i, align 4
  %cmp221.i = icmp eq %struct.malloc_segment* %214, null
  br i1 %cmp221.i, label %if.else245.i, label %land.rhs222.i

land.lhs.true231.i:                               ; preds = %land.rhs222.i
  %sflags232.i = getelementptr inbounds %struct.malloc_segment, %struct.malloc_segment* %sp.1105.i, i32 0, i32 3
  %215 = load i32, i32* %sflags232.i, align 4
  %and233.i = and i32 %215, 8
  %tobool234.i = icmp eq i32 %and233.i, 0
  br i1 %tobool234.i, label %if.then239.i, label %if.else245.i

if.then239.i:                                     ; preds = %land.lhs.true231.i
  store i8* %tbase.291.i, i8** %base223.i, align 4
  %size242.i = getelementptr inbounds %struct.malloc_segment, %struct.malloc_segment* %sp.1105.i, i32 0, i32 1
  %216 = load i32, i32* %size242.i, align 4
  %add243.i = add i32 %216, %tsize.290.i
  store i32 %add243.i, i32* %size242.i, align 4
  %add.ptr.i37.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 8
  %217 = ptrtoint i8* %add.ptr.i37.i to i32
  %and.i38.i = and i32 %217, 7
  %cmp.i39.i = icmp eq i32 %and.i38.i, 0
  br i1 %cmp.i39.i, label %cond.end.i45.i, label %cond.false.i41.i

cond.false.i41.i:                                 ; preds = %if.then239.i
  %218 = sub i32 0, %217
  %and3.i40.i = and i32 %218, 7
  br label %cond.end.i45.i

cond.end.i45.i:                                   ; preds = %cond.false.i41.i, %if.then239.i
  %cond.i42.i = phi i32 [ %and3.i40.i, %cond.false.i41.i ], [ 0, %if.then239.i ]
  %add.ptr4.i43.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %cond.i42.i
  %add.ptr224.sum.i = add i32 %tsize.290.i, 8
  %add.ptr5.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr224.sum.i
  %219 = ptrtoint i8* %add.ptr5.i.i to i32
  %and6.i44.i = and i32 %219, 7
  %cmp7.i.i = icmp eq i32 %and6.i44.i, 0
  br i1 %cmp7.i.i, label %cond.end14.i.i, label %cond.false9.i.i

cond.false9.i.i:                                  ; preds = %cond.end.i45.i
  %220 = sub i32 0, %219
  %and13.i.i = and i32 %220, 7
  br label %cond.end14.i.i

cond.end14.i.i:                                   ; preds = %cond.false9.i.i, %cond.end.i45.i
  %cond15.i.i = phi i32 [ %and13.i.i, %cond.false9.i.i ], [ 0, %cond.end.i45.i ]
  %add.ptr224.sum131.i = add i32 %cond15.i.i, %tsize.290.i
  %add.ptr16.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr224.sum131.i
  %221 = bitcast i8* %add.ptr16.i.i to %struct.malloc_chunk*
  %sub.ptr.lhs.cast.i46.i = ptrtoint i8* %add.ptr16.i.i to i32
  %sub.ptr.rhs.cast.i47.i = ptrtoint i8* %add.ptr4.i43.i to i32
  %sub.ptr.sub.i48.i = sub i32 %sub.ptr.lhs.cast.i46.i, %sub.ptr.rhs.cast.i47.i
  %add.ptr4.sum.i49.i = add i32 %cond.i42.i, %nb.0
  %add.ptr17.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr4.sum.i49.i
  %222 = bitcast i8* %add.ptr17.i.i to %struct.malloc_chunk*
  %sub18.i.i = sub i32 %sub.ptr.sub.i48.i, %nb.0
  %or19.i.i = or i32 %nb.0, 3
  %add.ptr4.sum1.i.i = add i32 %cond.i42.i, 4
  %head.i50.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr4.sum1.i.i
  %223 = bitcast i8* %head.i50.i to i32*
  store i32 %or19.i.i, i32* %223, align 4
  %224 = load %struct.malloc_chunk*, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 6), align 4
  %cmp20.i.i = icmp eq %struct.malloc_chunk* %221, %224
  br i1 %cmp20.i.i, label %if.then.i51.i, label %if.else.i.i

if.then.i51.i:                                    ; preds = %cond.end14.i.i
  %225 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 3), align 4
  %add.i.i = add i32 %225, %sub18.i.i
  store i32 %add.i.i, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 3), align 4
  store %struct.malloc_chunk* %222, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 6), align 4
  %or22.i.i = or i32 %add.i.i, 1
  %add.ptr17.sum35.i.i = add i32 %add.ptr4.sum.i49.i, 4
  %head23.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr17.sum35.i.i
  %226 = bitcast i8* %head23.i.i to i32*
  store i32 %or22.i.i, i32* %226, align 4
  br label %prepend_alloc.exit.i

if.else.i.i:                                      ; preds = %cond.end14.i.i
  %227 = load %struct.malloc_chunk*, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 5), align 4
  %cmp24.i.i = icmp eq %struct.malloc_chunk* %221, %227
  br i1 %cmp24.i.i, label %if.then25.i.i, label %if.else31.i.i

if.then25.i.i:                                    ; preds = %if.else.i.i
  %228 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 2), align 4
  %add26.i.i = add i32 %228, %sub18.i.i
  store i32 %add26.i.i, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 2), align 4
  store %struct.malloc_chunk* %222, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 5), align 4
  %or28.i.i = or i32 %add26.i.i, 1
  %add.ptr17.sum33.i.i = add i32 %add.ptr4.sum.i49.i, 4
  %head29.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr17.sum33.i.i
  %229 = bitcast i8* %head29.i.i to i32*
  store i32 %or28.i.i, i32* %229, align 4
  %add.ptr17.sum34.i.i = add i32 %add26.i.i, %add.ptr4.sum.i49.i
  %add.ptr30.i52.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr17.sum34.i.i
  %prev_foot.i53.i = bitcast i8* %add.ptr30.i52.i to i32*
  store i32 %add26.i.i, i32* %prev_foot.i53.i, align 4
  br label %prepend_alloc.exit.i

if.else31.i.i:                                    ; preds = %if.else.i.i
  %add.ptr16.sum.i.i = add i32 %tsize.290.i, 4
  %add.ptr224.sum132.i = add i32 %add.ptr16.sum.i.i, %cond15.i.i
  %head32.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr224.sum132.i
  %230 = bitcast i8* %head32.i.i to i32*
  %231 = load i32, i32* %230, align 4
  %and33.i.i = and i32 %231, 3
  %cmp34.i.i = icmp eq i32 %and33.i.i, 1
  br i1 %cmp34.i.i, label %if.then35.i.i, label %if.end207.i.i

if.then35.i.i:                                    ; preds = %if.else31.i.i
  %and37.i.i = and i32 %231, -8
  %shr.i54.i = lshr i32 %231, 3
  %cmp38.i.i = icmp ult i32 %231, 256
  br i1 %cmp38.i.i, label %if.then39.i.i, label %if.else73.i.i

if.then39.i.i:                                    ; preds = %if.then35.i.i
  %add.ptr16.sum3031.i.i = or i32 %cond15.i.i, 8
  %add.ptr224.sum142.i = add i32 %add.ptr16.sum3031.i.i, %tsize.290.i
  %fd.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr224.sum142.i
  %232 = bitcast i8* %fd.i.i to %struct.malloc_chunk**
  %233 = load %struct.malloc_chunk*, %struct.malloc_chunk** %232, align 4
  %add.ptr16.sum32.i.i = add i32 %tsize.290.i, 12
  %add.ptr224.sum143.i = add i32 %add.ptr16.sum32.i.i, %cond15.i.i
  %bk.i55.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr224.sum143.i
  %234 = bitcast i8* %bk.i55.i to %struct.malloc_chunk**
  %235 = load %struct.malloc_chunk*, %struct.malloc_chunk** %234, align 4
  %shl.i56.i = shl nuw nsw i32 %shr.i54.i, 1
  %arrayidx.i57.i = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %shl.i56.i
  %236 = bitcast %struct.malloc_chunk** %arrayidx.i57.i to %struct.malloc_chunk*
  %cmp41.i.i = icmp eq %struct.malloc_chunk* %233, %236
  br i1 %cmp41.i.i, label %if.then45.i.i, label %lor.rhs.i.i

lor.rhs.i.i:                                      ; preds = %if.then39.i.i
  %237 = bitcast %struct.malloc_chunk* %233 to i8*
  %238 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp42.i.i = icmp ult i8* %237, %238
  br i1 %cmp42.i.i, label %if.else71.i.i, label %land.rhs.i58.i

land.rhs.i58.i:                                   ; preds = %lor.rhs.i.i
  %bk43.i.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %233, i32 0, i32 3
  %239 = load %struct.malloc_chunk*, %struct.malloc_chunk** %bk43.i.i, align 4
  %cmp44.i.i = icmp eq %struct.malloc_chunk* %239, %221
  br i1 %cmp44.i.i, label %if.then45.i.i, label %if.else71.i.i

if.then45.i.i:                                    ; preds = %land.rhs.i58.i, %if.then39.i.i
  %cmp46.i59.i = icmp eq %struct.malloc_chunk* %235, %233
  br i1 %cmp46.i59.i, label %if.then47.i.i, label %if.else50.i60.i

if.then47.i.i:                                    ; preds = %if.then45.i.i
  %shl48.i.i = shl i32 1, %shr.i54.i
  %neg.i.i = xor i32 %shl48.i.i, -1
  %240 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  %and49.i.i = and i32 %240, %neg.i.i
  store i32 %and49.i.i, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  br label %if.end204.i.i

if.else50.i60.i:                                  ; preds = %if.then45.i.i
  %cmp54.i.i = icmp eq %struct.malloc_chunk* %235, %236
  br i1 %cmp54.i.i, label %if.else50.if.then66_crit_edge.i.i, label %lor.rhs55.i.i

if.else50.if.then66_crit_edge.i.i:                ; preds = %if.else50.i60.i
  %fd68.pre.i.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %235, i32 0, i32 2
  br label %if.then66.i.i

lor.rhs55.i.i:                                    ; preds = %if.else50.i60.i
  %241 = bitcast %struct.malloc_chunk* %235 to i8*
  %242 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp57.i.i = icmp ult i8* %241, %242
  br i1 %cmp57.i.i, label %if.else69.i.i, label %land.rhs58.i.i

land.rhs58.i.i:                                   ; preds = %lor.rhs55.i.i
  %fd59.i.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %235, i32 0, i32 2
  %243 = load %struct.malloc_chunk*, %struct.malloc_chunk** %fd59.i.i, align 4
  %cmp60.i.i = icmp eq %struct.malloc_chunk* %243, %221
  br i1 %cmp60.i.i, label %if.then66.i.i, label %if.else69.i.i

if.then66.i.i:                                    ; preds = %land.rhs58.i.i, %if.else50.if.then66_crit_edge.i.i
  %fd68.pre-phi.i.i = phi %struct.malloc_chunk** [ %fd68.pre.i.i, %if.else50.if.then66_crit_edge.i.i ], [ %fd59.i.i, %land.rhs58.i.i ]
  %bk67.i.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %233, i32 0, i32 3
  store %struct.malloc_chunk* %235, %struct.malloc_chunk** %bk67.i.i, align 4
  store %struct.malloc_chunk* %233, %struct.malloc_chunk** %fd68.pre-phi.i.i, align 4
  br label %if.end204.i.i

if.else69.i.i:                                    ; preds = %land.rhs58.i.i, %lor.rhs55.i.i
  tail call void @abort() #6
  unreachable

if.else71.i.i:                                    ; preds = %land.rhs.i58.i, %lor.rhs.i.i
  tail call void @abort() #6
  unreachable

if.else73.i.i:                                    ; preds = %if.then35.i.i
  %244 = bitcast i8* %add.ptr16.i.i to %struct.malloc_tree_chunk*
  %add.ptr16.sum23.i.i = or i32 %cond15.i.i, 24
  %add.ptr224.sum133.i = add i32 %add.ptr16.sum23.i.i, %tsize.290.i
  %parent.i61.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr224.sum133.i
  %245 = bitcast i8* %parent.i61.i to %struct.malloc_tree_chunk**
  %246 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %245, align 4
  %add.ptr16.sum4.i.i = add i32 %tsize.290.i, 12
  %add.ptr224.sum134.i = add i32 %add.ptr16.sum4.i.i, %cond15.i.i
  %bk74.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr224.sum134.i
  %247 = bitcast i8* %bk74.i.i to %struct.malloc_tree_chunk**
  %248 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %247, align 4
  %cmp75.i.i = icmp eq %struct.malloc_tree_chunk* %248, %244
  br i1 %cmp75.i.i, label %if.else95.i.i, label %if.then76.i.i

if.then76.i.i:                                    ; preds = %if.else73.i.i
  %add.ptr16.sum2829.i.i = or i32 %cond15.i.i, 8
  %add.ptr224.sum135.i = add i32 %add.ptr16.sum2829.i.i, %tsize.290.i
  %fd78.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr224.sum135.i
  %249 = bitcast i8* %fd78.i.i to %struct.malloc_tree_chunk**
  %250 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %249, align 4
  %251 = bitcast %struct.malloc_tree_chunk* %250 to i8*
  %252 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp81.i.i = icmp ult i8* %251, %252
  br i1 %cmp81.i.i, label %if.else93.i.i, label %land.lhs.true.i62.i

land.lhs.true.i62.i:                              ; preds = %if.then76.i.i
  %bk82.i.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %250, i32 0, i32 3
  %253 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %bk82.i.i, align 4
  %cmp83.i.i = icmp eq %struct.malloc_tree_chunk* %253, %244
  br i1 %cmp83.i.i, label %land.rhs84.i.i, label %if.else93.i.i

land.rhs84.i.i:                                   ; preds = %land.lhs.true.i62.i
  %fd85.i.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %248, i32 0, i32 2
  %254 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %fd85.i.i, align 4
  %cmp86.i.i = icmp eq %struct.malloc_tree_chunk* %254, %244
  br i1 %cmp86.i.i, label %if.then90.i.i, label %if.else93.i.i

if.then90.i.i:                                    ; preds = %land.rhs84.i.i
  store %struct.malloc_tree_chunk* %248, %struct.malloc_tree_chunk** %bk82.i.i, align 4
  store %struct.malloc_tree_chunk* %250, %struct.malloc_tree_chunk** %fd85.i.i, align 4
  br label %if.end119.i.i

if.else93.i.i:                                    ; preds = %land.rhs84.i.i, %land.lhs.true.i62.i, %if.then76.i.i
  tail call void @abort() #6
  unreachable

if.else95.i.i:                                    ; preds = %if.else73.i.i
  %add.ptr16.sum56.i.i = or i32 %cond15.i.i, 16
  %add.ptr224.sum140.i = add i32 %add.ptr16.sum.i.i, %add.ptr16.sum56.i.i
  %arrayidx96.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr224.sum140.i
  %255 = bitcast i8* %arrayidx96.i.i to %struct.malloc_tree_chunk**
  %256 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %255, align 4
  %cmp97.i.i = icmp eq %struct.malloc_tree_chunk* %256, null
  br i1 %cmp97.i.i, label %lor.lhs.false.i.i, label %while.cond.i.i

lor.lhs.false.i.i:                                ; preds = %if.else95.i.i
  %add.ptr224.sum141.i = add i32 %add.ptr16.sum56.i.i, %tsize.290.i
  %child.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr224.sum141.i
  %arrayidx99.i.i = bitcast i8* %child.i.i to %struct.malloc_tree_chunk**
  %257 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx99.i.i, align 4
  %cmp100.i.i = icmp eq %struct.malloc_tree_chunk* %257, null
  br i1 %cmp100.i.i, label %if.end119.i.i, label %while.cond.i.i

while.cond.i.i:                                   ; preds = %lor.rhs105.i.i, %while.cond.i.i, %lor.lhs.false.i.i, %if.else95.i.i
  %RP.0.i.i = phi %struct.malloc_tree_chunk** [ %arrayidx99.i.i, %lor.lhs.false.i.i ], [ %255, %if.else95.i.i ], [ %arrayidx103.i.i, %while.cond.i.i ], [ %arrayidx107.i.i, %lor.rhs105.i.i ]
  %R.0.i.i = phi %struct.malloc_tree_chunk* [ %257, %lor.lhs.false.i.i ], [ %256, %if.else95.i.i ], [ %258, %while.cond.i.i ], [ %259, %lor.rhs105.i.i ]
  %arrayidx103.i.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.0.i.i, i32 0, i32 4, i32 1
  %258 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx103.i.i, align 4
  %cmp104.i.i = icmp eq %struct.malloc_tree_chunk* %258, null
  br i1 %cmp104.i.i, label %lor.rhs105.i.i, label %while.cond.i.i

lor.rhs105.i.i:                                   ; preds = %while.cond.i.i
  %arrayidx107.i.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.0.i.i, i32 0, i32 4, i32 0
  %259 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx107.i.i, align 4
  %cmp108.i.i = icmp eq %struct.malloc_tree_chunk* %259, null
  br i1 %cmp108.i.i, label %while.end.i.i, label %while.cond.i.i

while.end.i.i:                                    ; preds = %lor.rhs105.i.i
  %260 = bitcast %struct.malloc_tree_chunk** %RP.0.i.i to i8*
  %261 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp112.i.i = icmp ult i8* %260, %261
  br i1 %cmp112.i.i, label %if.else116.i.i, label %if.then115.i.i

if.then115.i.i:                                   ; preds = %while.end.i.i
  store %struct.malloc_tree_chunk* null, %struct.malloc_tree_chunk** %RP.0.i.i, align 4
  br label %if.end119.i.i

if.else116.i.i:                                   ; preds = %while.end.i.i
  tail call void @abort() #6
  unreachable

if.end119.i.i:                                    ; preds = %if.then115.i.i, %lor.lhs.false.i.i, %if.then90.i.i
  %R.1.i.i = phi %struct.malloc_tree_chunk* [ %248, %if.then90.i.i ], [ %R.0.i.i, %if.then115.i.i ], [ null, %lor.lhs.false.i.i ]
  %cmp120.i63.i = icmp eq %struct.malloc_tree_chunk* %246, null
  br i1 %cmp120.i63.i, label %if.end204.i.i, label %if.then122.i65.i

if.then122.i65.i:                                 ; preds = %if.end119.i.i
  %add.ptr16.sum25.i.i = add i32 %tsize.290.i, 28
  %add.ptr224.sum136.i = add i32 %add.ptr16.sum25.i.i, %cond15.i.i
  %index.i64.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr224.sum136.i
  %262 = bitcast i8* %index.i64.i to i32*
  %263 = load i32, i32* %262, align 4
  %arrayidx123.i.i = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 11, i32 %263
  %264 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx123.i.i, align 4
  %cmp124.i.i = icmp eq %struct.malloc_tree_chunk* %244, %264
  br i1 %cmp124.i.i, label %if.then126.i.i, label %if.else135.i.i

if.then126.i.i:                                   ; preds = %if.then122.i65.i
  store %struct.malloc_tree_chunk* %R.1.i.i, %struct.malloc_tree_chunk** %arrayidx123.i.i, align 4
  %cond37.i.i = icmp eq %struct.malloc_tree_chunk* %R.1.i.i, null
  br i1 %cond37.i.i, label %if.end155.thread.i.i, label %if.then158.i.i

if.end155.thread.i.i:                             ; preds = %if.then126.i.i
  %shl131.i.i = shl i32 1, %263
  %neg132.i.i = xor i32 %shl131.i.i, -1
  %265 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 1), align 4
  %and133.i.i = and i32 %265, %neg132.i.i
  store i32 %and133.i.i, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 1), align 4
  br label %if.end204.i.i

if.else135.i.i:                                   ; preds = %if.then122.i65.i
  %266 = bitcast %struct.malloc_tree_chunk* %246 to i8*
  %267 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp137.i.i = icmp ult i8* %266, %267
  br i1 %cmp137.i.i, label %if.else153.i.i, label %if.then141.i.i

if.then141.i.i:                                   ; preds = %if.else135.i.i
  %arrayidx143.i.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %246, i32 0, i32 4, i32 0
  %268 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx143.i.i, align 4
  %cmp144.i.i = icmp eq %struct.malloc_tree_chunk* %268, %244
  br i1 %cmp144.i.i, label %if.then146.i.i, label %if.else149.i.i

if.then146.i.i:                                   ; preds = %if.then141.i.i
  store %struct.malloc_tree_chunk* %R.1.i.i, %struct.malloc_tree_chunk** %arrayidx143.i.i, align 4
  br label %if.end155.i.i

if.else149.i.i:                                   ; preds = %if.then141.i.i
  %arrayidx151.i.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %246, i32 0, i32 4, i32 1
  store %struct.malloc_tree_chunk* %R.1.i.i, %struct.malloc_tree_chunk** %arrayidx151.i.i, align 4
  br label %if.end155.i.i

if.else153.i.i:                                   ; preds = %if.else135.i.i
  tail call void @abort() #6
  unreachable

if.end155.i.i:                                    ; preds = %if.else149.i.i, %if.then146.i.i
  %cmp156.i.i = icmp eq %struct.malloc_tree_chunk* %R.1.i.i, null
  br i1 %cmp156.i.i, label %if.end204.i.i, label %if.then158.i.i

if.then158.i.i:                                   ; preds = %if.end155.i.i, %if.then126.i.i
  %269 = bitcast %struct.malloc_tree_chunk* %R.1.i.i to i8*
  %270 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp160.i.i = icmp ult i8* %269, %270
  br i1 %cmp160.i.i, label %if.else200.i.i, label %if.then164.i.i

if.then164.i.i:                                   ; preds = %if.then158.i.i
  %parent165.i.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.1.i.i, i32 0, i32 5
  store %struct.malloc_tree_chunk* %246, %struct.malloc_tree_chunk** %parent165.i.i, align 4
  %add.ptr16.sum2627.i.i = or i32 %cond15.i.i, 16
  %add.ptr224.sum137.i = add i32 %add.ptr16.sum2627.i.i, %tsize.290.i
  %child166.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr224.sum137.i
  %arrayidx167.i.i = bitcast i8* %child166.i.i to %struct.malloc_tree_chunk**
  %271 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx167.i.i, align 4
  %cmp168.i.i = icmp eq %struct.malloc_tree_chunk* %271, null
  br i1 %cmp168.i.i, label %if.end182.i.i, label %if.then170.i.i

if.then170.i.i:                                   ; preds = %if.then164.i.i
  %272 = bitcast %struct.malloc_tree_chunk* %271 to i8*
  %273 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp172.i.i = icmp ult i8* %272, %273
  br i1 %cmp172.i.i, label %if.else180.i.i, label %if.then176.i.i

if.then176.i.i:                                   ; preds = %if.then170.i.i
  %arrayidx178.i.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.1.i.i, i32 0, i32 4, i32 0
  store %struct.malloc_tree_chunk* %271, %struct.malloc_tree_chunk** %arrayidx178.i.i, align 4
  %parent179.i.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %271, i32 0, i32 5
  store %struct.malloc_tree_chunk* %R.1.i.i, %struct.malloc_tree_chunk** %parent179.i.i, align 4
  br label %if.end182.i.i

if.else180.i.i:                                   ; preds = %if.then170.i.i
  tail call void @abort() #6
  unreachable

if.end182.i.i:                                    ; preds = %if.then176.i.i, %if.then164.i.i
  %add.ptr224.sum138.i = add i32 %add.ptr16.sum.i.i, %add.ptr16.sum2627.i.i
  %arrayidx184.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr224.sum138.i
  %274 = bitcast i8* %arrayidx184.i.i to %struct.malloc_tree_chunk**
  %275 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %274, align 4
  %cmp185.i.i = icmp eq %struct.malloc_tree_chunk* %275, null
  br i1 %cmp185.i.i, label %if.end204.i.i, label %if.then187.i.i

if.then187.i.i:                                   ; preds = %if.end182.i.i
  %276 = bitcast %struct.malloc_tree_chunk* %275 to i8*
  %277 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp189.i.i = icmp ult i8* %276, %277
  br i1 %cmp189.i.i, label %if.else197.i.i, label %if.then193.i.i

if.then193.i.i:                                   ; preds = %if.then187.i.i
  %arrayidx195.i.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.1.i.i, i32 0, i32 4, i32 1
  store %struct.malloc_tree_chunk* %275, %struct.malloc_tree_chunk** %arrayidx195.i.i, align 4
  %parent196.i.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %275, i32 0, i32 5
  store %struct.malloc_tree_chunk* %R.1.i.i, %struct.malloc_tree_chunk** %parent196.i.i, align 4
  br label %if.end204.i.i

if.else197.i.i:                                   ; preds = %if.then187.i.i
  tail call void @abort() #6
  unreachable

if.else200.i.i:                                   ; preds = %if.then158.i.i
  tail call void @abort() #6
  unreachable

if.end204.i.i:                                    ; preds = %if.then193.i.i, %if.end182.i.i, %if.end155.i.i, %if.end155.thread.i.i, %if.end119.i.i, %if.then66.i.i, %if.then47.i.i
  %add.ptr16.sum7.i.i = or i32 %and37.i.i, %cond15.i.i
  %add.ptr224.sum139.i = add i32 %add.ptr16.sum7.i.i, %tsize.290.i
  %add.ptr205.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr224.sum139.i
  %278 = bitcast i8* %add.ptr205.i.i to %struct.malloc_chunk*
  %add206.i.i = add i32 %and37.i.i, %sub18.i.i
  br label %if.end207.i.i

if.end207.i.i:                                    ; preds = %if.end204.i.i, %if.else31.i.i
  %qsize.0.i.i = phi i32 [ %sub18.i.i, %if.else31.i.i ], [ %add206.i.i, %if.end204.i.i ]
  %oldfirst.0.i.i = phi %struct.malloc_chunk* [ %221, %if.else31.i.i ], [ %278, %if.end204.i.i ]
  %head208.i.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %oldfirst.0.i.i, i32 0, i32 1
  %279 = load i32, i32* %head208.i.i, align 4
  %and209.i.i = and i32 %279, -2
  store i32 %and209.i.i, i32* %head208.i.i, align 4
  %or210.i.i = or i32 %qsize.0.i.i, 1
  %add.ptr17.sum.i.i = add i32 %add.ptr4.sum.i49.i, 4
  %head211.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr17.sum.i.i
  %280 = bitcast i8* %head211.i.i to i32*
  store i32 %or210.i.i, i32* %280, align 4
  %add.ptr17.sum8.i.i = add i32 %qsize.0.i.i, %add.ptr4.sum.i49.i
  %add.ptr212.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr17.sum8.i.i
  %prev_foot213.i.i = bitcast i8* %add.ptr212.i.i to i32*
  store i32 %qsize.0.i.i, i32* %prev_foot213.i.i, align 4
  %shr214.i.i = lshr i32 %qsize.0.i.i, 3
  %cmp215.i.i = icmp ult i32 %qsize.0.i.i, 256
  br i1 %cmp215.i.i, label %if.then217.i.i, label %if.else249.i.i

if.then217.i.i:                                   ; preds = %if.end207.i.i
  %shl221.i.i = shl nuw nsw i32 %shr214.i.i, 1
  %arrayidx223.i.i = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %shl221.i.i
  %281 = bitcast %struct.malloc_chunk** %arrayidx223.i.i to %struct.malloc_chunk*
  %282 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  %shl226.i.i = shl i32 1, %shr214.i.i
  %and227.i.i = and i32 %282, %shl226.i.i
  %tobool228.i.i = icmp eq i32 %and227.i.i, 0
  br i1 %tobool228.i.i, label %if.then229.i.i, label %if.else233.i.i

if.then229.i.i:                                   ; preds = %if.then217.i.i
  %or232.i.i = or i32 %282, %shl226.i.i
  store i32 %or232.i.i, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  %arrayidx223.sum.pre.i.i = add i32 %shl221.i.i, 2
  %.pre.i66.i = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %arrayidx223.sum.pre.i.i
  br label %if.end244.i.i

if.else233.i.i:                                   ; preds = %if.then217.i.i
  %arrayidx223.sum24.i.i = add i32 %shl221.i.i, 2
  %283 = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %arrayidx223.sum24.i.i
  %284 = load %struct.malloc_chunk*, %struct.malloc_chunk** %283, align 4
  %285 = bitcast %struct.malloc_chunk* %284 to i8*
  %286 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp236.i.i = icmp ult i8* %285, %286
  br i1 %cmp236.i.i, label %if.else242.i.i, label %if.end244.i.i

if.else242.i.i:                                   ; preds = %if.else233.i.i
  tail call void @abort() #6
  unreachable

if.end244.i.i:                                    ; preds = %if.else233.i.i, %if.then229.i.i
  %.pre-phi.i67.i = phi %struct.malloc_chunk** [ %283, %if.else233.i.i ], [ %.pre.i66.i, %if.then229.i.i ]
  %F224.0.i.i = phi %struct.malloc_chunk* [ %284, %if.else233.i.i ], [ %281, %if.then229.i.i ]
  store %struct.malloc_chunk* %222, %struct.malloc_chunk** %.pre-phi.i67.i, align 4
  %bk246.i.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %F224.0.i.i, i32 0, i32 3
  store %struct.malloc_chunk* %222, %struct.malloc_chunk** %bk246.i.i, align 4
  %add.ptr17.sum22.i.i = add i32 %add.ptr4.sum.i49.i, 8
  %fd247.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr17.sum22.i.i
  %287 = bitcast i8* %fd247.i.i to %struct.malloc_chunk**
  store %struct.malloc_chunk* %F224.0.i.i, %struct.malloc_chunk** %287, align 4
  %add.ptr17.sum23.i.i = add i32 %add.ptr4.sum.i49.i, 12
  %bk248.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr17.sum23.i.i
  %288 = bitcast i8* %bk248.i.i to %struct.malloc_chunk**
  store %struct.malloc_chunk* %281, %struct.malloc_chunk** %288, align 4
  br label %prepend_alloc.exit.i

if.else249.i.i:                                   ; preds = %if.end207.i.i
  %289 = bitcast i8* %add.ptr17.i.i to %struct.malloc_tree_chunk*
  %shr253.i.i = lshr i32 %qsize.0.i.i, 8
  %cmp254.i.i = icmp eq i32 %shr253.i.i, 0
  br i1 %cmp254.i.i, label %if.end285.i.i, label %if.else257.i.i

if.else257.i.i:                                   ; preds = %if.else249.i.i
  %cmp258.i.i = icmp ugt i32 %qsize.0.i.i, 16777215
  br i1 %cmp258.i.i, label %if.end285.i.i, label %if.else261.i.i

if.else261.i.i:                                   ; preds = %if.else257.i.i
  %sub262.i.i = add i32 %shr253.i.i, 1048320
  %shr263.i.i = lshr i32 %sub262.i.i, 16
  %and264.i.i = and i32 %shr263.i.i, 8
  %shl265.i.i = shl i32 %shr253.i.i, %and264.i.i
  %sub266.i.i = add i32 %shl265.i.i, 520192
  %shr267.i.i = lshr i32 %sub266.i.i, 16
  %and268.i.i = and i32 %shr267.i.i, 4
  %add269.i.i = or i32 %and268.i.i, %and264.i.i
  %shl270.i.i = shl i32 %shl265.i.i, %and268.i.i
  %sub271.i.i = add i32 %shl270.i.i, 245760
  %shr272.i.i = lshr i32 %sub271.i.i, 16
  %and273.i.i = and i32 %shr272.i.i, 2
  %add274.i.i = or i32 %add269.i.i, %and273.i.i
  %sub275.i.i = sub i32 14, %add274.i.i
  %shl276.i.i = shl i32 %shl270.i.i, %and273.i.i
  %shr277.i.i = lshr i32 %shl276.i.i, 15
  %add278.i.i = add i32 %sub275.i.i, %shr277.i.i
  %shl279.i.i = shl nsw i32 %add278.i.i, 1
  %add280.i.i = add i32 %add278.i.i, 7
  %shr281.i.i = lshr i32 %qsize.0.i.i, %add280.i.i
  %and282.i.i = and i32 %shr281.i.i, 1
  %add283.i.i = or i32 %and282.i.i, %shl279.i.i
  br label %if.end285.i.i

if.end285.i.i:                                    ; preds = %if.else261.i.i, %if.else257.i.i, %if.else249.i.i
  %I252.0.i.i = phi i32 [ %add283.i.i, %if.else261.i.i ], [ 0, %if.else249.i.i ], [ 31, %if.else257.i.i ]
  %arrayidx287.i.i = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 11, i32 %I252.0.i.i
  %add.ptr17.sum9.i.i = add i32 %add.ptr4.sum.i49.i, 28
  %index288.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr17.sum9.i.i
  %290 = bitcast i8* %index288.i.i to i32*
  store i32 %I252.0.i.i, i32* %290, align 4
  %add.ptr17.sum10.i.i = add i32 %add.ptr4.sum.i49.i, 16
  %child289.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr17.sum10.i.i
  %child289.sum.i.i = add i32 %add.ptr4.sum.i49.i, 20
  %arrayidx290.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %child289.sum.i.i
  %291 = bitcast i8* %arrayidx290.i.i to %struct.malloc_tree_chunk**
  store %struct.malloc_tree_chunk* null, %struct.malloc_tree_chunk** %291, align 4
  %arrayidx292.i.i = bitcast i8* %child289.i.i to %struct.malloc_tree_chunk**
  store %struct.malloc_tree_chunk* null, %struct.malloc_tree_chunk** %arrayidx292.i.i, align 4
  %292 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 1), align 4
  %shl294.i.i = shl i32 1, %I252.0.i.i
  %and295.i.i = and i32 %292, %shl294.i.i
  %tobool296.i.i = icmp eq i32 %and295.i.i, 0
  br i1 %tobool296.i.i, label %if.then297.i.i, label %if.else304.i.i

if.then297.i.i:                                   ; preds = %if.end285.i.i
  %or300.i.i = or i32 %292, %shl294.i.i
  store i32 %or300.i.i, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 1), align 4
  store %struct.malloc_tree_chunk* %289, %struct.malloc_tree_chunk** %arrayidx287.i.i, align 4
  %293 = bitcast %struct.malloc_tree_chunk** %arrayidx287.i.i to %struct.malloc_tree_chunk*
  %add.ptr17.sum11.i.i = add i32 %add.ptr4.sum.i49.i, 24
  %parent301.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr17.sum11.i.i
  %294 = bitcast i8* %parent301.i.i to %struct.malloc_tree_chunk**
  store %struct.malloc_tree_chunk* %293, %struct.malloc_tree_chunk** %294, align 4
  %add.ptr17.sum12.i.i = add i32 %add.ptr4.sum.i49.i, 12
  %bk302.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr17.sum12.i.i
  %295 = bitcast i8* %bk302.i.i to %struct.malloc_tree_chunk**
  store %struct.malloc_tree_chunk* %289, %struct.malloc_tree_chunk** %295, align 4
  %add.ptr17.sum13.i.i = add i32 %add.ptr4.sum.i49.i, 8
  %fd303.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr17.sum13.i.i
  %296 = bitcast i8* %fd303.i.i to %struct.malloc_tree_chunk**
  store %struct.malloc_tree_chunk* %289, %struct.malloc_tree_chunk** %296, align 4
  br label %prepend_alloc.exit.i

if.else304.i.i:                                   ; preds = %if.end285.i.i
  %297 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx287.i.i, align 4
  %cmp306.i.i = icmp eq i32 %I252.0.i.i, 31
  br i1 %cmp306.i.i, label %cond.end314.i.i, label %cond.false309.i.i

cond.false309.i.i:                                ; preds = %if.else304.i.i
  %shr310.i.i = lshr i32 %I252.0.i.i, 1
  %sub313.i.i = sub i32 25, %shr310.i.i
  br label %cond.end314.i.i

cond.end314.i.i:                                  ; preds = %cond.false309.i.i, %if.else304.i.i
  %cond315.i.i = phi i32 [ %sub313.i.i, %cond.false309.i.i ], [ 0, %if.else304.i.i ]
  %head31739.i.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %297, i32 0, i32 1
  %298 = load i32, i32* %head31739.i.i, align 4
  %and31840.i.i = and i32 %298, -8
  %cmp31941.i.i = icmp eq i32 %and31840.i.i, %qsize.0.i.i
  br i1 %cmp31941.i.i, label %if.else342.i.i, label %if.then321.lr.ph.i.i

if.then321.lr.ph.i.i:                             ; preds = %cond.end314.i.i
  %shl316.i.i = shl i32 %qsize.0.i.i, %cond315.i.i
  br label %if.then321.i.i

for.cond.i68.i:                                   ; preds = %if.then321.i.i
  %shl326.i.i = shl i32 %K305.043.i.i, 1
  %head317.i.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %300, i32 0, i32 1
  %299 = load i32, i32* %head317.i.i, align 4
  %and318.i.i = and i32 %299, -8
  %cmp319.i.i = icmp eq i32 %and318.i.i, %qsize.0.i.i
  br i1 %cmp319.i.i, label %if.else342.i.i, label %if.then321.i.i

if.then321.i.i:                                   ; preds = %for.cond.i68.i, %if.then321.lr.ph.i.i
  %K305.043.i.i = phi i32 [ %shl316.i.i, %if.then321.lr.ph.i.i ], [ %shl326.i.i, %for.cond.i68.i ]
  %T.042.i.i = phi %struct.malloc_tree_chunk* [ %297, %if.then321.lr.ph.i.i ], [ %300, %for.cond.i68.i ]
  %shr322.i.i = lshr i32 %K305.043.i.i, 31
  %arrayidx325.i.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %T.042.i.i, i32 0, i32 4, i32 %shr322.i.i
  %300 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx325.i.i, align 4
  %cmp327.i.i = icmp eq %struct.malloc_tree_chunk* %300, null
  br i1 %cmp327.i.i, label %if.else330.i.i, label %for.cond.i68.i

if.else330.i.i:                                   ; preds = %if.then321.i.i
  %301 = bitcast %struct.malloc_tree_chunk** %arrayidx325.i.i to i8*
  %302 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp332.i.i = icmp ult i8* %301, %302
  br i1 %cmp332.i.i, label %if.else340.i.i, label %if.then336.i.i

if.then336.i.i:                                   ; preds = %if.else330.i.i
  store %struct.malloc_tree_chunk* %289, %struct.malloc_tree_chunk** %arrayidx325.i.i, align 4
  %add.ptr17.sum19.i.i = add i32 %add.ptr4.sum.i49.i, 24
  %parent337.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr17.sum19.i.i
  %303 = bitcast i8* %parent337.i.i to %struct.malloc_tree_chunk**
  store %struct.malloc_tree_chunk* %T.042.i.i, %struct.malloc_tree_chunk** %303, align 4
  %add.ptr17.sum20.i.i = add i32 %add.ptr4.sum.i49.i, 12
  %bk338.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr17.sum20.i.i
  %304 = bitcast i8* %bk338.i.i to %struct.malloc_tree_chunk**
  store %struct.malloc_tree_chunk* %289, %struct.malloc_tree_chunk** %304, align 4
  %add.ptr17.sum21.i.i = add i32 %add.ptr4.sum.i49.i, 8
  %fd339.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr17.sum21.i.i
  %305 = bitcast i8* %fd339.i.i to %struct.malloc_tree_chunk**
  store %struct.malloc_tree_chunk* %289, %struct.malloc_tree_chunk** %305, align 4
  br label %prepend_alloc.exit.i

if.else340.i.i:                                   ; preds = %if.else330.i.i
  tail call void @abort() #6
  unreachable

if.else342.i.i:                                   ; preds = %for.cond.i68.i, %cond.end314.i.i
  %T.0.lcssa.i69.i = phi %struct.malloc_tree_chunk* [ %297, %cond.end314.i.i ], [ %300, %for.cond.i68.i ]
  %fd344.i.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %T.0.lcssa.i69.i, i32 0, i32 2
  %306 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %fd344.i.i, align 4
  %307 = bitcast %struct.malloc_tree_chunk* %T.0.lcssa.i69.i to i8*
  %308 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp346.i.i = icmp ult i8* %307, %308
  br i1 %cmp346.i.i, label %if.else362.i.i, label %land.rhs348.i.i

land.rhs348.i.i:                                  ; preds = %if.else342.i.i
  %309 = bitcast %struct.malloc_tree_chunk* %306 to i8*
  %cmp350.i.i = icmp ult i8* %309, %308
  br i1 %cmp350.i.i, label %if.else362.i.i, label %if.then356.i.i

if.then356.i.i:                                   ; preds = %land.rhs348.i.i
  %bk357.i.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %306, i32 0, i32 3
  store %struct.malloc_tree_chunk* %289, %struct.malloc_tree_chunk** %bk357.i.i, align 4
  store %struct.malloc_tree_chunk* %289, %struct.malloc_tree_chunk** %fd344.i.i, align 4
  %add.ptr17.sum16.i.i = add i32 %add.ptr4.sum.i49.i, 8
  %fd359.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr17.sum16.i.i
  %310 = bitcast i8* %fd359.i.i to %struct.malloc_tree_chunk**
  store %struct.malloc_tree_chunk* %306, %struct.malloc_tree_chunk** %310, align 4
  %add.ptr17.sum17.i.i = add i32 %add.ptr4.sum.i49.i, 12
  %bk360.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr17.sum17.i.i
  %311 = bitcast i8* %bk360.i.i to %struct.malloc_tree_chunk**
  store %struct.malloc_tree_chunk* %T.0.lcssa.i69.i, %struct.malloc_tree_chunk** %311, align 4
  %add.ptr17.sum18.i.i = add i32 %add.ptr4.sum.i49.i, 24
  %parent361.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr17.sum18.i.i
  %312 = bitcast i8* %parent361.i.i to %struct.malloc_tree_chunk**
  store %struct.malloc_tree_chunk* null, %struct.malloc_tree_chunk** %312, align 4
  br label %prepend_alloc.exit.i

if.else362.i.i:                                   ; preds = %land.rhs348.i.i, %if.else342.i.i
  tail call void @abort() #6
  unreachable

prepend_alloc.exit.i:                             ; preds = %if.then356.i.i, %if.then336.i.i, %if.then297.i.i, %if.end244.i.i, %if.then25.i.i, %if.then.i51.i
  %add.ptr4.sum1415.i.i = or i32 %cond.i42.i, 8
  %add.ptr368.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr4.sum1415.i.i
  br label %postaction

if.else245.i:                                     ; preds = %land.lhs.true231.i, %while.body227.i
  %313 = bitcast %struct.malloc_chunk* %188 to i8*
  br label %for.cond.i.i.i

for.cond.i.i.i:                                   ; preds = %if.end.i.i.i, %if.else245.i
  %sp.0.i.i.i = phi %struct.malloc_segment* [ getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 16), %if.else245.i ], [ %316, %if.end.i.i.i ]
  %base.i.i.i = getelementptr inbounds %struct.malloc_segment, %struct.malloc_segment* %sp.0.i.i.i, i32 0, i32 0
  %314 = load i8*, i8** %base.i.i.i, align 4
  %cmp.i.i.i = icmp ugt i8* %314, %313
  br i1 %cmp.i.i.i, label %if.end.i.i.i, label %land.lhs.true.i.i.i

land.lhs.true.i.i.i:                              ; preds = %for.cond.i.i.i
  %size.i.i.i = getelementptr inbounds %struct.malloc_segment, %struct.malloc_segment* %sp.0.i.i.i, i32 0, i32 1
  %315 = load i32, i32* %size.i.i.i, align 4
  %add.ptr.i.i.i = getelementptr inbounds i8, i8* %314, i32 %315
  %cmp2.i.i.i = icmp ugt i8* %add.ptr.i.i.i, %313
  br i1 %cmp2.i.i.i, label %segment_holding.exit.i.i, label %if.end.i.i.i

if.end.i.i.i:                                     ; preds = %land.lhs.true.i.i.i, %for.cond.i.i.i
  %next.i.i.i = getelementptr inbounds %struct.malloc_segment, %struct.malloc_segment* %sp.0.i.i.i, i32 0, i32 2
  %316 = load %struct.malloc_segment*, %struct.malloc_segment** %next.i.i.i, align 4
  br label %for.cond.i.i.i

segment_holding.exit.i.i:                         ; preds = %land.lhs.true.i.i.i
  %add.ptr.sum.i.i = add i32 %315, -47
  %add.ptr2.sum.i.i = add i32 %315, -39
  %add.ptr3.i.i = getelementptr inbounds i8, i8* %314, i32 %add.ptr2.sum.i.i
  %317 = ptrtoint i8* %add.ptr3.i.i to i32
  %and.i14.i = and i32 %317, 7
  %cmp.i15.i = icmp eq i32 %and.i14.i, 0
  br i1 %cmp.i15.i, label %cond.end.i.i, label %cond.false.i16.i

cond.false.i16.i:                                 ; preds = %segment_holding.exit.i.i
  %318 = sub i32 0, %317
  %and6.i.i = and i32 %318, 7
  br label %cond.end.i.i

cond.end.i.i:                                     ; preds = %cond.false.i16.i, %segment_holding.exit.i.i
  %cond.i17.i = phi i32 [ %and6.i.i, %cond.false.i16.i ], [ 0, %segment_holding.exit.i.i ]
  %add.ptr2.sum1.i.i = add i32 %add.ptr.sum.i.i, %cond.i17.i
  %add.ptr7.i.i = getelementptr inbounds i8, i8* %314, i32 %add.ptr2.sum1.i.i
  %add.ptr82.i.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %188, i32 1
  %add.ptr8.i.i = bitcast %struct.malloc_chunk* %add.ptr82.i.i to i8*
  %cmp9.i.i = icmp ult i8* %add.ptr7.i.i, %add.ptr8.i.i
  %cond13.i.i = select i1 %cmp9.i.i, i8* %313, i8* %add.ptr7.i.i
  %add.ptr14.i.i = getelementptr inbounds i8, i8* %cond13.i.i, i32 8
  %319 = bitcast i8* %add.ptr14.i.i to %struct.malloc_segment*
  %sub16.i.i = add i32 %tsize.290.i, -40
  %add.ptr.i10.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 8
  %320 = ptrtoint i8* %add.ptr.i10.i.i to i32
  %and.i.i.i = and i32 %320, 7
  %cmp.i11.i.i = icmp eq i32 %and.i.i.i, 0
  br i1 %cmp.i11.i.i, label %init_top.exit.i.i, label %cond.false.i.i.i

cond.false.i.i.i:                                 ; preds = %cond.end.i.i
  %321 = sub i32 0, %320
  %and3.i.i.i = and i32 %321, 7
  br label %init_top.exit.i.i

init_top.exit.i.i:                                ; preds = %cond.false.i.i.i, %cond.end.i.i
  %cond.i.i.i = phi i32 [ %and3.i.i.i, %cond.false.i.i.i ], [ 0, %cond.end.i.i ]
  %add.ptr4.i.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %cond.i.i.i
  %322 = bitcast i8* %add.ptr4.i.i.i to %struct.malloc_chunk*
  %sub5.i.i.i = sub i32 %sub16.i.i, %cond.i.i.i
  store %struct.malloc_chunk* %322, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 6), align 4
  store i32 %sub5.i.i.i, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 3), align 4
  %or.i.i.i = or i32 %sub5.i.i.i, 1
  %add.ptr4.sum.i.i.i = add i32 %cond.i.i.i, 4
  %head.i.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr4.sum.i.i.i
  %323 = bitcast i8* %head.i.i.i to i32*
  store i32 %or.i.i.i, i32* %323, align 4
  %add.ptr6.sum.i.i.i = add i32 %tsize.290.i, -36
  %head7.i.i.i = getelementptr inbounds i8, i8* %tbase.291.i, i32 %add.ptr6.sum.i.i.i
  %324 = bitcast i8* %head7.i.i.i to i32*
  store i32 40, i32* %324, align 4
  %325 = load i32, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 4), align 4
  store i32 %325, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 7), align 4
  %head.i18.i = getelementptr inbounds i8, i8* %cond13.i.i, i32 4
  %326 = bitcast i8* %head.i18.i to i32*
  store i32 27, i32* %326, align 4
  tail call void @llvm.memcpy.p0i8.p0i8.i32(i8* %add.ptr14.i.i, i8* bitcast (%struct.malloc_segment* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 16) to i8*), i32 16, i32 4, i1 false) #1
  store i8* %tbase.291.i, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 16, i32 0), align 4
  store i32 %tsize.290.i, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 16, i32 1), align 4
  store i32 0, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 16, i32 3), align 4
  store %struct.malloc_segment* %319, %struct.malloc_segment** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 16, i32 2), align 4
  %add.ptr2418.i.i = getelementptr inbounds i8, i8* %cond13.i.i, i32 28
  %327 = bitcast i8* %add.ptr2418.i.i to i32*
  store i32 7, i32* %327, align 4
  %328 = getelementptr inbounds i8, i8* %cond13.i.i, i32 32
  %cmp2719.i.i = icmp ult i8* %328, %add.ptr.i.i.i
  br i1 %cmp2719.i.i, label %if.then.i19.i, label %for.end.i.i

if.then.i19.i:                                    ; preds = %if.then.i19.i, %init_top.exit.i.i
  %add.ptr2420.i.i = phi i32* [ %329, %if.then.i19.i ], [ %327, %init_top.exit.i.i ]
  %329 = getelementptr inbounds i32, i32* %add.ptr2420.i.i, i32 1
  store i32 7, i32* %329, align 4
  %330 = getelementptr inbounds i32, i32* %add.ptr2420.i.i, i32 2
  %331 = bitcast i32* %330 to i8*
  %cmp27.i.i = icmp ult i8* %331, %add.ptr.i.i.i
  br i1 %cmp27.i.i, label %if.then.i19.i, label %for.end.i.i

for.end.i.i:                                      ; preds = %if.then.i19.i, %init_top.exit.i.i
  %cmp28.i.i = icmp eq i8* %cond13.i.i, %313
  br i1 %cmp28.i.i, label %if.end248.i, label %if.then29.i.i

if.then29.i.i:                                    ; preds = %for.end.i.i
  %sub.ptr.lhs.cast.i.i = ptrtoint i8* %cond13.i.i to i32
  %sub.ptr.rhs.cast.i.i = ptrtoint %struct.malloc_chunk* %188 to i32
  %sub.ptr.sub.i.i = sub i32 %sub.ptr.lhs.cast.i.i, %sub.ptr.rhs.cast.i.i
  %add.ptr30.i.i = getelementptr inbounds i8, i8* %313, i32 %sub.ptr.sub.i.i
  %add.ptr30.sum.i.i = add i32 %sub.ptr.sub.i.i, 4
  %head31.i.i = getelementptr inbounds i8, i8* %313, i32 %add.ptr30.sum.i.i
  %332 = bitcast i8* %head31.i.i to i32*
  %333 = load i32, i32* %332, align 4
  %and32.i.i = and i32 %333, -2
  store i32 %and32.i.i, i32* %332, align 4
  %or33.i.i = or i32 %sub.ptr.sub.i.i, 1
  %head34.i.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %188, i32 0, i32 1
  store i32 %or33.i.i, i32* %head34.i.i, align 4
  %prev_foot.i.i = bitcast i8* %add.ptr30.i.i to i32*
  store i32 %sub.ptr.sub.i.i, i32* %prev_foot.i.i, align 4
  %shr.i.i = lshr i32 %sub.ptr.sub.i.i, 3
  %cmp36.i.i = icmp ult i32 %sub.ptr.sub.i.i, 256
  br i1 %cmp36.i.i, label %if.then37.i.i, label %if.else56.i.i

if.then37.i.i:                                    ; preds = %if.then29.i.i
  %shl.i20.i = shl nuw nsw i32 %shr.i.i, 1
  %arrayidx.i21.i = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %shl.i20.i
  %334 = bitcast %struct.malloc_chunk** %arrayidx.i21.i to %struct.malloc_chunk*
  %335 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  %shl39.i.i = shl i32 1, %shr.i.i
  %and40.i.i = and i32 %335, %shl39.i.i
  %tobool.i.i = icmp eq i32 %and40.i.i, 0
  br i1 %tobool.i.i, label %if.then41.i.i, label %if.else45.i.i

if.then41.i.i:                                    ; preds = %if.then37.i.i
  %or44.i.i = or i32 %335, %shl39.i.i
  store i32 %or44.i.i, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  %arrayidx.sum.pre.i.i = add i32 %shl.i20.i, 2
  %.pre.i.i = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %arrayidx.sum.pre.i.i
  br label %if.end52.i.i

if.else45.i.i:                                    ; preds = %if.then37.i.i
  %arrayidx.sum9.i.i = add i32 %shl.i20.i, 2
  %336 = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %arrayidx.sum9.i.i
  %337 = load %struct.malloc_chunk*, %struct.malloc_chunk** %336, align 4
  %338 = bitcast %struct.malloc_chunk* %337 to i8*
  %339 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp46.i.i = icmp ult i8* %338, %339
  br i1 %cmp46.i.i, label %if.else50.i.i, label %if.end52.i.i

if.else50.i.i:                                    ; preds = %if.else45.i.i
  tail call void @abort() #6
  unreachable

if.end52.i.i:                                     ; preds = %if.else45.i.i, %if.then41.i.i
  %.pre-phi.i.i = phi %struct.malloc_chunk** [ %336, %if.else45.i.i ], [ %.pre.i.i, %if.then41.i.i ]
  %F.0.i.i = phi %struct.malloc_chunk* [ %337, %if.else45.i.i ], [ %334, %if.then41.i.i ]
  store %struct.malloc_chunk* %188, %struct.malloc_chunk** %.pre-phi.i.i, align 4
  %bk.i.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %F.0.i.i, i32 0, i32 3
  store %struct.malloc_chunk* %188, %struct.malloc_chunk** %bk.i.i, align 4
  %fd54.i.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %188, i32 0, i32 2
  store %struct.malloc_chunk* %F.0.i.i, %struct.malloc_chunk** %fd54.i.i, align 4
  %bk55.i.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %188, i32 0, i32 3
  store %struct.malloc_chunk* %334, %struct.malloc_chunk** %bk55.i.i, align 4
  br label %if.end248.i

if.else56.i.i:                                    ; preds = %if.then29.i.i
  %340 = bitcast %struct.malloc_chunk* %188 to %struct.malloc_tree_chunk*
  %shr58.i.i = lshr i32 %sub.ptr.sub.i.i, 8
  %cmp59.i.i = icmp eq i32 %shr58.i.i, 0
  br i1 %cmp59.i.i, label %if.end90.i.i, label %if.else62.i.i

if.else62.i.i:                                    ; preds = %if.else56.i.i
  %cmp63.i.i = icmp ugt i32 %sub.ptr.sub.i.i, 16777215
  br i1 %cmp63.i.i, label %if.end90.i.i, label %if.else66.i.i

if.else66.i.i:                                    ; preds = %if.else62.i.i
  %sub67.i.i = add i32 %shr58.i.i, 1048320
  %shr68.i.i = lshr i32 %sub67.i.i, 16
  %and69.i.i = and i32 %shr68.i.i, 8
  %shl70.i.i = shl i32 %shr58.i.i, %and69.i.i
  %sub71.i.i = add i32 %shl70.i.i, 520192
  %shr72.i.i = lshr i32 %sub71.i.i, 16
  %and73.i.i = and i32 %shr72.i.i, 4
  %add74.i.i = or i32 %and73.i.i, %and69.i.i
  %shl75.i.i = shl i32 %shl70.i.i, %and73.i.i
  %sub76.i.i = add i32 %shl75.i.i, 245760
  %shr77.i.i = lshr i32 %sub76.i.i, 16
  %and78.i.i = and i32 %shr77.i.i, 2
  %add79.i.i = or i32 %add74.i.i, %and78.i.i
  %sub80.i.i = sub i32 14, %add79.i.i
  %shl81.i.i = shl i32 %shl75.i.i, %and78.i.i
  %shr82.i.i = lshr i32 %shl81.i.i, 15
  %add83.i.i = add i32 %sub80.i.i, %shr82.i.i
  %shl84.i.i = shl nsw i32 %add83.i.i, 1
  %add85.i.i = add i32 %add83.i.i, 7
  %shr86.i.i = lshr i32 %sub.ptr.sub.i.i, %add85.i.i
  %and87.i.i = and i32 %shr86.i.i, 1
  %add88.i.i = or i32 %and87.i.i, %shl84.i.i
  br label %if.end90.i.i

if.end90.i.i:                                     ; preds = %if.else66.i.i, %if.else62.i.i, %if.else56.i.i
  %I57.0.i.i = phi i32 [ %add88.i.i, %if.else66.i.i ], [ 0, %if.else56.i.i ], [ 31, %if.else62.i.i ]
  %arrayidx91.i.i = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 11, i32 %I57.0.i.i
  %index.i.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %188, i32 1, i32 3
  %I57.0.c.i.i = inttoptr i32 %I57.0.i.i to %struct.malloc_chunk*
  store %struct.malloc_chunk* %I57.0.c.i.i, %struct.malloc_chunk** %index.i.i, align 4
  %arrayidx92.i.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %188, i32 1, i32 1
  store i32 0, i32* %arrayidx92.i.i, align 4
  %341 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %188, i32 1, i32 0
  store i32 0, i32* %341, align 4
  %342 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 1), align 4
  %shl95.i.i = shl i32 1, %I57.0.i.i
  %and96.i.i = and i32 %342, %shl95.i.i
  %tobool97.i.i = icmp eq i32 %and96.i.i, 0
  br i1 %tobool97.i.i, label %if.then98.i.i, label %if.else104.i.i

if.then98.i.i:                                    ; preds = %if.end90.i.i
  %or101.i.i = or i32 %342, %shl95.i.i
  store i32 %or101.i.i, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 1), align 4
  store %struct.malloc_tree_chunk* %340, %struct.malloc_tree_chunk** %arrayidx91.i.i, align 4
  %parent.i.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %188, i32 1, i32 2
  %.c.i.i = bitcast %struct.malloc_tree_chunk** %arrayidx91.i.i to %struct.malloc_chunk*
  store %struct.malloc_chunk* %.c.i.i, %struct.malloc_chunk** %parent.i.i, align 4
  %bk102.i.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %188, i32 0, i32 3
  store %struct.malloc_chunk* %188, %struct.malloc_chunk** %bk102.i.i, align 4
  %fd103.i.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %188, i32 0, i32 2
  store %struct.malloc_chunk* %188, %struct.malloc_chunk** %fd103.i.i, align 4
  br label %if.end248.i

if.else104.i.i:                                   ; preds = %if.end90.i.i
  %343 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx91.i.i, align 4
  %cmp106.i.i = icmp eq i32 %I57.0.i.i, 31
  br i1 %cmp106.i.i, label %cond.end114.i.i, label %cond.false109.i.i

cond.false109.i.i:                                ; preds = %if.else104.i.i
  %shr110.i.i = lshr i32 %I57.0.i.i, 1
  %sub113.i.i = sub i32 25, %shr110.i.i
  br label %cond.end114.i.i

cond.end114.i.i:                                  ; preds = %cond.false109.i.i, %if.else104.i.i
  %cond115.i.i = phi i32 [ %sub113.i.i, %cond.false109.i.i ], [ 0, %if.else104.i.i ]
  %head11813.i.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %343, i32 0, i32 1
  %344 = load i32, i32* %head11813.i.i, align 4
  %and11914.i.i = and i32 %344, -8
  %cmp12015.i.i = icmp eq i32 %and11914.i.i, %sub.ptr.sub.i.i
  br i1 %cmp12015.i.i, label %if.else143.i.i, label %if.then122.lr.ph.i.i

if.then122.lr.ph.i.i:                             ; preds = %cond.end114.i.i
  %shl116.i.i = shl i32 %sub.ptr.sub.i.i, %cond115.i.i
  br label %if.then122.i.i

for.cond117.i.i:                                  ; preds = %if.then122.i.i
  %shl127.i.i = shl i32 %K105.017.i.i, 1
  %head118.i.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %346, i32 0, i32 1
  %345 = load i32, i32* %head118.i.i, align 4
  %and119.i.i = and i32 %345, -8
  %cmp120.i.i = icmp eq i32 %and119.i.i, %sub.ptr.sub.i.i
  br i1 %cmp120.i.i, label %if.else143.i.i, label %if.then122.i.i

if.then122.i.i:                                   ; preds = %for.cond117.i.i, %if.then122.lr.ph.i.i
  %K105.017.i.i = phi i32 [ %shl116.i.i, %if.then122.lr.ph.i.i ], [ %shl127.i.i, %for.cond117.i.i ]
  %T.016.i.i = phi %struct.malloc_tree_chunk* [ %343, %if.then122.lr.ph.i.i ], [ %346, %for.cond117.i.i ]
  %shr123.i.i = lshr i32 %K105.017.i.i, 31
  %arrayidx126.i.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %T.016.i.i, i32 0, i32 4, i32 %shr123.i.i
  %346 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx126.i.i, align 4
  %cmp128.i.i = icmp eq %struct.malloc_tree_chunk* %346, null
  br i1 %cmp128.i.i, label %if.else131.i.i, label %for.cond117.i.i

if.else131.i.i:                                   ; preds = %if.then122.i.i
  %347 = bitcast %struct.malloc_tree_chunk** %arrayidx126.i.i to i8*
  %348 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp133.i.i = icmp ult i8* %347, %348
  br i1 %cmp133.i.i, label %if.else141.i.i, label %if.then137.i.i

if.then137.i.i:                                   ; preds = %if.else131.i.i
  store %struct.malloc_tree_chunk* %340, %struct.malloc_tree_chunk** %arrayidx126.i.i, align 4
  %parent138.i.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %188, i32 1, i32 2
  %T.0.c6.i.i = bitcast %struct.malloc_tree_chunk* %T.016.i.i to %struct.malloc_chunk*
  store %struct.malloc_chunk* %T.0.c6.i.i, %struct.malloc_chunk** %parent138.i.i, align 4
  %bk139.i.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %188, i32 0, i32 3
  store %struct.malloc_chunk* %188, %struct.malloc_chunk** %bk139.i.i, align 4
  %fd140.i.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %188, i32 0, i32 2
  store %struct.malloc_chunk* %188, %struct.malloc_chunk** %fd140.i.i, align 4
  br label %if.end248.i

if.else141.i.i:                                   ; preds = %if.else131.i.i
  tail call void @abort() #6
  unreachable

if.else143.i.i:                                   ; preds = %for.cond117.i.i, %cond.end114.i.i
  %T.0.lcssa.i.i = phi %struct.malloc_tree_chunk* [ %343, %cond.end114.i.i ], [ %346, %for.cond117.i.i ]
  %fd145.i.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %T.0.lcssa.i.i, i32 0, i32 2
  %349 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %fd145.i.i, align 4
  %350 = bitcast %struct.malloc_tree_chunk* %T.0.lcssa.i.i to i8*
  %351 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp147.i.i = icmp ult i8* %350, %351
  br i1 %cmp147.i.i, label %if.else160.i.i, label %land.rhs.i.i

land.rhs.i.i:                                     ; preds = %if.else143.i.i
  %352 = bitcast %struct.malloc_tree_chunk* %349 to i8*
  %cmp150.i.i = icmp ult i8* %352, %351
  br i1 %cmp150.i.i, label %if.else160.i.i, label %if.then154.i.i

if.then154.i.i:                                   ; preds = %land.rhs.i.i
  %bk155.i.i = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %349, i32 0, i32 3
  store %struct.malloc_tree_chunk* %340, %struct.malloc_tree_chunk** %bk155.i.i, align 4
  store %struct.malloc_tree_chunk* %340, %struct.malloc_tree_chunk** %fd145.i.i, align 4
  %fd157.i.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %188, i32 0, i32 2
  %.c5.i.i = bitcast %struct.malloc_tree_chunk* %349 to %struct.malloc_chunk*
  store %struct.malloc_chunk* %.c5.i.i, %struct.malloc_chunk** %fd157.i.i, align 4
  %bk158.i.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %188, i32 0, i32 3
  %T.0.c.i.i = bitcast %struct.malloc_tree_chunk* %T.0.lcssa.i.i to %struct.malloc_chunk*
  store %struct.malloc_chunk* %T.0.c.i.i, %struct.malloc_chunk** %bk158.i.i, align 4
  %parent159.i.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %188, i32 1, i32 2
  store %struct.malloc_chunk* null, %struct.malloc_chunk** %parent159.i.i, align 4
  br label %if.end248.i

if.else160.i.i:                                   ; preds = %land.rhs.i.i, %if.else143.i.i
  tail call void @abort() #6
  unreachable

if.end248.i:                                      ; preds = %if.then154.i.i, %if.then137.i.i, %if.then98.i.i, %if.end52.i.i, %for.end.i.i, %init_top.exit36.i, %init_top.exit.i
  %353 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 3), align 4
  %cmp250.i = icmp ugt i32 %353, %nb.0
  br i1 %cmp250.i, label %if.then251.i, label %if.end264.i

if.then251.i:                                     ; preds = %if.end248.i
  %sub253.i = sub i32 %353, %nb.0
  store i32 %sub253.i, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 3), align 4
  %354 = load %struct.malloc_chunk*, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 6), align 4
  %355 = bitcast %struct.malloc_chunk* %354 to i8*
  %add.ptr255.i = getelementptr inbounds i8, i8* %355, i32 %nb.0
  %356 = bitcast i8* %add.ptr255.i to %struct.malloc_chunk*
  store %struct.malloc_chunk* %356, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 6), align 4
  %or257.i = or i32 %sub253.i, 1
  %add.ptr255.sum.i = add i32 %nb.0, 4
  %head258.i = getelementptr inbounds i8, i8* %355, i32 %add.ptr255.sum.i
  %357 = bitcast i8* %head258.i to i32*
  store i32 %or257.i, i32* %357, align 4
  %or260.i = or i32 %nb.0, 3
  %head261.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %354, i32 0, i32 1
  store i32 %or260.i, i32* %head261.i, align 4
  %add.ptr262.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %354, i32 0, i32 2
  %358 = bitcast %struct.malloc_chunk** %add.ptr262.i to i8*
  br label %postaction

if.end264.i:                                      ; preds = %if.end248.i, %if.end143.i, %if.then125.i, %if.end121.i
  %call265.i = tail call i32* @__errno_location() #7
  store i32 12, i32* %call265.i, align 4
  br label %postaction

postaction:                                       ; preds = %if.end264.i, %if.then251.i, %prepend_alloc.exit.i, %if.then16.i, %if.end8.i, %if.then185, %if.end180, %tmalloc_large.exit, %tmalloc_small.exit, %if.end125, %if.end21
  %mem.0 = phi i8* [ %11, %if.end21 ], [ %31, %if.end125 ], [ %77, %tmalloc_small.exit ], [ %160, %if.end180 ], [ %166, %if.then185 ], [ %151, %tmalloc_large.exit ], [ %358, %if.then251.i ], [ null, %if.end264.i ], [ %add.ptr368.i.i, %prepend_alloc.exit.i ], [ null, %if.end8.i ], [ null, %if.then16.i ]
  ret i8* %mem.0
}

; Function Attrs: noreturn
declare void @abort() #4

; Function Attrs: nounwind
define weak void @free(i8* %mem) #0 {
entry:
  %cmp = icmp eq i8* %mem, null
  br i1 %cmp, label %if.end635, label %if.then

if.then:                                          ; preds = %entry
  %add.ptr = getelementptr inbounds i8, i8* %mem, i32 -8
  %0 = bitcast i8* %add.ptr to %struct.malloc_chunk*
  %1 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp1 = icmp ult i8* %add.ptr, %1
  br i1 %cmp1, label %erroraction, label %land.rhs

land.rhs:                                         ; preds = %if.then
  %head = getelementptr inbounds i8, i8* %mem, i32 -4
  %2 = bitcast i8* %head to i32*
  %3 = load i32, i32* %2, align 4
  %and = and i32 %3, 3
  %cmp2 = icmp eq i32 %and, 1
  br i1 %cmp2, label %erroraction, label %if.then3

if.then3:                                         ; preds = %land.rhs
  %and5 = and i32 %3, -8
  %add.ptr.sum = add i32 %and5, -8
  %add.ptr6 = getelementptr inbounds i8, i8* %mem, i32 %add.ptr.sum
  %4 = bitcast i8* %add.ptr6 to %struct.malloc_chunk*
  %and8 = and i32 %3, 1
  %tobool9 = icmp eq i32 %and8, 0
  br i1 %tobool9, label %if.then10, label %if.end224

if.then10:                                        ; preds = %if.then3
  %prev_foot = bitcast i8* %add.ptr to i32*
  %5 = load i32, i32* %prev_foot, align 4
  %cmp13 = icmp eq i32 %and, 0
  br i1 %cmp13, label %if.end635, label %if.else

if.else:                                          ; preds = %if.then10
  %add.ptr.sum230 = sub i32 -8, %5
  %add.ptr16 = getelementptr inbounds i8, i8* %mem, i32 %add.ptr.sum230
  %6 = bitcast i8* %add.ptr16 to %struct.malloc_chunk*
  %add17 = add i32 %5, %and5
  %cmp18 = icmp ult i8* %add.ptr16, %1
  br i1 %cmp18, label %erroraction, label %if.then21

if.then21:                                        ; preds = %if.else
  %7 = load %struct.malloc_chunk*, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 5), align 4
  %cmp22 = icmp eq %struct.malloc_chunk* %6, %7
  br i1 %cmp22, label %if.else208, label %if.then24

if.then24:                                        ; preds = %if.then21
  %shr = lshr i32 %5, 3
  %cmp25 = icmp ult i32 %5, 256
  br i1 %cmp25, label %if.then27, label %if.else72

if.then27:                                        ; preds = %if.then24
  %add.ptr16.sum257 = add i32 %add.ptr.sum230, 8
  %fd = getelementptr inbounds i8, i8* %mem, i32 %add.ptr16.sum257
  %8 = bitcast i8* %fd to %struct.malloc_chunk**
  %9 = load %struct.malloc_chunk*, %struct.malloc_chunk** %8, align 4
  %add.ptr16.sum258 = add i32 %add.ptr.sum230, 12
  %bk = getelementptr inbounds i8, i8* %mem, i32 %add.ptr16.sum258
  %10 = bitcast i8* %bk to %struct.malloc_chunk**
  %11 = load %struct.malloc_chunk*, %struct.malloc_chunk** %10, align 4
  %shl = shl nuw nsw i32 %shr, 1
  %arrayidx = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %shl
  %12 = bitcast %struct.malloc_chunk** %arrayidx to %struct.malloc_chunk*
  %cmp29 = icmp eq %struct.malloc_chunk* %9, %12
  br i1 %cmp29, label %if.then41, label %lor.rhs

lor.rhs:                                          ; preds = %if.then27
  %13 = bitcast %struct.malloc_chunk* %9 to i8*
  %cmp31 = icmp ult i8* %13, %1
  br i1 %cmp31, label %if.else70, label %land.rhs33

land.rhs33:                                       ; preds = %lor.rhs
  %bk34 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %9, i32 0, i32 3
  %14 = load %struct.malloc_chunk*, %struct.malloc_chunk** %bk34, align 4
  %cmp35 = icmp eq %struct.malloc_chunk* %14, %6
  br i1 %cmp35, label %if.then41, label %if.else70

if.then41:                                        ; preds = %land.rhs33, %if.then27
  %cmp42 = icmp eq %struct.malloc_chunk* %11, %9
  br i1 %cmp42, label %if.then44, label %if.else47

if.then44:                                        ; preds = %if.then41
  %shl45 = shl i32 1, %shr
  %neg = xor i32 %shl45, -1
  %15 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  %and46 = and i32 %15, %neg
  store i32 %and46, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  br label %if.end224

if.else47:                                        ; preds = %if.then41
  %cmp50 = icmp eq %struct.malloc_chunk* %11, %12
  br i1 %cmp50, label %if.else47.if.then65_crit_edge, label %lor.rhs52

if.else47.if.then65_crit_edge:                    ; preds = %if.else47
  %fd67.pre = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %11, i32 0, i32 2
  br label %if.then65

lor.rhs52:                                        ; preds = %if.else47
  %16 = bitcast %struct.malloc_chunk* %11 to i8*
  %cmp53 = icmp ult i8* %16, %1
  br i1 %cmp53, label %if.else68, label %land.rhs55

land.rhs55:                                       ; preds = %lor.rhs52
  %fd56 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %11, i32 0, i32 2
  %17 = load %struct.malloc_chunk*, %struct.malloc_chunk** %fd56, align 4
  %cmp57 = icmp eq %struct.malloc_chunk* %17, %6
  br i1 %cmp57, label %if.then65, label %if.else68

if.then65:                                        ; preds = %land.rhs55, %if.else47.if.then65_crit_edge
  %fd67.pre-phi = phi %struct.malloc_chunk** [ %fd67.pre, %if.else47.if.then65_crit_edge ], [ %fd56, %land.rhs55 ]
  %bk66 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %9, i32 0, i32 3
  store %struct.malloc_chunk* %11, %struct.malloc_chunk** %bk66, align 4
  store %struct.malloc_chunk* %9, %struct.malloc_chunk** %fd67.pre-phi, align 4
  br label %if.end224

if.else68:                                        ; preds = %land.rhs55, %lor.rhs52
  tail call void @abort() #6
  unreachable

if.else70:                                        ; preds = %land.rhs33, %lor.rhs
  tail call void @abort() #6
  unreachable

if.else72:                                        ; preds = %if.then24
  %18 = bitcast i8* %add.ptr16 to %struct.malloc_tree_chunk*
  %add.ptr16.sum251 = add i32 %add.ptr.sum230, 24
  %parent = getelementptr inbounds i8, i8* %mem, i32 %add.ptr16.sum251
  %19 = bitcast i8* %parent to %struct.malloc_tree_chunk**
  %20 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %19, align 4
  %add.ptr16.sum252 = add i32 %add.ptr.sum230, 12
  %bk73 = getelementptr inbounds i8, i8* %mem, i32 %add.ptr16.sum252
  %21 = bitcast i8* %bk73 to %struct.malloc_tree_chunk**
  %22 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %21, align 4
  %cmp74 = icmp eq %struct.malloc_tree_chunk* %22, %18
  br i1 %cmp74, label %if.else98, label %if.then76

if.then76:                                        ; preds = %if.else72
  %add.ptr16.sum256 = add i32 %add.ptr.sum230, 8
  %fd78 = getelementptr inbounds i8, i8* %mem, i32 %add.ptr16.sum256
  %23 = bitcast i8* %fd78 to %struct.malloc_tree_chunk**
  %24 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %23, align 4
  %25 = bitcast %struct.malloc_tree_chunk* %24 to i8*
  %cmp80 = icmp ult i8* %25, %1
  br i1 %cmp80, label %if.else96, label %land.lhs.true

land.lhs.true:                                    ; preds = %if.then76
  %bk82 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %24, i32 0, i32 3
  %26 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %bk82, align 4
  %cmp83 = icmp eq %struct.malloc_tree_chunk* %26, %18
  br i1 %cmp83, label %land.rhs85, label %if.else96

land.rhs85:                                       ; preds = %land.lhs.true
  %fd86 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %22, i32 0, i32 2
  %27 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %fd86, align 4
  %cmp87 = icmp eq %struct.malloc_tree_chunk* %27, %18
  br i1 %cmp87, label %if.then93, label %if.else96

if.then93:                                        ; preds = %land.rhs85
  store %struct.malloc_tree_chunk* %22, %struct.malloc_tree_chunk** %bk82, align 4
  store %struct.malloc_tree_chunk* %24, %struct.malloc_tree_chunk** %fd86, align 4
  br label %if.end126

if.else96:                                        ; preds = %land.rhs85, %land.lhs.true, %if.then76
  tail call void @abort() #6
  unreachable

if.else98:                                        ; preds = %if.else72
  %child.sum = add i32 %add.ptr.sum230, 20
  %arrayidx99 = getelementptr inbounds i8, i8* %mem, i32 %child.sum
  %28 = bitcast i8* %arrayidx99 to %struct.malloc_tree_chunk**
  %29 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %28, align 4
  %cmp100 = icmp eq %struct.malloc_tree_chunk* %29, null
  br i1 %cmp100, label %lor.lhs.false, label %while.cond

lor.lhs.false:                                    ; preds = %if.else98
  %add.ptr16.sum253 = add i32 %add.ptr.sum230, 16
  %child = getelementptr inbounds i8, i8* %mem, i32 %add.ptr16.sum253
  %arrayidx103 = bitcast i8* %child to %struct.malloc_tree_chunk**
  %30 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx103, align 4
  %cmp104 = icmp eq %struct.malloc_tree_chunk* %30, null
  br i1 %cmp104, label %if.end126, label %while.cond

while.cond:                                       ; preds = %lor.rhs111, %while.cond, %lor.lhs.false, %if.else98
  %RP.0 = phi %struct.malloc_tree_chunk** [ %arrayidx103, %lor.lhs.false ], [ %28, %if.else98 ], [ %arrayidx108, %while.cond ], [ %arrayidx113, %lor.rhs111 ]
  %R.0 = phi %struct.malloc_tree_chunk* [ %30, %lor.lhs.false ], [ %29, %if.else98 ], [ %31, %while.cond ], [ %32, %lor.rhs111 ]
  %arrayidx108 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.0, i32 0, i32 4, i32 1
  %31 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx108, align 4
  %cmp109 = icmp eq %struct.malloc_tree_chunk* %31, null
  br i1 %cmp109, label %lor.rhs111, label %while.cond

lor.rhs111:                                       ; preds = %while.cond
  %arrayidx113 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.0, i32 0, i32 4, i32 0
  %32 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx113, align 4
  %cmp114 = icmp eq %struct.malloc_tree_chunk* %32, null
  br i1 %cmp114, label %while.end, label %while.cond

while.end:                                        ; preds = %lor.rhs111
  %33 = bitcast %struct.malloc_tree_chunk** %RP.0 to i8*
  %cmp118 = icmp ult i8* %33, %1
  br i1 %cmp118, label %if.else123, label %if.then122

if.then122:                                       ; preds = %while.end
  store %struct.malloc_tree_chunk* null, %struct.malloc_tree_chunk** %RP.0, align 4
  br label %if.end126

if.else123:                                       ; preds = %while.end
  tail call void @abort() #6
  unreachable

if.end126:                                        ; preds = %if.then122, %lor.lhs.false, %if.then93
  %R.1 = phi %struct.malloc_tree_chunk* [ %22, %if.then93 ], [ %R.0, %if.then122 ], [ null, %lor.lhs.false ]
  %cmp127 = icmp eq %struct.malloc_tree_chunk* %20, null
  br i1 %cmp127, label %if.end224, label %if.then129

if.then129:                                       ; preds = %if.end126
  %add.ptr16.sum254 = add i32 %add.ptr.sum230, 28
  %index = getelementptr inbounds i8, i8* %mem, i32 %add.ptr16.sum254
  %34 = bitcast i8* %index to i32*
  %35 = load i32, i32* %34, align 4
  %arrayidx130 = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 11, i32 %35
  %36 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx130, align 4
  %cmp131 = icmp eq %struct.malloc_tree_chunk* %18, %36
  br i1 %cmp131, label %if.then133, label %if.else142

if.then133:                                       ; preds = %if.then129
  store %struct.malloc_tree_chunk* %R.1, %struct.malloc_tree_chunk** %arrayidx130, align 4
  %cond263 = icmp eq %struct.malloc_tree_chunk* %R.1, null
  br i1 %cond263, label %if.end161.thread, label %if.then164

if.end161.thread:                                 ; preds = %if.then133
  %shl138 = shl i32 1, %35
  %neg139 = xor i32 %shl138, -1
  %37 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 1), align 4
  %and140 = and i32 %37, %neg139
  store i32 %and140, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 1), align 4
  br label %if.end224

if.else142:                                       ; preds = %if.then129
  %38 = bitcast %struct.malloc_tree_chunk* %20 to i8*
  %39 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp143 = icmp ult i8* %38, %39
  br i1 %cmp143, label %if.else159, label %if.then147

if.then147:                                       ; preds = %if.else142
  %arrayidx149 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %20, i32 0, i32 4, i32 0
  %40 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx149, align 4
  %cmp150 = icmp eq %struct.malloc_tree_chunk* %40, %18
  br i1 %cmp150, label %if.then152, label %if.else155

if.then152:                                       ; preds = %if.then147
  store %struct.malloc_tree_chunk* %R.1, %struct.malloc_tree_chunk** %arrayidx149, align 4
  br label %if.end161

if.else155:                                       ; preds = %if.then147
  %arrayidx157 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %20, i32 0, i32 4, i32 1
  store %struct.malloc_tree_chunk* %R.1, %struct.malloc_tree_chunk** %arrayidx157, align 4
  br label %if.end161

if.else159:                                       ; preds = %if.else142
  tail call void @abort() #6
  unreachable

if.end161:                                        ; preds = %if.else155, %if.then152
  %cmp162 = icmp eq %struct.malloc_tree_chunk* %R.1, null
  br i1 %cmp162, label %if.end224, label %if.then164

if.then164:                                       ; preds = %if.end161, %if.then133
  %41 = bitcast %struct.malloc_tree_chunk* %R.1 to i8*
  %42 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp165 = icmp ult i8* %41, %42
  br i1 %cmp165, label %if.else203, label %if.then169

if.then169:                                       ; preds = %if.then164
  %parent170 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.1, i32 0, i32 5
  store %struct.malloc_tree_chunk* %20, %struct.malloc_tree_chunk** %parent170, align 4
  %add.ptr16.sum255 = add i32 %add.ptr.sum230, 16
  %child171 = getelementptr inbounds i8, i8* %mem, i32 %add.ptr16.sum255
  %arrayidx172 = bitcast i8* %child171 to %struct.malloc_tree_chunk**
  %43 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx172, align 4
  %cmp173 = icmp eq %struct.malloc_tree_chunk* %43, null
  br i1 %cmp173, label %if.end186, label %if.then175

if.then175:                                       ; preds = %if.then169
  %44 = bitcast %struct.malloc_tree_chunk* %43 to i8*
  %45 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp176 = icmp ult i8* %44, %45
  br i1 %cmp176, label %if.else184, label %if.then180

if.then180:                                       ; preds = %if.then175
  %arrayidx182 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.1, i32 0, i32 4, i32 0
  store %struct.malloc_tree_chunk* %43, %struct.malloc_tree_chunk** %arrayidx182, align 4
  %parent183 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %43, i32 0, i32 5
  store %struct.malloc_tree_chunk* %R.1, %struct.malloc_tree_chunk** %parent183, align 4
  br label %if.end186

if.else184:                                       ; preds = %if.then175
  tail call void @abort() #6
  unreachable

if.end186:                                        ; preds = %if.then180, %if.then169
  %child171.sum = add i32 %add.ptr.sum230, 20
  %arrayidx188 = getelementptr inbounds i8, i8* %mem, i32 %child171.sum
  %46 = bitcast i8* %arrayidx188 to %struct.malloc_tree_chunk**
  %47 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %46, align 4
  %cmp189 = icmp eq %struct.malloc_tree_chunk* %47, null
  br i1 %cmp189, label %if.end224, label %if.then191

if.then191:                                       ; preds = %if.end186
  %48 = bitcast %struct.malloc_tree_chunk* %47 to i8*
  %49 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp192 = icmp ult i8* %48, %49
  br i1 %cmp192, label %if.else200, label %if.then196

if.then196:                                       ; preds = %if.then191
  %arrayidx198 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.1, i32 0, i32 4, i32 1
  store %struct.malloc_tree_chunk* %47, %struct.malloc_tree_chunk** %arrayidx198, align 4
  %parent199 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %47, i32 0, i32 5
  store %struct.malloc_tree_chunk* %R.1, %struct.malloc_tree_chunk** %parent199, align 4
  br label %if.end224

if.else200:                                       ; preds = %if.then191
  tail call void @abort() #6
  unreachable

if.else203:                                       ; preds = %if.then164
  tail call void @abort() #6
  unreachable

if.else208:                                       ; preds = %if.then21
  %add.ptr6.sum = add i32 %and5, -4
  %head209 = getelementptr inbounds i8, i8* %mem, i32 %add.ptr6.sum
  %50 = bitcast i8* %head209 to i32*
  %51 = load i32, i32* %50, align 4
  %and210 = and i32 %51, 3
  %cmp211 = icmp eq i32 %and210, 3
  br i1 %cmp211, label %if.then213, label %if.end224

if.then213:                                       ; preds = %if.else208
  store i32 %add17, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 2), align 4
  %52 = load i32, i32* %50, align 4
  %and215 = and i32 %52, -2
  store i32 %and215, i32* %50, align 4
  %or = or i32 %add17, 1
  %add.ptr16.sum = add i32 %add.ptr.sum230, 4
  %head216 = getelementptr inbounds i8, i8* %mem, i32 %add.ptr16.sum
  %53 = bitcast i8* %head216 to i32*
  store i32 %or, i32* %53, align 4
  %prev_foot218 = bitcast i8* %add.ptr6 to i32*
  store i32 %add17, i32* %prev_foot218, align 4
  br label %if.end635

if.end224:                                        ; preds = %if.else208, %if.then196, %if.end186, %if.end161, %if.end161.thread, %if.end126, %if.then65, %if.then44, %if.then3
  %psize.0 = phi i32 [ %and5, %if.then3 ], [ %add17, %if.then44 ], [ %add17, %if.then65 ], [ %add17, %if.then196 ], [ %add17, %if.end186 ], [ %add17, %if.end161 ], [ %add17, %if.end126 ], [ %add17, %if.else208 ], [ %add17, %if.end161.thread ]
  %p.0 = phi %struct.malloc_chunk* [ %0, %if.then3 ], [ %6, %if.then44 ], [ %6, %if.then65 ], [ %6, %if.then196 ], [ %6, %if.end186 ], [ %6, %if.end161 ], [ %6, %if.end126 ], [ %6, %if.else208 ], [ %6, %if.end161.thread ]
  %54 = bitcast %struct.malloc_chunk* %p.0 to i8*
  %cmp225 = icmp ult i8* %54, %add.ptr6
  br i1 %cmp225, label %land.rhs227, label %erroraction

land.rhs227:                                      ; preds = %if.end224
  %add.ptr6.sum249 = add i32 %and5, -4
  %head228 = getelementptr inbounds i8, i8* %mem, i32 %add.ptr6.sum249
  %55 = bitcast i8* %head228 to i32*
  %56 = load i32, i32* %55, align 4
  %and229 = and i32 %56, 1
  %tobool230 = icmp eq i32 %and229, 0
  br i1 %tobool230, label %erroraction, label %if.then235

if.then235:                                       ; preds = %land.rhs227
  %and237 = and i32 %56, 2
  %tobool238 = icmp eq i32 %and237, 0
  br i1 %tobool238, label %if.then239, label %if.else485

if.then239:                                       ; preds = %if.then235
  %57 = load %struct.malloc_chunk*, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 6), align 4
  %cmp240 = icmp eq %struct.malloc_chunk* %4, %57
  br i1 %cmp240, label %if.then242, label %if.else250

if.then242:                                       ; preds = %if.then239
  %58 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 3), align 4
  %add243 = add i32 %58, %psize.0
  store i32 %add243, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 3), align 4
  store %struct.malloc_chunk* %p.0, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 6), align 4
  %or244 = or i32 %add243, 1
  %head245 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.0, i32 0, i32 1
  store i32 %or244, i32* %head245, align 4
  %59 = load %struct.malloc_chunk*, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 5), align 4
  %cmp246 = icmp eq %struct.malloc_chunk* %p.0, %59
  br i1 %cmp246, label %if.then248, label %if.end635

if.then248:                                       ; preds = %if.then242
  store %struct.malloc_chunk* null, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 5), align 4
  store i32 0, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 2), align 4
  br label %if.end635

if.else250:                                       ; preds = %if.then239
  %60 = load %struct.malloc_chunk*, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 5), align 4
  %cmp251 = icmp eq %struct.malloc_chunk* %4, %60
  br i1 %cmp251, label %if.then253, label %if.else259

if.then253:                                       ; preds = %if.else250
  %61 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 2), align 4
  %add254 = add i32 %61, %psize.0
  store i32 %add254, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 2), align 4
  store %struct.malloc_chunk* %p.0, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 5), align 4
  %or255 = or i32 %add254, 1
  %head256 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.0, i32 0, i32 1
  store i32 %or255, i32* %head256, align 4
  %add.ptr257 = getelementptr inbounds i8, i8* %54, i32 %add254
  %prev_foot258 = bitcast i8* %add.ptr257 to i32*
  store i32 %add254, i32* %prev_foot258, align 4
  br label %if.end635

if.else259:                                       ; preds = %if.else250
  %and261 = and i32 %56, -8
  %add262 = add i32 %and261, %psize.0
  %shr263 = lshr i32 %56, 3
  %cmp264 = icmp ult i32 %56, 256
  br i1 %cmp264, label %if.then266, label %if.else323

if.then266:                                       ; preds = %if.else259
  %fd268 = getelementptr inbounds i8, i8* %mem, i32 %and5
  %62 = bitcast i8* %fd268 to %struct.malloc_chunk**
  %63 = load %struct.malloc_chunk*, %struct.malloc_chunk** %62, align 4
  %add.ptr6.sum247248 = or i32 %and5, 4
  %bk270 = getelementptr inbounds i8, i8* %mem, i32 %add.ptr6.sum247248
  %64 = bitcast i8* %bk270 to %struct.malloc_chunk**
  %65 = load %struct.malloc_chunk*, %struct.malloc_chunk** %64, align 4
  %shl273 = shl nuw nsw i32 %shr263, 1
  %arrayidx274 = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %shl273
  %66 = bitcast %struct.malloc_chunk** %arrayidx274 to %struct.malloc_chunk*
  %cmp275 = icmp eq %struct.malloc_chunk* %63, %66
  br i1 %cmp275, label %if.then290, label %lor.rhs277

lor.rhs277:                                       ; preds = %if.then266
  %67 = bitcast %struct.malloc_chunk* %63 to i8*
  %68 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp278 = icmp ult i8* %67, %68
  br i1 %cmp278, label %if.else321, label %land.rhs280

land.rhs280:                                      ; preds = %lor.rhs277
  %bk281 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %63, i32 0, i32 3
  %69 = load %struct.malloc_chunk*, %struct.malloc_chunk** %bk281, align 4
  %cmp282 = icmp eq %struct.malloc_chunk* %69, %4
  br i1 %cmp282, label %if.then290, label %if.else321

if.then290:                                       ; preds = %land.rhs280, %if.then266
  %cmp291 = icmp eq %struct.malloc_chunk* %65, %63
  br i1 %cmp291, label %if.then293, label %if.else297

if.then293:                                       ; preds = %if.then290
  %shl294 = shl i32 1, %shr263
  %neg295 = xor i32 %shl294, -1
  %70 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  %and296 = and i32 %70, %neg295
  store i32 %and296, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  br label %if.end474

if.else297:                                       ; preds = %if.then290
  %cmp300 = icmp eq %struct.malloc_chunk* %65, %66
  br i1 %cmp300, label %if.else297.if.then315_crit_edge, label %lor.rhs302

if.else297.if.then315_crit_edge:                  ; preds = %if.else297
  %fd317.pre = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %65, i32 0, i32 2
  br label %if.then315

lor.rhs302:                                       ; preds = %if.else297
  %71 = bitcast %struct.malloc_chunk* %65 to i8*
  %72 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp303 = icmp ult i8* %71, %72
  br i1 %cmp303, label %if.else318, label %land.rhs305

land.rhs305:                                      ; preds = %lor.rhs302
  %fd306 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %65, i32 0, i32 2
  %73 = load %struct.malloc_chunk*, %struct.malloc_chunk** %fd306, align 4
  %cmp307 = icmp eq %struct.malloc_chunk* %73, %4
  br i1 %cmp307, label %if.then315, label %if.else318

if.then315:                                       ; preds = %land.rhs305, %if.else297.if.then315_crit_edge
  %fd317.pre-phi = phi %struct.malloc_chunk** [ %fd317.pre, %if.else297.if.then315_crit_edge ], [ %fd306, %land.rhs305 ]
  %bk316 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %63, i32 0, i32 3
  store %struct.malloc_chunk* %65, %struct.malloc_chunk** %bk316, align 4
  store %struct.malloc_chunk* %63, %struct.malloc_chunk** %fd317.pre-phi, align 4
  br label %if.end474

if.else318:                                       ; preds = %land.rhs305, %lor.rhs302
  tail call void @abort() #6
  unreachable

if.else321:                                       ; preds = %land.rhs280, %lor.rhs277
  tail call void @abort() #6
  unreachable

if.else323:                                       ; preds = %if.else259
  %74 = bitcast i8* %add.ptr6 to %struct.malloc_tree_chunk*
  %add.ptr6.sum232 = add i32 %and5, 16
  %parent326 = getelementptr inbounds i8, i8* %mem, i32 %add.ptr6.sum232
  %75 = bitcast i8* %parent326 to %struct.malloc_tree_chunk**
  %76 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %75, align 4
  %add.ptr6.sum233234 = or i32 %and5, 4
  %bk328 = getelementptr inbounds i8, i8* %mem, i32 %add.ptr6.sum233234
  %77 = bitcast i8* %bk328 to %struct.malloc_tree_chunk**
  %78 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %77, align 4
  %cmp329 = icmp eq %struct.malloc_tree_chunk* %78, %74
  br i1 %cmp329, label %if.else354, label %if.then331

if.then331:                                       ; preds = %if.else323
  %fd333 = getelementptr inbounds i8, i8* %mem, i32 %and5
  %79 = bitcast i8* %fd333 to %struct.malloc_tree_chunk**
  %80 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %79, align 4
  %81 = bitcast %struct.malloc_tree_chunk* %80 to i8*
  %82 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp335 = icmp ult i8* %81, %82
  br i1 %cmp335, label %if.else352, label %land.lhs.true337

land.lhs.true337:                                 ; preds = %if.then331
  %bk338 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %80, i32 0, i32 3
  %83 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %bk338, align 4
  %cmp339 = icmp eq %struct.malloc_tree_chunk* %83, %74
  br i1 %cmp339, label %land.rhs341, label %if.else352

land.rhs341:                                      ; preds = %land.lhs.true337
  %fd342 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %78, i32 0, i32 2
  %84 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %fd342, align 4
  %cmp343 = icmp eq %struct.malloc_tree_chunk* %84, %74
  br i1 %cmp343, label %if.then349, label %if.else352

if.then349:                                       ; preds = %land.rhs341
  store %struct.malloc_tree_chunk* %78, %struct.malloc_tree_chunk** %bk338, align 4
  store %struct.malloc_tree_chunk* %80, %struct.malloc_tree_chunk** %fd342, align 4
  br label %if.end389

if.else352:                                       ; preds = %land.rhs341, %land.lhs.true337, %if.then331
  tail call void @abort() #6
  unreachable

if.else354:                                       ; preds = %if.else323
  %child356.sum = add i32 %and5, 12
  %arrayidx357 = getelementptr inbounds i8, i8* %mem, i32 %child356.sum
  %85 = bitcast i8* %arrayidx357 to %struct.malloc_tree_chunk**
  %86 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %85, align 4
  %cmp358 = icmp eq %struct.malloc_tree_chunk* %86, null
  br i1 %cmp358, label %lor.lhs.false360, label %while.cond367

lor.lhs.false360:                                 ; preds = %if.else354
  %add.ptr6.sum235 = add i32 %and5, 8
  %child356 = getelementptr inbounds i8, i8* %mem, i32 %add.ptr6.sum235
  %arrayidx362 = bitcast i8* %child356 to %struct.malloc_tree_chunk**
  %87 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx362, align 4
  %cmp363 = icmp eq %struct.malloc_tree_chunk* %87, null
  br i1 %cmp363, label %if.end389, label %while.cond367

while.cond367:                                    ; preds = %lor.rhs372, %while.cond367, %lor.lhs.false360, %if.else354
  %RP355.0 = phi %struct.malloc_tree_chunk** [ %arrayidx362, %lor.lhs.false360 ], [ %85, %if.else354 ], [ %arrayidx369, %while.cond367 ], [ %arrayidx374, %lor.rhs372 ]
  %R327.0 = phi %struct.malloc_tree_chunk* [ %87, %lor.lhs.false360 ], [ %86, %if.else354 ], [ %88, %while.cond367 ], [ %89, %lor.rhs372 ]
  %arrayidx369 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R327.0, i32 0, i32 4, i32 1
  %88 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx369, align 4
  %cmp370 = icmp eq %struct.malloc_tree_chunk* %88, null
  br i1 %cmp370, label %lor.rhs372, label %while.cond367

lor.rhs372:                                       ; preds = %while.cond367
  %arrayidx374 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R327.0, i32 0, i32 4, i32 0
  %89 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx374, align 4
  %cmp375 = icmp eq %struct.malloc_tree_chunk* %89, null
  br i1 %cmp375, label %while.end380, label %while.cond367

while.end380:                                     ; preds = %lor.rhs372
  %90 = bitcast %struct.malloc_tree_chunk** %RP355.0 to i8*
  %91 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp381 = icmp ult i8* %90, %91
  br i1 %cmp381, label %if.else386, label %if.then385

if.then385:                                       ; preds = %while.end380
  store %struct.malloc_tree_chunk* null, %struct.malloc_tree_chunk** %RP355.0, align 4
  br label %if.end389

if.else386:                                       ; preds = %while.end380
  tail call void @abort() #6
  unreachable

if.end389:                                        ; preds = %if.then385, %lor.lhs.false360, %if.then349
  %R327.1 = phi %struct.malloc_tree_chunk* [ %78, %if.then349 ], [ %R327.0, %if.then385 ], [ null, %lor.lhs.false360 ]
  %cmp390 = icmp eq %struct.malloc_tree_chunk* %76, null
  br i1 %cmp390, label %if.end474, label %if.then392

if.then392:                                       ; preds = %if.end389
  %add.ptr6.sum243 = add i32 %and5, 20
  %index394 = getelementptr inbounds i8, i8* %mem, i32 %add.ptr6.sum243
  %92 = bitcast i8* %index394 to i32*
  %93 = load i32, i32* %92, align 4
  %arrayidx395 = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 11, i32 %93
  %94 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx395, align 4
  %cmp396 = icmp eq %struct.malloc_tree_chunk* %74, %94
  br i1 %cmp396, label %if.then398, label %if.else407

if.then398:                                       ; preds = %if.then392
  store %struct.malloc_tree_chunk* %R327.1, %struct.malloc_tree_chunk** %arrayidx395, align 4
  %cond264 = icmp eq %struct.malloc_tree_chunk* %R327.1, null
  br i1 %cond264, label %if.end426.thread, label %if.then429

if.end426.thread:                                 ; preds = %if.then398
  %shl403 = shl i32 1, %93
  %neg404 = xor i32 %shl403, -1
  %95 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 1), align 4
  %and405 = and i32 %95, %neg404
  store i32 %and405, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 1), align 4
  br label %if.end474

if.else407:                                       ; preds = %if.then392
  %96 = bitcast %struct.malloc_tree_chunk* %76 to i8*
  %97 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp408 = icmp ult i8* %96, %97
  br i1 %cmp408, label %if.else424, label %if.then412

if.then412:                                       ; preds = %if.else407
  %arrayidx414 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %76, i32 0, i32 4, i32 0
  %98 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx414, align 4
  %cmp415 = icmp eq %struct.malloc_tree_chunk* %98, %74
  br i1 %cmp415, label %if.then417, label %if.else420

if.then417:                                       ; preds = %if.then412
  store %struct.malloc_tree_chunk* %R327.1, %struct.malloc_tree_chunk** %arrayidx414, align 4
  br label %if.end426

if.else420:                                       ; preds = %if.then412
  %arrayidx422 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %76, i32 0, i32 4, i32 1
  store %struct.malloc_tree_chunk* %R327.1, %struct.malloc_tree_chunk** %arrayidx422, align 4
  br label %if.end426

if.else424:                                       ; preds = %if.else407
  tail call void @abort() #6
  unreachable

if.end426:                                        ; preds = %if.else420, %if.then417
  %cmp427 = icmp eq %struct.malloc_tree_chunk* %R327.1, null
  br i1 %cmp427, label %if.end474, label %if.then429

if.then429:                                       ; preds = %if.end426, %if.then398
  %99 = bitcast %struct.malloc_tree_chunk* %R327.1 to i8*
  %100 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp430 = icmp ult i8* %99, %100
  br i1 %cmp430, label %if.else470, label %if.then434

if.then434:                                       ; preds = %if.then429
  %parent437 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R327.1, i32 0, i32 5
  store %struct.malloc_tree_chunk* %76, %struct.malloc_tree_chunk** %parent437, align 4
  %add.ptr6.sum244 = add i32 %and5, 8
  %child438 = getelementptr inbounds i8, i8* %mem, i32 %add.ptr6.sum244
  %arrayidx439 = bitcast i8* %child438 to %struct.malloc_tree_chunk**
  %101 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx439, align 4
  %cmp440 = icmp eq %struct.malloc_tree_chunk* %101, null
  br i1 %cmp440, label %if.end453, label %if.then442

if.then442:                                       ; preds = %if.then434
  %102 = bitcast %struct.malloc_tree_chunk* %101 to i8*
  %103 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp443 = icmp ult i8* %102, %103
  br i1 %cmp443, label %if.else451, label %if.then447

if.then447:                                       ; preds = %if.then442
  %arrayidx449 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R327.1, i32 0, i32 4, i32 0
  store %struct.malloc_tree_chunk* %101, %struct.malloc_tree_chunk** %arrayidx449, align 4
  %parent450 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %101, i32 0, i32 5
  store %struct.malloc_tree_chunk* %R327.1, %struct.malloc_tree_chunk** %parent450, align 4
  br label %if.end453

if.else451:                                       ; preds = %if.then442
  tail call void @abort() #6
  unreachable

if.end453:                                        ; preds = %if.then447, %if.then434
  %child438.sum = add i32 %and5, 12
  %arrayidx455 = getelementptr inbounds i8, i8* %mem, i32 %child438.sum
  %104 = bitcast i8* %arrayidx455 to %struct.malloc_tree_chunk**
  %105 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %104, align 4
  %cmp456 = icmp eq %struct.malloc_tree_chunk* %105, null
  br i1 %cmp456, label %if.end474, label %if.then458

if.then458:                                       ; preds = %if.end453
  %106 = bitcast %struct.malloc_tree_chunk* %105 to i8*
  %107 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp459 = icmp ult i8* %106, %107
  br i1 %cmp459, label %if.else467, label %if.then463

if.then463:                                       ; preds = %if.then458
  %arrayidx465 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R327.1, i32 0, i32 4, i32 1
  store %struct.malloc_tree_chunk* %105, %struct.malloc_tree_chunk** %arrayidx465, align 4
  %parent466 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %105, i32 0, i32 5
  store %struct.malloc_tree_chunk* %R327.1, %struct.malloc_tree_chunk** %parent466, align 4
  br label %if.end474

if.else467:                                       ; preds = %if.then458
  tail call void @abort() #6
  unreachable

if.else470:                                       ; preds = %if.then429
  tail call void @abort() #6
  unreachable

if.end474:                                        ; preds = %if.then463, %if.end453, %if.end426, %if.end426.thread, %if.end389, %if.then315, %if.then293
  %or475 = or i32 %add262, 1
  %head476 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.0, i32 0, i32 1
  store i32 %or475, i32* %head476, align 4
  %add.ptr477 = getelementptr inbounds i8, i8* %54, i32 %add262
  %prev_foot478 = bitcast i8* %add.ptr477 to i32*
  store i32 %add262, i32* %prev_foot478, align 4
  %108 = load %struct.malloc_chunk*, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 5), align 4
  %cmp479 = icmp eq %struct.malloc_chunk* %p.0, %108
  br i1 %cmp479, label %if.then481, label %if.end492

if.then481:                                       ; preds = %if.end474
  store i32 %add262, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 2), align 4
  br label %if.end635

if.else485:                                       ; preds = %if.then235
  %and487 = and i32 %56, -2
  store i32 %and487, i32* %55, align 4
  %or488 = or i32 %psize.0, 1
  %head489 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.0, i32 0, i32 1
  store i32 %or488, i32* %head489, align 4
  %add.ptr490 = getelementptr inbounds i8, i8* %54, i32 %psize.0
  %prev_foot491 = bitcast i8* %add.ptr490 to i32*
  store i32 %psize.0, i32* %prev_foot491, align 4
  br label %if.end492

if.end492:                                        ; preds = %if.else485, %if.end474
  %psize.1 = phi i32 [ %psize.0, %if.else485 ], [ %add262, %if.end474 ]
  %shr493 = lshr i32 %psize.1, 3
  %cmp494 = icmp ult i32 %psize.1, 256
  br i1 %cmp494, label %if.then496, label %if.else524

if.then496:                                       ; preds = %if.end492
  %shl500 = shl nuw nsw i32 %shr493, 1
  %arrayidx501 = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %shl500
  %109 = bitcast %struct.malloc_chunk** %arrayidx501 to %struct.malloc_chunk*
  %110 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  %shl503 = shl i32 1, %shr493
  %and504 = and i32 %110, %shl503
  %tobool505 = icmp eq i32 %and504, 0
  br i1 %tobool505, label %if.then506, label %if.else509

if.then506:                                       ; preds = %if.then496
  %or508 = or i32 %110, %shl503
  store i32 %or508, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  %arrayidx501.sum.pre = add i32 %shl500, 2
  %.pre = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %arrayidx501.sum.pre
  br label %if.end519

if.else509:                                       ; preds = %if.then496
  %arrayidx501.sum242 = add i32 %shl500, 2
  %111 = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %arrayidx501.sum242
  %112 = load %struct.malloc_chunk*, %struct.malloc_chunk** %111, align 4
  %113 = bitcast %struct.malloc_chunk* %112 to i8*
  %114 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp511 = icmp ult i8* %113, %114
  br i1 %cmp511, label %if.else517, label %if.end519

if.else517:                                       ; preds = %if.else509
  tail call void @abort() #6
  unreachable

if.end519:                                        ; preds = %if.else509, %if.then506
  %.pre-phi = phi %struct.malloc_chunk** [ %111, %if.else509 ], [ %.pre, %if.then506 ]
  %F502.0 = phi %struct.malloc_chunk* [ %112, %if.else509 ], [ %109, %if.then506 ]
  store %struct.malloc_chunk* %p.0, %struct.malloc_chunk** %.pre-phi, align 4
  %bk521 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %F502.0, i32 0, i32 3
  store %struct.malloc_chunk* %p.0, %struct.malloc_chunk** %bk521, align 4
  %fd522 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.0, i32 0, i32 2
  store %struct.malloc_chunk* %F502.0, %struct.malloc_chunk** %fd522, align 4
  %bk523 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.0, i32 0, i32 3
  store %struct.malloc_chunk* %109, %struct.malloc_chunk** %bk523, align 4
  br label %if.end635

if.else524:                                       ; preds = %if.end492
  %115 = bitcast %struct.malloc_chunk* %p.0 to %struct.malloc_tree_chunk*
  %shr527 = lshr i32 %psize.1, 8
  %cmp528 = icmp eq i32 %shr527, 0
  br i1 %cmp528, label %if.end558, label %if.else531

if.else531:                                       ; preds = %if.else524
  %cmp532 = icmp ugt i32 %psize.1, 16777215
  br i1 %cmp532, label %if.end558, label %if.else535

if.else535:                                       ; preds = %if.else531
  %sub = add i32 %shr527, 1048320
  %shr536 = lshr i32 %sub, 16
  %and537 = and i32 %shr536, 8
  %shl538 = shl i32 %shr527, %and537
  %sub539 = add i32 %shl538, 520192
  %shr540 = lshr i32 %sub539, 16
  %and541 = and i32 %shr540, 4
  %add542 = or i32 %and541, %and537
  %shl543 = shl i32 %shl538, %and541
  %sub544 = add i32 %shl543, 245760
  %shr545 = lshr i32 %sub544, 16
  %and546 = and i32 %shr545, 2
  %add547 = or i32 %add542, %and546
  %sub548 = sub i32 14, %add547
  %shl549 = shl i32 %shl543, %and546
  %shr550 = lshr i32 %shl549, 15
  %add551 = add i32 %sub548, %shr550
  %shl552 = shl nsw i32 %add551, 1
  %add553 = add i32 %add551, 7
  %shr554 = lshr i32 %psize.1, %add553
  %and555 = and i32 %shr554, 1
  %add556 = or i32 %and555, %shl552
  br label %if.end558

if.end558:                                        ; preds = %if.else535, %if.else531, %if.else524
  %I526.0 = phi i32 [ %add556, %if.else535 ], [ 0, %if.else524 ], [ 31, %if.else531 ]
  %arrayidx559 = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 11, i32 %I526.0
  %index560 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.0, i32 1, i32 3
  %I526.0.c = inttoptr i32 %I526.0 to %struct.malloc_chunk*
  store %struct.malloc_chunk* %I526.0.c, %struct.malloc_chunk** %index560, align 4
  %arrayidx562 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.0, i32 1, i32 1
  store i32 0, i32* %arrayidx562, align 4
  %116 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.0, i32 1, i32 0
  store i32 0, i32* %116, align 4
  %117 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 1), align 4
  %shl565 = shl i32 1, %I526.0
  %and566 = and i32 %117, %shl565
  %tobool567 = icmp eq i32 %and566, 0
  br i1 %tobool567, label %if.then568, label %if.else574

if.then568:                                       ; preds = %if.end558
  %or570 = or i32 %117, %shl565
  store i32 %or570, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 1), align 4
  store %struct.malloc_tree_chunk* %115, %struct.malloc_tree_chunk** %arrayidx559, align 4
  %parent571 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.0, i32 1, i32 2
  %.c = bitcast %struct.malloc_tree_chunk** %arrayidx559 to %struct.malloc_chunk*
  store %struct.malloc_chunk* %.c, %struct.malloc_chunk** %parent571, align 4
  %bk572 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.0, i32 0, i32 3
  store %struct.malloc_chunk* %p.0, %struct.malloc_chunk** %bk572, align 4
  %fd573 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.0, i32 0, i32 2
  store %struct.malloc_chunk* %p.0, %struct.malloc_chunk** %fd573, align 4
  br label %if.end627

if.else574:                                       ; preds = %if.end558
  %118 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx559, align 4
  %cmp576 = icmp eq i32 %I526.0, 31
  br i1 %cmp576, label %cond.end, label %cond.false

cond.false:                                       ; preds = %if.else574
  %shr578 = lshr i32 %I526.0, 1
  %sub581 = sub i32 25, %shr578
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %if.else574
  %cond = phi i32 [ %sub581, %cond.false ], [ 0, %if.else574 ]
  %head583266 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %118, i32 0, i32 1
  %119 = load i32, i32* %head583266, align 4
  %and584267 = and i32 %119, -8
  %cmp585268 = icmp eq i32 %and584267, %psize.1
  br i1 %cmp585268, label %if.else607, label %if.then587.lr.ph

if.then587.lr.ph:                                 ; preds = %cond.end
  %shl582 = shl i32 %psize.1, %cond
  br label %if.then587

for.cond:                                         ; preds = %if.then587
  %shl592 = shl i32 %K575.0270, 1
  %head583 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %121, i32 0, i32 1
  %120 = load i32, i32* %head583, align 4
  %and584 = and i32 %120, -8
  %cmp585 = icmp eq i32 %and584, %psize.1
  br i1 %cmp585, label %if.else607, label %if.then587

if.then587:                                       ; preds = %for.cond, %if.then587.lr.ph
  %K575.0270 = phi i32 [ %shl582, %if.then587.lr.ph ], [ %shl592, %for.cond ]
  %T.0269 = phi %struct.malloc_tree_chunk* [ %118, %if.then587.lr.ph ], [ %121, %for.cond ]
  %shr588 = lshr i32 %K575.0270, 31
  %arrayidx591 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %T.0269, i32 0, i32 4, i32 %shr588
  %121 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx591, align 4
  %cmp593 = icmp eq %struct.malloc_tree_chunk* %121, null
  br i1 %cmp593, label %if.else596, label %for.cond

if.else596:                                       ; preds = %if.then587
  %122 = bitcast %struct.malloc_tree_chunk** %arrayidx591 to i8*
  %123 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp597 = icmp ult i8* %122, %123
  br i1 %cmp597, label %if.else605, label %if.then601

if.then601:                                       ; preds = %if.else596
  store %struct.malloc_tree_chunk* %115, %struct.malloc_tree_chunk** %arrayidx591, align 4
  %parent602 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.0, i32 1, i32 2
  %T.0.c239 = bitcast %struct.malloc_tree_chunk* %T.0269 to %struct.malloc_chunk*
  store %struct.malloc_chunk* %T.0.c239, %struct.malloc_chunk** %parent602, align 4
  %bk603 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.0, i32 0, i32 3
  store %struct.malloc_chunk* %p.0, %struct.malloc_chunk** %bk603, align 4
  %fd604 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.0, i32 0, i32 2
  store %struct.malloc_chunk* %p.0, %struct.malloc_chunk** %fd604, align 4
  br label %if.end627

if.else605:                                       ; preds = %if.else596
  tail call void @abort() #6
  unreachable

if.else607:                                       ; preds = %for.cond, %cond.end
  %T.0.lcssa = phi %struct.malloc_tree_chunk* [ %118, %cond.end ], [ %121, %for.cond ]
  %fd609 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %T.0.lcssa, i32 0, i32 2
  %124 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %fd609, align 4
  %125 = bitcast %struct.malloc_tree_chunk* %T.0.lcssa to i8*
  %126 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp610 = icmp ult i8* %125, %126
  br i1 %cmp610, label %if.else625, label %land.rhs612

land.rhs612:                                      ; preds = %if.else607
  %127 = bitcast %struct.malloc_tree_chunk* %124 to i8*
  %cmp613 = icmp ult i8* %127, %126
  br i1 %cmp613, label %if.else625, label %if.then619

if.then619:                                       ; preds = %land.rhs612
  %bk620 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %124, i32 0, i32 3
  store %struct.malloc_tree_chunk* %115, %struct.malloc_tree_chunk** %bk620, align 4
  store %struct.malloc_tree_chunk* %115, %struct.malloc_tree_chunk** %fd609, align 4
  %fd622 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.0, i32 0, i32 2
  %.c238 = bitcast %struct.malloc_tree_chunk* %124 to %struct.malloc_chunk*
  store %struct.malloc_chunk* %.c238, %struct.malloc_chunk** %fd622, align 4
  %bk623 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.0, i32 0, i32 3
  %T.0.c = bitcast %struct.malloc_tree_chunk* %T.0.lcssa to %struct.malloc_chunk*
  store %struct.malloc_chunk* %T.0.c, %struct.malloc_chunk** %bk623, align 4
  %parent624 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.0, i32 1, i32 2
  store %struct.malloc_chunk* null, %struct.malloc_chunk** %parent624, align 4
  br label %if.end627

if.else625:                                       ; preds = %land.rhs612, %if.else607
  tail call void @abort() #6
  unreachable

if.end627:                                        ; preds = %if.then619, %if.then601, %if.then568
  %128 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 8), align 4
  %dec = add i32 %128, -1
  store i32 %dec, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 8), align 4
  %cmp628 = icmp eq i32 %dec, 0
  br i1 %cmp628, label %while.cond.i, label %if.end635

while.cond.i:                                     ; preds = %while.cond.i, %if.end627
  %sp.0.in.i = phi %struct.malloc_segment** [ %next4.i, %while.cond.i ], [ getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 16, i32 2), %if.end627 ]
  %sp.0.i = load %struct.malloc_segment*, %struct.malloc_segment** %sp.0.in.i, align 4
  %cmp.i = icmp eq %struct.malloc_segment* %sp.0.i, null
  %next4.i = getelementptr inbounds %struct.malloc_segment, %struct.malloc_segment* %sp.0.i, i32 0, i32 2
  br i1 %cmp.i, label %release_unused_segments.exit, label %while.cond.i

release_unused_segments.exit:                     ; preds = %while.cond.i
  store i32 -1, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 8), align 4
  br label %if.end635

erroraction:                                      ; preds = %land.rhs227, %if.end224, %if.else, %land.rhs, %if.then
  tail call void @abort() #6
  unreachable

if.end635:                                        ; preds = %release_unused_segments.exit, %if.end627, %if.end519, %if.then481, %if.then253, %if.then248, %if.then242, %if.then213, %if.then10, %entry
  ret void
}

; Function Attrs: nounwind
define weak i8* @calloc(i32 %n_elements, i32 %elem_size) #0 {
entry:
  %cmp = icmp eq i32 %n_elements, 0
  br i1 %cmp, label %if.end3, label %if.then

if.then:                                          ; preds = %entry
  %mul = mul i32 %elem_size, %n_elements
  %or = or i32 %elem_size, %n_elements
  %tobool = icmp ugt i32 %or, 65535
  br i1 %tobool, label %land.lhs.true, label %if.end3

land.lhs.true:                                    ; preds = %if.then
  %div = udiv i32 %mul, %n_elements
  %cmp1 = icmp eq i32 %div, %elem_size
  %mul. = select i1 %cmp1, i32 %mul, i32 -1
  br label %if.end3

if.end3:                                          ; preds = %land.lhs.true, %if.then, %entry
  %req.0 = phi i32 [ %mul, %if.then ], [ 0, %entry ], [ %mul., %land.lhs.true ]
  %call = tail call i8* @malloc(i32 %req.0)
  %cmp4 = icmp eq i8* %call, null
  br i1 %cmp4, label %if.end9, label %land.lhs.true5

land.lhs.true5:                                   ; preds = %if.end3
  %head = getelementptr inbounds i8, i8* %call, i32 -4
  %0 = bitcast i8* %head to i32*
  %1 = load i32, i32* %0, align 4
  %and6 = and i32 %1, 3
  %cmp7 = icmp eq i32 %and6, 0
  br i1 %cmp7, label %if.end9, label %if.then8

if.then8:                                         ; preds = %land.lhs.true5
  tail call void @llvm.memset.p0i8.i32(i8* %call, i8 0, i32 %req.0, i32 1, i1 false)
  br label %if.end9

if.end9:                                          ; preds = %if.then8, %land.lhs.true5, %if.end3
  ret i8* %call
}

; Function Attrs: nounwind
declare void @llvm.memset.p0i8.i32(i8* nocapture, i8, i32, i32, i1) #1

; Function Attrs: nounwind
define weak i8* @realloc(i8* %oldmem, i32 %bytes) #0 {
entry:
  %cmp = icmp eq i8* %oldmem, null
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %call = tail call i8* @malloc(i32 %bytes)
  br label %if.end27

if.else:                                          ; preds = %entry
  %cmp1 = icmp ugt i32 %bytes, -65
  br i1 %cmp1, label %if.then2, label %if.else4

if.then2:                                         ; preds = %if.else
  %call3 = tail call i32* @__errno_location() #7
  store i32 12, i32* %call3, align 4
  br label %if.end27

if.else4:                                         ; preds = %if.else
  %cmp5 = icmp ult i32 %bytes, 11
  br i1 %cmp5, label %cond.end, label %cond.false

cond.false:                                       ; preds = %if.else4
  %add6 = add i32 %bytes, 11
  %and = and i32 %add6, -8
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %if.else4
  %cond = phi i32 [ %and, %cond.false ], [ 16, %if.else4 ]
  %add.ptr = getelementptr inbounds i8, i8* %oldmem, i32 -8
  %0 = bitcast i8* %add.ptr to %struct.malloc_chunk*
  %call7 = tail call fastcc %struct.malloc_chunk* @try_realloc_chunk(%struct.malloc_chunk* %0, i32 %cond)
  %cmp8 = icmp eq %struct.malloc_chunk* %call7, null
  br i1 %cmp8, label %if.else11, label %if.then9

if.then9:                                         ; preds = %cond.end
  %add.ptr10 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %call7, i32 0, i32 2
  %1 = bitcast %struct.malloc_chunk** %add.ptr10 to i8*
  br label %if.end27

if.else11:                                        ; preds = %cond.end
  %call12 = tail call i8* @malloc(i32 %bytes)
  %cmp13 = icmp eq i8* %call12, null
  br i1 %cmp13, label %if.end27, label %if.then14

if.then14:                                        ; preds = %if.else11
  %head = getelementptr inbounds i8, i8* %oldmem, i32 -4
  %2 = bitcast i8* %head to i32*
  %3 = load i32, i32* %2, align 4
  %and15 = and i32 %3, -8
  %and17 = and i32 %3, 3
  %cmp18 = icmp eq i32 %and17, 0
  %cond19 = select i1 %cmp18, i32 8, i32 4
  %sub = sub i32 %and15, %cond19
  %cmp20 = icmp ult i32 %sub, %bytes
  %cond24 = select i1 %cmp20, i32 %sub, i32 %bytes
  tail call void @llvm.memcpy.p0i8.p0i8.i32(i8* %call12, i8* %oldmem, i32 %cond24, i32 1, i1 false)
  tail call void @free(i8* %oldmem)
  br label %if.end27

if.end27:                                         ; preds = %if.then14, %if.else11, %if.then9, %if.then2, %if.then
  %mem.0 = phi i8* [ %call, %if.then ], [ null, %if.then2 ], [ %1, %if.then9 ], [ %call12, %if.then14 ], [ null, %if.else11 ]
  ret i8* %mem.0
}

; Function Attrs: nounwind readnone
declare i32* @__errno_location() #3

; Function Attrs: nounwind
define internal fastcc %struct.malloc_chunk* @try_realloc_chunk(%struct.malloc_chunk* %p, i32 %nb) #0 {
entry:
  %head = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p, i32 0, i32 1
  %0 = load i32, i32* %head, align 4
  %and = and i32 %0, -8
  %1 = bitcast %struct.malloc_chunk* %p to i8*
  %add.ptr = getelementptr inbounds i8, i8* %1, i32 %and
  %2 = bitcast i8* %add.ptr to %struct.malloc_chunk*
  %3 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp = icmp ult i8* %1, %3
  br i1 %cmp, label %if.else327, label %land.lhs.true

land.lhs.true:                                    ; preds = %entry
  %and2 = and i32 %0, 3
  %cmp3 = icmp ne i32 %and2, 1
  %cmp5 = icmp ult i8* %1, %add.ptr
  %or.cond = and i1 %cmp3, %cmp5
  br i1 %or.cond, label %land.rhs, label %if.else327

land.rhs:                                         ; preds = %land.lhs.true
  %add.ptr.sum1516 = or i32 %and, 4
  %head6 = getelementptr inbounds i8, i8* %1, i32 %add.ptr.sum1516
  %4 = bitcast i8* %head6 to i32*
  %5 = load i32, i32* %4, align 4
  %and7 = and i32 %5, 1
  %tobool = icmp eq i32 %and7, 0
  br i1 %tobool, label %if.else327, label %if.then

if.then:                                          ; preds = %land.rhs
  %cmp11 = icmp eq i32 %and2, 0
  br i1 %cmp11, label %if.then12, label %if.else

if.then12:                                        ; preds = %if.then
  %cmp.i = icmp ult i32 %nb, 256
  br i1 %cmp.i, label %if.end328, label %if.end.i

if.end.i:                                         ; preds = %if.then12
  %add.i = add i32 %nb, 4
  %cmp1.i = icmp ult i32 %and, %add.i
  br i1 %cmp1.i, label %if.end34.i, label %land.lhs.true.i

land.lhs.true.i:                                  ; preds = %if.end.i
  %sub.i = sub i32 %and, %nb
  %6 = load i32, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 2), align 4
  %shl.i = shl i32 %6, 1
  %cmp2.i = icmp ugt i32 %sub.i, %shl.i
  br i1 %cmp2.i, label %if.end34.i, label %if.end328

if.end34.i:                                       ; preds = %land.lhs.true.i, %if.end.i
  br label %if.end328

if.else:                                          ; preds = %if.then
  %cmp13 = icmp ult i32 %and, %nb
  br i1 %cmp13, label %if.else33, label %if.then14

if.then14:                                        ; preds = %if.else
  %sub = sub i32 %and, %nb
  %cmp15 = icmp ugt i32 %sub, 15
  br i1 %cmp15, label %if.then16, label %if.end328

if.then16:                                        ; preds = %if.then14
  %add.ptr17 = getelementptr inbounds i8, i8* %1, i32 %nb
  %7 = bitcast i8* %add.ptr17 to %struct.malloc_chunk*
  %and19 = and i32 %0, 1
  %or = or i32 %and19, %nb
  %or20 = or i32 %or, 2
  store i32 %or20, i32* %head, align 4
  %add.ptr17.sum = add i32 %nb, 4
  %head23 = getelementptr inbounds i8, i8* %1, i32 %add.ptr17.sum
  %8 = bitcast i8* %head23 to i32*
  %or28 = or i32 %sub, 3
  store i32 %or28, i32* %8, align 4
  %9 = load i32, i32* %4, align 4
  %or32 = or i32 %9, 1
  store i32 %or32, i32* %4, align 4
  tail call fastcc void @dispose_chunk(%struct.malloc_chunk* %7, i32 %sub)
  br label %if.end328

if.else33:                                        ; preds = %if.else
  %10 = load %struct.malloc_chunk*, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 6), align 4
  %cmp34 = icmp eq %struct.malloc_chunk* %2, %10
  br i1 %cmp34, label %if.then35, label %if.else55

if.then35:                                        ; preds = %if.else33
  %11 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 3), align 4
  %add = add i32 %11, %and
  %cmp36 = icmp ugt i32 %add, %nb
  br i1 %cmp36, label %if.then37, label %if.end328

if.then37:                                        ; preds = %if.then35
  %sub40 = sub i32 %add, %nb
  %add.ptr41 = getelementptr inbounds i8, i8* %1, i32 %nb
  %12 = bitcast i8* %add.ptr41 to %struct.malloc_chunk*
  %and43 = and i32 %0, 1
  %or44 = or i32 %and43, %nb
  %or45 = or i32 %or44, 2
  store i32 %or45, i32* %head, align 4
  %add.ptr41.sum = add i32 %nb, 4
  %head48 = getelementptr inbounds i8, i8* %1, i32 %add.ptr41.sum
  %13 = bitcast i8* %head48 to i32*
  %or50 = or i32 %sub40, 1
  store i32 %or50, i32* %13, align 4
  store %struct.malloc_chunk* %12, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 6), align 4
  store i32 %sub40, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 3), align 4
  br label %if.end328

if.else55:                                        ; preds = %if.else33
  %14 = load %struct.malloc_chunk*, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 5), align 4
  %cmp56 = icmp eq %struct.malloc_chunk* %2, %14
  br i1 %cmp56, label %if.then57, label %if.else98

if.then57:                                        ; preds = %if.else55
  %15 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 2), align 4
  %add58 = add i32 %15, %and
  %cmp59 = icmp ult i32 %add58, %nb
  br i1 %cmp59, label %if.end328, label %if.then60

if.then60:                                        ; preds = %if.then57
  %sub62 = sub i32 %add58, %nb
  %cmp63 = icmp ugt i32 %sub62, 15
  br i1 %cmp63, label %if.then64, label %if.else83

if.then64:                                        ; preds = %if.then60
  %add.ptr66 = getelementptr inbounds i8, i8* %1, i32 %nb
  %16 = bitcast i8* %add.ptr66 to %struct.malloc_chunk*
  %add.ptr67 = getelementptr inbounds i8, i8* %1, i32 %add58
  %and69 = and i32 %0, 1
  %or70 = or i32 %and69, %nb
  %or71 = or i32 %or70, 2
  store i32 %or71, i32* %head, align 4
  %add.ptr66.sum = add i32 %nb, 4
  %head74 = getelementptr inbounds i8, i8* %1, i32 %add.ptr66.sum
  %17 = bitcast i8* %head74 to i32*
  %or76 = or i32 %sub62, 1
  store i32 %or76, i32* %17, align 4
  %prev_foot = bitcast i8* %add.ptr67 to i32*
  store i32 %sub62, i32* %prev_foot, align 4
  %add.ptr67.sum = add i32 %add58, 4
  %head79 = getelementptr inbounds i8, i8* %1, i32 %add.ptr67.sum
  %18 = bitcast i8* %head79 to i32*
  %19 = load i32, i32* %18, align 4
  %and80 = and i32 %19, -2
  store i32 %and80, i32* %18, align 4
  br label %if.end96

if.else83:                                        ; preds = %if.then60
  %and87 = and i32 %0, 1
  %or88 = or i32 %and87, %add58
  %or89 = or i32 %or88, 2
  store i32 %or89, i32* %head, align 4
  %add.ptr91.sum = add i32 %add58, 4
  %head92 = getelementptr inbounds i8, i8* %1, i32 %add.ptr91.sum
  %20 = bitcast i8* %head92 to i32*
  %21 = load i32, i32* %20, align 4
  %or93 = or i32 %21, 1
  store i32 %or93, i32* %20, align 4
  br label %if.end96

if.end96:                                         ; preds = %if.else83, %if.then64
  %storemerge12 = phi i32 [ %sub62, %if.then64 ], [ 0, %if.else83 ]
  %storemerge = phi %struct.malloc_chunk* [ %16, %if.then64 ], [ null, %if.else83 ]
  store i32 %storemerge12, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 2), align 4
  store %struct.malloc_chunk* %storemerge, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 5), align 4
  br label %if.end328

if.else98:                                        ; preds = %if.else55
  %and100 = and i32 %5, 2
  %tobool101 = icmp eq i32 %and100, 0
  br i1 %tobool101, label %if.then102, label %if.end328

if.then102:                                       ; preds = %if.else98
  %and104 = and i32 %5, -8
  %add105 = add i32 %and104, %and
  %cmp106 = icmp ult i32 %add105, %nb
  br i1 %cmp106, label %if.end328, label %if.then107

if.then107:                                       ; preds = %if.then102
  %sub110 = sub i32 %add105, %nb
  %shr = lshr i32 %5, 3
  %cmp111 = icmp ult i32 %5, 256
  br i1 %cmp111, label %if.then112, label %if.else154

if.then112:                                       ; preds = %if.then107
  %add.ptr.sum10 = add i32 %and, 8
  %fd = getelementptr inbounds i8, i8* %1, i32 %add.ptr.sum10
  %22 = bitcast i8* %fd to %struct.malloc_chunk**
  %23 = load %struct.malloc_chunk*, %struct.malloc_chunk** %22, align 4
  %add.ptr.sum11 = add i32 %and, 12
  %bk = getelementptr inbounds i8, i8* %1, i32 %add.ptr.sum11
  %24 = bitcast i8* %bk to %struct.malloc_chunk**
  %25 = load %struct.malloc_chunk*, %struct.malloc_chunk** %24, align 4
  %shl = shl nuw nsw i32 %shr, 1
  %arrayidx = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %shl
  %26 = bitcast %struct.malloc_chunk** %arrayidx to %struct.malloc_chunk*
  %cmp114 = icmp eq %struct.malloc_chunk* %23, %26
  br i1 %cmp114, label %if.then124, label %lor.rhs

lor.rhs:                                          ; preds = %if.then112
  %27 = bitcast %struct.malloc_chunk* %23 to i8*
  %cmp116 = icmp ult i8* %27, %3
  br i1 %cmp116, label %if.else152, label %land.rhs117

land.rhs117:                                      ; preds = %lor.rhs
  %bk118 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %23, i32 0, i32 3
  %28 = load %struct.malloc_chunk*, %struct.malloc_chunk** %bk118, align 4
  %cmp119 = icmp eq %struct.malloc_chunk* %28, %2
  br i1 %cmp119, label %if.then124, label %if.else152

if.then124:                                       ; preds = %land.rhs117, %if.then112
  %cmp125 = icmp eq %struct.malloc_chunk* %25, %23
  br i1 %cmp125, label %if.then126, label %if.else129

if.then126:                                       ; preds = %if.then124
  %shl127 = shl i32 1, %shr
  %neg = xor i32 %shl127, -1
  %29 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  %and128 = and i32 %29, %neg
  store i32 %and128, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  br label %if.end287

if.else129:                                       ; preds = %if.then124
  %cmp133 = icmp eq %struct.malloc_chunk* %25, %26
  br i1 %cmp133, label %if.else129.if.then146_crit_edge, label %lor.rhs134

if.else129.if.then146_crit_edge:                  ; preds = %if.else129
  %fd148.pre = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %25, i32 0, i32 2
  br label %if.then146

lor.rhs134:                                       ; preds = %if.else129
  %30 = bitcast %struct.malloc_chunk* %25 to i8*
  %cmp136 = icmp ult i8* %30, %3
  br i1 %cmp136, label %if.else149, label %land.rhs137

land.rhs137:                                      ; preds = %lor.rhs134
  %fd138 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %25, i32 0, i32 2
  %31 = load %struct.malloc_chunk*, %struct.malloc_chunk** %fd138, align 4
  %cmp139 = icmp eq %struct.malloc_chunk* %31, %2
  br i1 %cmp139, label %if.then146, label %if.else149

if.then146:                                       ; preds = %land.rhs137, %if.else129.if.then146_crit_edge
  %fd148.pre-phi = phi %struct.malloc_chunk** [ %fd148.pre, %if.else129.if.then146_crit_edge ], [ %fd138, %land.rhs137 ]
  %bk147 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %23, i32 0, i32 3
  store %struct.malloc_chunk* %25, %struct.malloc_chunk** %bk147, align 4
  store %struct.malloc_chunk* %23, %struct.malloc_chunk** %fd148.pre-phi, align 4
  br label %if.end287

if.else149:                                       ; preds = %land.rhs137, %lor.rhs134
  tail call void @abort() #6
  unreachable

if.else152:                                       ; preds = %land.rhs117, %lor.rhs
  tail call void @abort() #6
  unreachable

if.else154:                                       ; preds = %if.then107
  %32 = bitcast i8* %add.ptr to %struct.malloc_tree_chunk*
  %add.ptr.sum = add i32 %and, 24
  %parent = getelementptr inbounds i8, i8* %1, i32 %add.ptr.sum
  %33 = bitcast i8* %parent to %struct.malloc_tree_chunk**
  %34 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %33, align 4
  %add.ptr.sum2 = add i32 %and, 12
  %bk155 = getelementptr inbounds i8, i8* %1, i32 %add.ptr.sum2
  %35 = bitcast i8* %bk155 to %struct.malloc_tree_chunk**
  %36 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %35, align 4
  %cmp156 = icmp eq %struct.malloc_tree_chunk* %36, %32
  br i1 %cmp156, label %if.else178, label %if.then157

if.then157:                                       ; preds = %if.else154
  %add.ptr.sum9 = add i32 %and, 8
  %fd159 = getelementptr inbounds i8, i8* %1, i32 %add.ptr.sum9
  %37 = bitcast i8* %fd159 to %struct.malloc_tree_chunk**
  %38 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %37, align 4
  %39 = bitcast %struct.malloc_tree_chunk* %38 to i8*
  %cmp162 = icmp ult i8* %39, %3
  br i1 %cmp162, label %if.else176, label %land.lhs.true163

land.lhs.true163:                                 ; preds = %if.then157
  %bk164 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %38, i32 0, i32 3
  %40 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %bk164, align 4
  %cmp165 = icmp eq %struct.malloc_tree_chunk* %40, %32
  br i1 %cmp165, label %land.rhs166, label %if.else176

land.rhs166:                                      ; preds = %land.lhs.true163
  %fd167 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %36, i32 0, i32 2
  %41 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %fd167, align 4
  %cmp168 = icmp eq %struct.malloc_tree_chunk* %41, %32
  br i1 %cmp168, label %if.then173, label %if.else176

if.then173:                                       ; preds = %land.rhs166
  store %struct.malloc_tree_chunk* %36, %struct.malloc_tree_chunk** %bk164, align 4
  store %struct.malloc_tree_chunk* %38, %struct.malloc_tree_chunk** %fd167, align 4
  br label %if.end202

if.else176:                                       ; preds = %land.rhs166, %land.lhs.true163, %if.then157
  tail call void @abort() #6
  unreachable

if.else178:                                       ; preds = %if.else154
  %child.sum = add i32 %and, 20
  %arrayidx179 = getelementptr inbounds i8, i8* %1, i32 %child.sum
  %42 = bitcast i8* %arrayidx179 to %struct.malloc_tree_chunk**
  %43 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %42, align 4
  %cmp180 = icmp eq %struct.malloc_tree_chunk* %43, null
  br i1 %cmp180, label %lor.lhs.false, label %while.cond

lor.lhs.false:                                    ; preds = %if.else178
  %add.ptr.sum3 = add i32 %and, 16
  %child = getelementptr inbounds i8, i8* %1, i32 %add.ptr.sum3
  %arrayidx182 = bitcast i8* %child to %struct.malloc_tree_chunk**
  %44 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx182, align 4
  %cmp183 = icmp eq %struct.malloc_tree_chunk* %44, null
  br i1 %cmp183, label %if.end202, label %while.cond

while.cond:                                       ; preds = %lor.rhs188, %while.cond, %lor.lhs.false, %if.else178
  %RP.0 = phi %struct.malloc_tree_chunk** [ %arrayidx182, %lor.lhs.false ], [ %42, %if.else178 ], [ %arrayidx186, %while.cond ], [ %arrayidx190, %lor.rhs188 ]
  %R.0 = phi %struct.malloc_tree_chunk* [ %44, %lor.lhs.false ], [ %43, %if.else178 ], [ %45, %while.cond ], [ %46, %lor.rhs188 ]
  %arrayidx186 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.0, i32 0, i32 4, i32 1
  %45 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx186, align 4
  %cmp187 = icmp eq %struct.malloc_tree_chunk* %45, null
  br i1 %cmp187, label %lor.rhs188, label %while.cond

lor.rhs188:                                       ; preds = %while.cond
  %arrayidx190 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.0, i32 0, i32 4, i32 0
  %46 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx190, align 4
  %cmp191 = icmp eq %struct.malloc_tree_chunk* %46, null
  br i1 %cmp191, label %while.end, label %while.cond

while.end:                                        ; preds = %lor.rhs188
  %47 = bitcast %struct.malloc_tree_chunk** %RP.0 to i8*
  %cmp195 = icmp ult i8* %47, %3
  br i1 %cmp195, label %if.else199, label %if.then198

if.then198:                                       ; preds = %while.end
  store %struct.malloc_tree_chunk* null, %struct.malloc_tree_chunk** %RP.0, align 4
  br label %if.end202

if.else199:                                       ; preds = %while.end
  tail call void @abort() #6
  unreachable

if.end202:                                        ; preds = %if.then198, %lor.lhs.false, %if.then173
  %R.1 = phi %struct.malloc_tree_chunk* [ %36, %if.then173 ], [ %R.0, %if.then198 ], [ null, %lor.lhs.false ]
  %cmp203 = icmp eq %struct.malloc_tree_chunk* %34, null
  br i1 %cmp203, label %if.end287, label %if.then205

if.then205:                                       ; preds = %if.end202
  %add.ptr.sum7 = add i32 %and, 28
  %index = getelementptr inbounds i8, i8* %1, i32 %add.ptr.sum7
  %48 = bitcast i8* %index to i32*
  %49 = load i32, i32* %48, align 4
  %arrayidx206 = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 11, i32 %49
  %50 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx206, align 4
  %cmp207 = icmp eq %struct.malloc_tree_chunk* %32, %50
  br i1 %cmp207, label %if.then209, label %if.else218

if.then209:                                       ; preds = %if.then205
  store %struct.malloc_tree_chunk* %R.1, %struct.malloc_tree_chunk** %arrayidx206, align 4
  %cond = icmp eq %struct.malloc_tree_chunk* %R.1, null
  br i1 %cond, label %if.end238.thread, label %if.then241

if.end238.thread:                                 ; preds = %if.then209
  %shl214 = shl i32 1, %49
  %neg215 = xor i32 %shl214, -1
  %51 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 1), align 4
  %and216 = and i32 %51, %neg215
  store i32 %and216, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 1), align 4
  br label %if.end287

if.else218:                                       ; preds = %if.then205
  %52 = bitcast %struct.malloc_tree_chunk* %34 to i8*
  %53 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp220 = icmp ult i8* %52, %53
  br i1 %cmp220, label %if.else236, label %if.then224

if.then224:                                       ; preds = %if.else218
  %arrayidx226 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %34, i32 0, i32 4, i32 0
  %54 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx226, align 4
  %cmp227 = icmp eq %struct.malloc_tree_chunk* %54, %32
  br i1 %cmp227, label %if.then229, label %if.else232

if.then229:                                       ; preds = %if.then224
  store %struct.malloc_tree_chunk* %R.1, %struct.malloc_tree_chunk** %arrayidx226, align 4
  br label %if.end238

if.else232:                                       ; preds = %if.then224
  %arrayidx234 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %34, i32 0, i32 4, i32 1
  store %struct.malloc_tree_chunk* %R.1, %struct.malloc_tree_chunk** %arrayidx234, align 4
  br label %if.end238

if.else236:                                       ; preds = %if.else218
  tail call void @abort() #6
  unreachable

if.end238:                                        ; preds = %if.else232, %if.then229
  %cmp239 = icmp eq %struct.malloc_tree_chunk* %R.1, null
  br i1 %cmp239, label %if.end287, label %if.then241

if.then241:                                       ; preds = %if.end238, %if.then209
  %55 = bitcast %struct.malloc_tree_chunk* %R.1 to i8*
  %56 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp243 = icmp ult i8* %55, %56
  br i1 %cmp243, label %if.else283, label %if.then247

if.then247:                                       ; preds = %if.then241
  %parent248 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.1, i32 0, i32 5
  store %struct.malloc_tree_chunk* %34, %struct.malloc_tree_chunk** %parent248, align 4
  %add.ptr.sum8 = add i32 %and, 16
  %child249 = getelementptr inbounds i8, i8* %1, i32 %add.ptr.sum8
  %arrayidx250 = bitcast i8* %child249 to %struct.malloc_tree_chunk**
  %57 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx250, align 4
  %cmp251 = icmp eq %struct.malloc_tree_chunk* %57, null
  br i1 %cmp251, label %if.end265, label %if.then253

if.then253:                                       ; preds = %if.then247
  %58 = bitcast %struct.malloc_tree_chunk* %57 to i8*
  %59 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp255 = icmp ult i8* %58, %59
  br i1 %cmp255, label %if.else263, label %if.then259

if.then259:                                       ; preds = %if.then253
  %arrayidx261 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.1, i32 0, i32 4, i32 0
  store %struct.malloc_tree_chunk* %57, %struct.malloc_tree_chunk** %arrayidx261, align 4
  %parent262 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %57, i32 0, i32 5
  store %struct.malloc_tree_chunk* %R.1, %struct.malloc_tree_chunk** %parent262, align 4
  br label %if.end265

if.else263:                                       ; preds = %if.then253
  tail call void @abort() #6
  unreachable

if.end265:                                        ; preds = %if.then259, %if.then247
  %child249.sum = add i32 %and, 20
  %arrayidx267 = getelementptr inbounds i8, i8* %1, i32 %child249.sum
  %60 = bitcast i8* %arrayidx267 to %struct.malloc_tree_chunk**
  %61 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %60, align 4
  %cmp268 = icmp eq %struct.malloc_tree_chunk* %61, null
  br i1 %cmp268, label %if.end287, label %if.then270

if.then270:                                       ; preds = %if.end265
  %62 = bitcast %struct.malloc_tree_chunk* %61 to i8*
  %63 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp272 = icmp ult i8* %62, %63
  br i1 %cmp272, label %if.else280, label %if.then276

if.then276:                                       ; preds = %if.then270
  %arrayidx278 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.1, i32 0, i32 4, i32 1
  store %struct.malloc_tree_chunk* %61, %struct.malloc_tree_chunk** %arrayidx278, align 4
  %parent279 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %61, i32 0, i32 5
  store %struct.malloc_tree_chunk* %R.1, %struct.malloc_tree_chunk** %parent279, align 4
  br label %if.end287

if.else280:                                       ; preds = %if.then270
  tail call void @abort() #6
  unreachable

if.else283:                                       ; preds = %if.then241
  tail call void @abort() #6
  unreachable

if.end287:                                        ; preds = %if.then276, %if.end265, %if.end238, %if.end238.thread, %if.end202, %if.then146, %if.then126
  %cmp288 = icmp ult i32 %sub110, 16
  br i1 %cmp288, label %if.then290, label %if.else301

if.then290:                                       ; preds = %if.end287
  %64 = load i32, i32* %head, align 4
  %and294 = and i32 %64, 1
  %or295 = or i32 %add105, %and294
  %or296 = or i32 %or295, 2
  store i32 %or296, i32* %head, align 4
  %add.ptr298.sum6 = or i32 %add105, 4
  %head299 = getelementptr inbounds i8, i8* %1, i32 %add.ptr298.sum6
  %65 = bitcast i8* %head299 to i32*
  %66 = load i32, i32* %65, align 4
  %or300 = or i32 %66, 1
  store i32 %or300, i32* %65, align 4
  br label %if.end328

if.else301:                                       ; preds = %if.end287
  %add.ptr303 = getelementptr inbounds i8, i8* %1, i32 %nb
  %67 = bitcast i8* %add.ptr303 to %struct.malloc_chunk*
  %68 = load i32, i32* %head, align 4
  %and305 = and i32 %68, 1
  %or306 = or i32 %and305, %nb
  %or307 = or i32 %or306, 2
  store i32 %or307, i32* %head, align 4
  %add.ptr303.sum = add i32 %nb, 4
  %head310 = getelementptr inbounds i8, i8* %1, i32 %add.ptr303.sum
  %69 = bitcast i8* %head310 to i32*
  %or315 = or i32 %sub110, 3
  store i32 %or315, i32* %69, align 4
  %add.ptr317.sum5 = or i32 %add105, 4
  %head318 = getelementptr inbounds i8, i8* %1, i32 %add.ptr317.sum5
  %70 = bitcast i8* %head318 to i32*
  %71 = load i32, i32* %70, align 4
  %or319 = or i32 %71, 1
  store i32 %or319, i32* %70, align 4
  tail call fastcc void @dispose_chunk(%struct.malloc_chunk* %67, i32 %sub110)
  br label %if.end328

if.else327:                                       ; preds = %land.rhs, %land.lhs.true, %entry
  tail call void @abort() #6
  unreachable

if.end328:                                        ; preds = %if.else301, %if.then290, %if.then102, %if.else98, %if.end96, %if.then57, %if.then37, %if.then35, %if.then16, %if.then14, %if.end34.i, %land.lhs.true.i, %if.then12
  %newp.0 = phi %struct.malloc_chunk* [ %p, %if.then37 ], [ null, %if.then35 ], [ %p, %if.end96 ], [ null, %if.then57 ], [ null, %if.else98 ], [ null, %if.then102 ], [ %p, %if.then16 ], [ %p, %if.then14 ], [ %p, %if.else301 ], [ %p, %if.then290 ], [ null, %if.end34.i ], [ null, %if.then12 ], [ %p, %land.lhs.true.i ]
  ret %struct.malloc_chunk* %newp.0
}

; Function Attrs: nounwind
define weak i8* @realloc_in_place(i8* %oldmem, i32 %bytes) #0 {
entry:
  %cmp = icmp eq i8* %oldmem, null
  br i1 %cmp, label %if.end9, label %if.then

if.then:                                          ; preds = %entry
  %cmp1 = icmp ugt i32 %bytes, -65
  br i1 %cmp1, label %if.then2, label %if.else

if.then2:                                         ; preds = %if.then
  %call = tail call i32* @__errno_location() #7
  store i32 12, i32* %call, align 4
  br label %if.end9

if.else:                                          ; preds = %if.then
  %cmp3 = icmp ult i32 %bytes, 11
  br i1 %cmp3, label %cond.end, label %cond.false

cond.false:                                       ; preds = %if.else
  %add4 = add i32 %bytes, 11
  %and = and i32 %add4, -8
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %if.else
  %cond = phi i32 [ %and, %cond.false ], [ 16, %if.else ]
  %add.ptr = getelementptr inbounds i8, i8* %oldmem, i32 -8
  %0 = bitcast i8* %add.ptr to %struct.malloc_chunk*
  %call5 = tail call fastcc %struct.malloc_chunk* @try_realloc_chunk(%struct.malloc_chunk* %0, i32 %cond)
  %cmp6 = icmp eq %struct.malloc_chunk* %call5, %0
  %oldmem. = select i1 %cmp6, i8* %oldmem, i8* null
  ret i8* %oldmem.

if.end9:                                          ; preds = %if.then2, %entry
  ret i8* null
}

; Function Attrs: nounwind
define weak i8* @memalign(i32 %alignment, i32 %bytes) #0 {
entry:
  %cmp = icmp ult i32 %alignment, 9
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %call = tail call i8* @malloc(i32 %bytes)
  br label %return

if.end:                                           ; preds = %entry
  %call1 = tail call fastcc i8* @internal_memalign(i32 %alignment, i32 %bytes)
  br label %return

return:                                           ; preds = %if.end, %if.then
  %retval.0 = phi i8* [ %call, %if.then ], [ %call1, %if.end ]
  ret i8* %retval.0
}

; Function Attrs: nounwind
define internal fastcc i8* @internal_memalign(i32 %alignment, i32 %bytes) #0 {
entry:
  %cmp = icmp ult i32 %alignment, 16
  %.alignment = select i1 %cmp, i32 16, i32 %alignment
  %sub = add i32 %.alignment, -1
  %and = and i32 %sub, %.alignment
  %cmp1 = icmp eq i32 %and, 0
  br i1 %cmp1, label %if.end4, label %while.cond

while.cond:                                       ; preds = %while.cond, %entry
  %a.0 = phi i32 [ %shl, %while.cond ], [ 16, %entry ]
  %cmp3 = icmp ult i32 %a.0, %.alignment
  %shl = shl i32 %a.0, 1
  br i1 %cmp3, label %while.cond, label %if.end4

if.end4:                                          ; preds = %while.cond, %entry
  %alignment.addr.1 = phi i32 [ %.alignment, %entry ], [ %a.0, %while.cond ]
  %sub5 = sub i32 -64, %alignment.addr.1
  %cmp6 = icmp ugt i32 %sub5, %bytes
  br i1 %cmp6, label %if.else, label %if.then9

if.then9:                                         ; preds = %if.end4
  %call = tail call i32* @__errno_location() #7
  store i32 12, i32* %call, align 4
  br label %if.end96

if.else:                                          ; preds = %if.end4
  %cmp11 = icmp ult i32 %bytes, 11
  br i1 %cmp11, label %cond.end, label %cond.false

cond.false:                                       ; preds = %if.else
  %add12 = add i32 %bytes, 11
  %and13 = and i32 %add12, -8
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %if.else
  %cond = phi i32 [ %and13, %cond.false ], [ 16, %if.else ]
  %add14 = add i32 %alignment.addr.1, 12
  %sub16 = add i32 %add14, %cond
  %call17 = tail call i8* @malloc(i32 %sub16)
  %cmp18 = icmp eq i8* %call17, null
  br i1 %cmp18, label %if.end96, label %if.then19

if.then19:                                        ; preds = %cond.end
  %add.ptr = getelementptr inbounds i8, i8* %call17, i32 -8
  %0 = bitcast i8* %add.ptr to %struct.malloc_chunk*
  %1 = ptrtoint i8* %call17 to i32
  %sub20 = add i32 %alignment.addr.1, -1
  %and21 = and i32 %1, %sub20
  %cmp22 = icmp eq i32 %and21, 0
  br i1 %cmp22, label %if.end64, label %if.then23

if.then23:                                        ; preds = %if.then19
  %add.ptr25 = getelementptr inbounds i8, i8* %call17, i32 %sub20
  %2 = ptrtoint i8* %add.ptr25 to i32
  %sub26 = sub i32 0, %alignment.addr.1
  %and27 = and i32 %2, %sub26
  %3 = inttoptr i32 %and27 to i8*
  %add.ptr28 = getelementptr inbounds i8, i8* %3, i32 -8
  %sub.ptr.lhs.cast = ptrtoint i8* %add.ptr28 to i32
  %sub.ptr.rhs.cast = ptrtoint i8* %add.ptr to i32
  %sub.ptr.sub = sub i32 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast
  %cmp29 = icmp ugt i32 %sub.ptr.sub, 15
  br i1 %cmp29, label %cond.end33, label %cond.false31

cond.false31:                                     ; preds = %if.then23
  %add.ptr28.sum = add i32 %alignment.addr.1, -8
  %add.ptr32 = getelementptr inbounds i8, i8* %3, i32 %add.ptr28.sum
  br label %cond.end33

cond.end33:                                       ; preds = %cond.false31, %if.then23
  %cond34 = phi i8* [ %add.ptr32, %cond.false31 ], [ %add.ptr28, %if.then23 ]
  %4 = bitcast i8* %cond34 to %struct.malloc_chunk*
  %sub.ptr.lhs.cast35 = ptrtoint i8* %cond34 to i32
  %sub.ptr.sub37 = sub i32 %sub.ptr.lhs.cast35, %sub.ptr.rhs.cast
  %head = getelementptr inbounds i8, i8* %call17, i32 -4
  %5 = bitcast i8* %head to i32*
  %6 = load i32, i32* %5, align 4
  %and38 = and i32 %6, -8
  %sub39 = sub i32 %and38, %sub.ptr.sub37
  %and41 = and i32 %6, 3
  %cmp42 = icmp eq i32 %and41, 0
  br i1 %cmp42, label %if.then43, label %if.else47

if.then43:                                        ; preds = %cond.end33
  %prev_foot = bitcast i8* %add.ptr to i32*
  %7 = load i32, i32* %prev_foot, align 4
  %add44 = add i32 %7, %sub.ptr.sub37
  %prev_foot45 = bitcast i8* %cond34 to i32*
  store i32 %add44, i32* %prev_foot45, align 4
  %head46 = getelementptr inbounds i8, i8* %cond34, i32 4
  %8 = bitcast i8* %head46 to i32*
  store i32 %sub39, i32* %8, align 4
  br label %if.end64

if.else47:                                        ; preds = %cond.end33
  %head48 = getelementptr inbounds i8, i8* %cond34, i32 4
  %9 = bitcast i8* %head48 to i32*
  %10 = load i32, i32* %9, align 4
  %and49 = and i32 %10, 1
  %or = or i32 %sub39, %and49
  %or50 = or i32 %or, 2
  store i32 %or50, i32* %9, align 4
  %add.ptr52.sum = add i32 %sub39, 4
  %head53 = getelementptr inbounds i8, i8* %cond34, i32 %add.ptr52.sum
  %11 = bitcast i8* %head53 to i32*
  %12 = load i32, i32* %11, align 4
  %or54 = or i32 %12, 1
  store i32 %or54, i32* %11, align 4
  %13 = load i32, i32* %5, align 4
  %and56 = and i32 %13, 1
  %or57 = or i32 %sub.ptr.sub37, %and56
  %or58 = or i32 %or57, 2
  store i32 %or58, i32* %5, align 4
  %add.ptr60.sum = add i32 %sub.ptr.sub37, -4
  %head61 = getelementptr inbounds i8, i8* %call17, i32 %add.ptr60.sum
  %14 = bitcast i8* %head61 to i32*
  %15 = load i32, i32* %14, align 4
  %or62 = or i32 %15, 1
  store i32 %or62, i32* %14, align 4
  tail call fastcc void @dispose_chunk(%struct.malloc_chunk* %0, i32 %sub.ptr.sub37)
  br label %if.end64

if.end64:                                         ; preds = %if.else47, %if.then43, %if.then19
  %p.0 = phi %struct.malloc_chunk* [ %0, %if.then19 ], [ %4, %if.else47 ], [ %4, %if.then43 ]
  %head65 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.0, i32 0, i32 1
  %16 = load i32, i32* %head65, align 4
  %and66 = and i32 %16, 3
  %cmp67 = icmp eq i32 %and66, 0
  br i1 %cmp67, label %if.end93, label %if.then68

if.then68:                                        ; preds = %if.end64
  %and70 = and i32 %16, -8
  %add71 = add i32 %cond, 16
  %cmp72 = icmp ugt i32 %and70, %add71
  br i1 %cmp72, label %if.then73, label %if.end93

if.then73:                                        ; preds = %if.then68
  %sub74 = sub i32 %and70, %cond
  %17 = bitcast %struct.malloc_chunk* %p.0 to i8*
  %add.ptr75 = getelementptr inbounds i8, i8* %17, i32 %cond
  %18 = bitcast i8* %add.ptr75 to %struct.malloc_chunk*
  %and77 = and i32 %16, 1
  %or78 = or i32 %cond, %and77
  %or79 = or i32 %or78, 2
  store i32 %or79, i32* %head65, align 4
  %add.ptr75.sum1 = or i32 %cond, 4
  %head82 = getelementptr inbounds i8, i8* %17, i32 %add.ptr75.sum1
  %19 = bitcast i8* %head82 to i32*
  %or87 = or i32 %sub74, 3
  store i32 %or87, i32* %19, align 4
  %add.ptr89.sum2 = or i32 %and70, 4
  %head90 = getelementptr inbounds i8, i8* %17, i32 %add.ptr89.sum2
  %20 = bitcast i8* %head90 to i32*
  %21 = load i32, i32* %20, align 4
  %or91 = or i32 %21, 1
  store i32 %or91, i32* %20, align 4
  tail call fastcc void @dispose_chunk(%struct.malloc_chunk* %18, i32 %sub74)
  br label %if.end93

if.end93:                                         ; preds = %if.then73, %if.then68, %if.end64
  %add.ptr94 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.0, i32 0, i32 2
  %22 = bitcast %struct.malloc_chunk** %add.ptr94 to i8*
  br label %if.end96

if.end96:                                         ; preds = %if.end93, %cond.end, %if.then9
  %mem.0 = phi i8* [ null, %if.then9 ], [ %22, %if.end93 ], [ null, %cond.end ]
  ret i8* %mem.0
}

; Function Attrs: nounwind
define weak i32 @posix_memalign(i8** %pp, i32 %alignment, i32 %bytes) #0 {
entry:
  %cmp = icmp eq i32 %alignment, 8
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %call = tail call i8* @malloc(i32 %bytes)
  br label %if.end15

if.else:                                          ; preds = %entry
  %div = lshr i32 %alignment, 2
  %rem = and i32 %alignment, 3
  %cmp1 = icmp ne i32 %rem, 0
  %cmp2 = icmp eq i32 %div, 0
  %or.cond = or i1 %cmp1, %cmp2
  br i1 %or.cond, label %return, label %lor.lhs.false3

lor.lhs.false3:                                   ; preds = %if.else
  %sub = add i32 %div, 1073741823
  %and = and i32 %sub, %div
  %cmp4 = icmp eq i32 %and, 0
  br i1 %cmp4, label %if.else6, label %return

if.else6:                                         ; preds = %lor.lhs.false3
  %sub7 = sub i32 -64, %alignment
  %cmp8 = icmp ult i32 %sub7, %bytes
  br i1 %cmp8, label %return, label %if.then9

if.then9:                                         ; preds = %if.else6
  %cmp10 = icmp ult i32 %alignment, 16
  %.alignment = select i1 %cmp10, i32 16, i32 %alignment
  %call12 = tail call fastcc i8* @internal_memalign(i32 %.alignment, i32 %bytes)
  br label %if.end15

if.end15:                                         ; preds = %if.then9, %if.then
  %mem.0 = phi i8* [ %call, %if.then ], [ %call12, %if.then9 ]
  %cmp16 = icmp eq i8* %mem.0, null
  br i1 %cmp16, label %return, label %if.else18

if.else18:                                        ; preds = %if.end15
  store i8* %mem.0, i8** %pp, align 4
  br label %return

return:                                           ; preds = %if.else18, %if.end15, %if.else6, %lor.lhs.false3, %if.else
  %retval.0 = phi i32 [ 0, %if.else18 ], [ 22, %if.else ], [ 22, %lor.lhs.false3 ], [ 12, %if.end15 ], [ 12, %if.else6 ]
  ret i32 %retval.0
}

; Function Attrs: nounwind
define weak i8* @valloc(i32 %bytes) #0 {
entry:
  %0 = load i32, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 0), align 4
  %cmp = icmp eq i32 %0, 0
  br i1 %cmp, label %if.then.i, label %lor.end

if.then.i:                                        ; preds = %entry
  %call.i = tail call i32 @sysconf(i32 30) #1
  %sub.i = add i32 %call.i, -1
  %and.i = and i32 %sub.i, %call.i
  %cmp1.i = icmp eq i32 %and.i, 0
  br i1 %cmp1.i, label %init_mparams.exit, label %if.then5.i

if.then5.i:                                       ; preds = %if.then.i
  tail call void @abort() #6
  unreachable

init_mparams.exit:                                ; preds = %if.then.i
  store i32 %call.i, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 2), align 4
  store i32 %call.i, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 1), align 4
  store i32 -1, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 3), align 4
  store i32 -1, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 4), align 4
  store i32 0, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 5), align 4
  store i32 0, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 15), align 4
  %call6.i = tail call i32 @time(i32* null) #1
  %xor.i = and i32 %call6.i, -16
  %and7.i = xor i32 %xor.i, 1431655768
  store volatile i32 %and7.i, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 0), align 4
  br label %lor.end

lor.end:                                          ; preds = %init_mparams.exit, %entry
  %1 = load i32, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 1), align 4
  %call1 = tail call i8* @memalign(i32 %1, i32 %bytes)
  ret i8* %call1
}

; Function Attrs: nounwind
define weak i8* @pvalloc(i32 %bytes) #0 {
entry:
  %0 = load i32, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 0), align 4
  %cmp = icmp eq i32 %0, 0
  br i1 %cmp, label %if.then.i, label %lor.end

if.then.i:                                        ; preds = %entry
  %call.i = tail call i32 @sysconf(i32 30) #1
  %sub.i = add i32 %call.i, -1
  %and.i = and i32 %sub.i, %call.i
  %cmp1.i = icmp eq i32 %and.i, 0
  br i1 %cmp1.i, label %init_mparams.exit, label %if.then5.i

if.then5.i:                                       ; preds = %if.then.i
  tail call void @abort() #6
  unreachable

init_mparams.exit:                                ; preds = %if.then.i
  store i32 %call.i, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 2), align 4
  store i32 %call.i, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 1), align 4
  store i32 -1, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 3), align 4
  store i32 -1, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 4), align 4
  store i32 0, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 5), align 4
  store i32 0, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 15), align 4
  %call6.i = tail call i32 @time(i32* null) #1
  %xor.i = and i32 %call6.i, -16
  %and7.i = xor i32 %xor.i, 1431655768
  store volatile i32 %and7.i, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 0), align 4
  br label %lor.end

lor.end:                                          ; preds = %init_mparams.exit, %entry
  %1 = load i32, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 1), align 4
  %add = add i32 %bytes, -1
  %sub = add i32 %add, %1
  %neg = sub i32 0, %1
  %and = and i32 %sub, %neg
  %call2 = tail call i8* @memalign(i32 %1, i32 %and)
  ret i8* %call2
}

; Function Attrs: nounwind
define weak i8** @independent_calloc(i32 %n_elements, i32 %elem_size, i8** %chunks) #0 {
entry:
  %sz = alloca i32, align 4
  store i32 %elem_size, i32* %sz, align 4
  %call = call fastcc i8** @ialloc(i32 %n_elements, i32* %sz, i32 3, i8** %chunks)
  ret i8** %call
}

; Function Attrs: nounwind
define internal fastcc i8** @ialloc(i32 %n_elements, i32* nocapture %sizes, i32 %opts, i8** %chunks) #0 {
entry:
  %0 = load i32, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 0), align 4
  %cmp = icmp eq i32 %0, 0
  br i1 %cmp, label %if.then.i, label %lor.end

if.then.i:                                        ; preds = %entry
  %call.i = tail call i32 @sysconf(i32 30) #1
  %sub.i = add i32 %call.i, -1
  %and.i = and i32 %sub.i, %call.i
  %cmp1.i = icmp eq i32 %and.i, 0
  br i1 %cmp1.i, label %init_mparams.exit, label %if.then5.i

if.then5.i:                                       ; preds = %if.then.i
  tail call void @abort() #6
  unreachable

init_mparams.exit:                                ; preds = %if.then.i
  store i32 %call.i, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 2), align 4
  store i32 %call.i, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 1), align 4
  store i32 -1, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 3), align 4
  store i32 -1, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 4), align 4
  store i32 0, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 5), align 4
  store i32 0, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 15), align 4
  %call6.i = tail call i32 @time(i32* null) #1
  %xor.i = and i32 %call6.i, -16
  %and7.i = xor i32 %xor.i, 1431655768
  store volatile i32 %and7.i, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 0), align 4
  br label %lor.end

lor.end:                                          ; preds = %init_mparams.exit, %entry
  %cmp1 = icmp eq i8** %chunks, null
  %cmp2 = icmp eq i32 %n_elements, 0
  br i1 %cmp1, label %if.else, label %if.then

if.then:                                          ; preds = %lor.end
  br i1 %cmp2, label %return, label %if.end11

if.else:                                          ; preds = %lor.end
  br i1 %cmp2, label %if.then5, label %if.end7

if.then5:                                         ; preds = %if.else
  %call6 = tail call i8* @malloc(i32 0)
  %1 = bitcast i8* %call6 to i8**
  br label %return

if.end7:                                          ; preds = %if.else
  %mul = shl i32 %n_elements, 2
  %cmp8 = icmp ult i32 %mul, 11
  br i1 %cmp8, label %if.end11, label %cond.false

cond.false:                                       ; preds = %if.end7
  %add10 = add i32 %mul, 11
  %and = and i32 %add10, -8
  br label %if.end11

if.end11:                                         ; preds = %cond.false, %if.end7, %if.then
  %array_size.0 = phi i32 [ 0, %if.then ], [ %and, %cond.false ], [ 16, %if.end7 ]
  %marray.0 = phi i8** [ %chunks, %if.then ], [ null, %cond.false ], [ null, %if.end7 ]
  %and12 = and i32 %opts, 1
  %tobool13 = icmp eq i32 %and12, 0
  br i1 %tobool13, label %for.cond.preheader, label %if.then14

for.cond.preheader:                               ; preds = %if.end11
  br i1 %cmp2, label %if.end36, label %for.body

if.then14:                                        ; preds = %if.end11
  %2 = load i32, i32* %sizes, align 4
  %cmp15 = icmp ult i32 %2, 11
  br i1 %cmp15, label %cond.end21, label %cond.false17

cond.false17:                                     ; preds = %if.then14
  %add19 = add i32 %2, 11
  %and20 = and i32 %add19, -8
  br label %cond.end21

cond.end21:                                       ; preds = %cond.false17, %if.then14
  %cond22 = phi i32 [ %and20, %cond.false17 ], [ 16, %if.then14 ]
  %mul23 = mul i32 %cond22, %n_elements
  br label %if.end36

for.body:                                         ; preds = %cond.end33, %for.cond.preheader
  %i.09 = phi i32 [ %inc, %cond.end33 ], [ 0, %for.cond.preheader ]
  %contents_size.08 = phi i32 [ %add35, %cond.end33 ], [ 0, %for.cond.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %sizes, i32 %i.09
  %3 = load i32, i32* %arrayidx, align 4
  %cmp26 = icmp ult i32 %3, 11
  br i1 %cmp26, label %cond.end33, label %cond.false28

cond.false28:                                     ; preds = %for.body
  %add31 = add i32 %3, 11
  %and32 = and i32 %add31, -8
  br label %cond.end33

cond.end33:                                       ; preds = %cond.false28, %for.body
  %cond34 = phi i32 [ %and32, %cond.false28 ], [ 16, %for.body ]
  %add35 = add i32 %cond34, %contents_size.08
  %inc = add i32 %i.09, 1
  %cmp25 = icmp eq i32 %inc, %n_elements
  br i1 %cmp25, label %if.end36, label %for.body

if.end36:                                         ; preds = %cond.end33, %cond.end21, %for.cond.preheader
  %contents_size.1 = phi i32 [ %mul23, %cond.end21 ], [ 0, %for.cond.preheader ], [ %add35, %cond.end33 ]
  %element_size.0 = phi i32 [ %cond22, %cond.end21 ], [ 0, %for.cond.preheader ], [ 0, %cond.end33 ]
  %add37 = add i32 %array_size.0, -4
  %sub = add i32 %add37, %contents_size.1
  %call39 = tail call i8* @malloc(i32 %sub)
  %cmp44 = icmp eq i8* %call39, null
  br i1 %cmp44, label %return, label %if.end46

if.end46:                                         ; preds = %if.end36
  %add.ptr = getelementptr inbounds i8, i8* %call39, i32 -8
  %head = getelementptr inbounds i8, i8* %call39, i32 -4
  %4 = bitcast i8* %head to i32*
  %5 = load i32, i32* %4, align 4
  %and47 = and i32 %5, -8
  %and48 = and i32 %opts, 2
  %tobool49 = icmp eq i32 %and48, 0
  br i1 %tobool49, label %if.end53, label %if.then50

if.then50:                                        ; preds = %if.end46
  %sub51 = sub i32 -4, %array_size.0
  %sub52 = add i32 %sub51, %and47
  tail call void @llvm.memset.p0i8.i32(i8* %call39, i8 0, i32 %sub52, i32 4, i1 false)
  br label %if.end53

if.end53:                                         ; preds = %if.then50, %if.end46
  %cmp54 = icmp eq i8** %marray.0, null
  br i1 %cmp54, label %if.then55, label %if.end61

if.then55:                                        ; preds = %if.end53
  %sub57 = sub i32 %and47, %contents_size.1
  %add.ptr58 = getelementptr inbounds i8, i8* %call39, i32 %contents_size.1
  %6 = bitcast i8* %add.ptr58 to i8**
  %or59 = or i32 %sub57, 3
  %add.ptr56.sum = add i32 %contents_size.1, -4
  %head60 = getelementptr inbounds i8, i8* %call39, i32 %add.ptr56.sum
  %7 = bitcast i8* %head60 to i32*
  store i32 %or59, i32* %7, align 4
  br label %if.end61

if.end61:                                         ; preds = %if.then55, %if.end53
  %remainder_size.0 = phi i32 [ %contents_size.1, %if.then55 ], [ %and47, %if.end53 ]
  %marray.1 = phi i8** [ %6, %if.then55 ], [ %marray.0, %if.end53 ]
  store i8* %call39, i8** %marray.1, align 4
  %sub65 = add i32 %n_elements, -1
  %cmp662 = icmp eq i32 %sub65, 0
  br i1 %cmp662, label %if.else87, label %if.then67.lr.ph

if.then67.lr.ph:                                  ; preds = %if.end61
  %cmp68 = icmp eq i32 %element_size.0, 0
  br i1 %cmp68, label %if.else70.us, label %if.end81

if.else70.us:                                     ; preds = %if.end81.us, %if.then67.lr.ph
  %i.15.us = phi i32 [ %inc93.us, %if.end81.us ], [ 0, %if.then67.lr.ph ]
  %remainder_size.14.us = phi i32 [ %sub82.us, %if.end81.us ], [ %remainder_size.0, %if.then67.lr.ph ]
  %p.0.in3.us = phi i8* [ %add.ptr86.us, %if.end81.us ], [ %add.ptr, %if.then67.lr.ph ]
  %arrayidx71.us = getelementptr inbounds i32, i32* %sizes, i32 %i.15.us
  %8 = load i32, i32* %arrayidx71.us, align 4
  %cmp72.us = icmp ult i32 %8, 11
  br i1 %cmp72.us, label %if.end81.us, label %cond.false74.us

cond.false74.us:                                  ; preds = %if.else70.us
  %add77.us = add i32 %8, 11
  %and78.us = and i32 %add77.us, -8
  br label %if.end81.us

if.end81.us:                                      ; preds = %cond.false74.us, %if.else70.us
  %size.0.us = phi i32 [ %and78.us, %cond.false74.us ], [ 16, %if.else70.us ]
  %sub82.us = sub i32 %remainder_size.14.us, %size.0.us
  %or84.us = or i32 %size.0.us, 3
  %head85.us = getelementptr inbounds i8, i8* %p.0.in3.us, i32 4
  %9 = bitcast i8* %head85.us to i32*
  store i32 %or84.us, i32* %9, align 4
  %add.ptr86.us = getelementptr inbounds i8, i8* %p.0.in3.us, i32 %size.0.us
  %inc93.us = add i32 %i.15.us, 1
  %add.ptr86.us.sum = add i32 %size.0.us, 8
  %add.ptr63.us = getelementptr inbounds i8, i8* %p.0.in3.us, i32 %add.ptr86.us.sum
  %arrayidx64.us = getelementptr inbounds i8*, i8** %marray.1, i32 %inc93.us
  store i8* %add.ptr63.us, i8** %arrayidx64.us, align 4
  %cmp66.us = icmp eq i32 %inc93.us, %sub65
  br i1 %cmp66.us, label %if.else87, label %if.else70.us

if.end81:                                         ; preds = %if.end81, %if.then67.lr.ph
  %i.15 = phi i32 [ %inc93, %if.end81 ], [ 0, %if.then67.lr.ph ]
  %remainder_size.14 = phi i32 [ %sub82, %if.end81 ], [ %remainder_size.0, %if.then67.lr.ph ]
  %p.0.in3 = phi i8* [ %add.ptr86, %if.end81 ], [ %add.ptr, %if.then67.lr.ph ]
  %sub82 = sub i32 %remainder_size.14, %element_size.0
  %or84 = or i32 %element_size.0, 3
  %head85 = getelementptr inbounds i8, i8* %p.0.in3, i32 4
  %10 = bitcast i8* %head85 to i32*
  store i32 %or84, i32* %10, align 4
  %add.ptr86 = getelementptr inbounds i8, i8* %p.0.in3, i32 %element_size.0
  %inc93 = add i32 %i.15, 1
  %add.ptr86.sum = add i32 %element_size.0, 8
  %add.ptr63 = getelementptr inbounds i8, i8* %p.0.in3, i32 %add.ptr86.sum
  %arrayidx64 = getelementptr inbounds i8*, i8** %marray.1, i32 %inc93
  store i8* %add.ptr63, i8** %arrayidx64, align 4
  %cmp66 = icmp eq i32 %inc93, %sub65
  br i1 %cmp66, label %if.else87, label %if.end81

if.else87:                                        ; preds = %if.end81, %if.end81.us, %if.end61
  %remainder_size.1.lcssa = phi i32 [ %remainder_size.0, %if.end61 ], [ %sub82.us, %if.end81.us ], [ %sub82, %if.end81 ]
  %p.0.in.lcssa = phi i8* [ %add.ptr, %if.end61 ], [ %add.ptr86.us, %if.end81.us ], [ %add.ptr86, %if.end81 ]
  %or89 = or i32 %remainder_size.1.lcssa, 3
  %head90 = getelementptr inbounds i8, i8* %p.0.in.lcssa, i32 4
  %11 = bitcast i8* %head90 to i32*
  store i32 %or89, i32* %11, align 4
  br label %return

return:                                           ; preds = %if.else87, %if.end36, %if.then5, %if.then
  %retval.0 = phi i8** [ %marray.1, %if.else87 ], [ %1, %if.then5 ], [ %chunks, %if.then ], [ null, %if.end36 ]
  ret i8** %retval.0
}

; Function Attrs: nounwind
define weak i8** @independent_comalloc(i32 %n_elements, i32* %sizes, i8** %chunks) #0 {
entry:
  %call = tail call fastcc i8** @ialloc(i32 %n_elements, i32* %sizes, i32 0, i8** %chunks)
  ret i8** %call
}

; Function Attrs: nounwind
define weak i32 @bulk_free(i8** %array, i32 %nelem) #0 {
entry:
  %arrayidx.i = getelementptr inbounds i8*, i8** %array, i32 %nelem
  %cmp6.i = icmp eq i32 %nelem, 0
  br i1 %cmp6.i, label %internal_bulk_free.exit, label %for.body.i

for.body.i:                                       ; preds = %for.inc.i, %entry
  %a.07.i = phi i8** [ %array, %entry ], [ %incdec.ptr.pre-phi.i, %for.inc.i ]
  %0 = load i8*, i8** %a.07.i, align 4
  %cmp1.i = icmp eq i8* %0, null
  br i1 %cmp1.i, label %for.body.for.inc_crit_edge.i, label %if.then.i

for.body.for.inc_crit_edge.i:                     ; preds = %for.body.i
  %incdec.ptr.pre.i = getelementptr inbounds i8*, i8** %a.07.i, i32 1
  br label %for.inc.i

if.then.i:                                        ; preds = %for.body.i
  %add.ptr.i = getelementptr inbounds i8, i8* %0, i32 -8
  %1 = bitcast i8* %add.ptr.i to %struct.malloc_chunk*
  %head.i = getelementptr inbounds i8, i8* %0, i32 -4
  %2 = bitcast i8* %head.i to i32*
  %3 = load i32, i32* %2, align 4
  %and.i = and i32 %3, -8
  store i8* null, i8** %a.07.i, align 4
  %4 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp2.i = icmp ult i8* %add.ptr.i, %4
  br i1 %cmp2.i, label %if.else25.i, label %land.rhs.i

land.rhs.i:                                       ; preds = %if.then.i
  %and4.i = and i32 %3, 3
  %cmp5.i = icmp eq i32 %and4.i, 1
  br i1 %cmp5.i, label %if.else25.i, label %if.then6.i

if.then6.i:                                       ; preds = %land.rhs.i
  %add.ptr7.i = getelementptr inbounds i8*, i8** %a.07.i, i32 1
  %and91.i = add i32 %3, -8
  %add.ptr.sum.i = and i32 %and91.i, -8
  %cmp11.i = icmp eq i8** %add.ptr7.i, %arrayidx.i
  br i1 %cmp11.i, label %if.else.i, label %land.lhs.true.i

land.lhs.true.i:                                  ; preds = %if.then6.i
  %5 = load i8*, i8** %add.ptr7.i, align 4
  %add.ptr10.sum.i = add i32 %add.ptr.sum.i, 8
  %add.ptr12.i = getelementptr inbounds i8, i8* %0, i32 %add.ptr10.sum.i
  %cmp13.i = icmp eq i8* %5, %add.ptr12.i
  br i1 %cmp13.i, label %if.then14.i, label %if.else.i

if.then14.i:                                      ; preds = %land.lhs.true.i
  %add.ptr10.sum23.i = or i32 %add.ptr.sum.i, 4
  %head15.i = getelementptr inbounds i8, i8* %0, i32 %add.ptr10.sum23.i
  %6 = bitcast i8* %head15.i to i32*
  %7 = load i32, i32* %6, align 4
  %and16.i = and i32 %7, -8
  %add.i = add i32 %and16.i, %and.i
  %and18.i = and i32 %3, 1
  %or.i = or i32 %and18.i, %add.i
  %or19.i = or i32 %or.i, 2
  store i32 %or19.i, i32* %2, align 4
  %add.ptr21.sum.i = add i32 %add.i, -4
  %head22.i = getelementptr inbounds i8, i8* %0, i32 %add.ptr21.sum.i
  %8 = bitcast i8* %head22.i to i32*
  %9 = load i32, i32* %8, align 4
  %or23.i = or i32 %9, 1
  store i32 %or23.i, i32* %8, align 4
  store i8* %0, i8** %add.ptr7.i, align 4
  br label %for.inc.i

if.else.i:                                        ; preds = %land.lhs.true.i, %if.then6.i
  tail call fastcc void @dispose_chunk(%struct.malloc_chunk* %1, i32 %and.i) #1
  br label %for.inc.i

if.else25.i:                                      ; preds = %land.rhs.i, %if.then.i
  tail call void @abort() #6
  unreachable

for.inc.i:                                        ; preds = %if.else.i, %if.then14.i, %for.body.for.inc_crit_edge.i
  %incdec.ptr.pre-phi.i = phi i8** [ %incdec.ptr.pre.i, %for.body.for.inc_crit_edge.i ], [ %add.ptr7.i, %if.then14.i ], [ %add.ptr7.i, %if.else.i ]
  %cmp.i = icmp eq i8** %incdec.ptr.pre-phi.i, %arrayidx.i
  br i1 %cmp.i, label %internal_bulk_free.exit, label %for.body.i

internal_bulk_free.exit:                          ; preds = %for.inc.i, %entry
  ret i32 0
}

; Function Attrs: nounwind
define weak i32 @malloc_trim(i32 %pad) #0 {
entry:
  %0 = load i32, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 0), align 4
  %cmp = icmp eq i32 %0, 0
  br i1 %cmp, label %if.then.i, label %lor.end.i

if.then.i:                                        ; preds = %entry
  %call.i = tail call i32 @sysconf(i32 30) #1
  %sub.i = add i32 %call.i, -1
  %and.i = and i32 %sub.i, %call.i
  %cmp1.i = icmp eq i32 %and.i, 0
  br i1 %cmp1.i, label %init_mparams.exit, label %if.then5.i

if.then5.i:                                       ; preds = %if.then.i
  tail call void @abort() #6
  unreachable

init_mparams.exit:                                ; preds = %if.then.i
  store i32 %call.i, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 2), align 4
  store i32 %call.i, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 1), align 4
  store i32 -1, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 3), align 4
  store i32 -1, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 4), align 4
  store i32 0, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 5), align 4
  store i32 0, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 15), align 4
  %call6.i = tail call i32 @time(i32* null) #1
  %xor.i = and i32 %call6.i, -16
  %and7.i = xor i32 %xor.i, 1431655768
  store volatile i32 %and7.i, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 0), align 4
  br label %lor.end.i

lor.end.i:                                        ; preds = %init_mparams.exit, %entry
  %cmp1.i2 = icmp ult i32 %pad, -64
  br i1 %cmp1.i2, label %land.lhs.true.i, label %sys_trim.exit

land.lhs.true.i:                                  ; preds = %lor.end.i
  %1 = load %struct.malloc_chunk*, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 6), align 4
  %cmp2.i = icmp eq %struct.malloc_chunk* %1, null
  br i1 %cmp2.i, label %sys_trim.exit, label %if.then.i3

if.then.i3:                                       ; preds = %land.lhs.true.i
  %add.i = add i32 %pad, 40
  %2 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 3), align 4
  %cmp3.i = icmp ugt i32 %2, %add.i
  br i1 %cmp3.i, label %if.then4.i, label %land.lhs.true45.i

if.then4.i:                                       ; preds = %if.then.i3
  %3 = load i32, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 2), align 4
  %sub6.i = sub i32 -41, %pad
  %sub.i4 = add i32 %sub6.i, %2
  %add7.i = add i32 %sub.i4, %3
  %div.i = udiv i32 %add7.i, %3
  %4 = bitcast %struct.malloc_chunk* %1 to i8*
  br label %for.cond.i.i

for.cond.i.i:                                     ; preds = %if.end.i2.i, %if.then4.i
  %sp.0.i.i = phi %struct.malloc_segment* [ getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 16), %if.then4.i ], [ %7, %if.end.i2.i ]
  %base.i.i = getelementptr inbounds %struct.malloc_segment, %struct.malloc_segment* %sp.0.i.i, i32 0, i32 0
  %5 = load i8*, i8** %base.i.i, align 4
  %cmp.i1.i = icmp ugt i8* %5, %4
  br i1 %cmp.i1.i, label %if.end.i2.i, label %land.lhs.true.i.i

land.lhs.true.i.i:                                ; preds = %for.cond.i.i
  %size.i.i = getelementptr inbounds %struct.malloc_segment, %struct.malloc_segment* %sp.0.i.i, i32 0, i32 1
  %6 = load i32, i32* %size.i.i, align 4
  %add.ptr.i.i = getelementptr inbounds i8, i8* %5, i32 %6
  %cmp2.i.i = icmp ugt i8* %add.ptr.i.i, %4
  br i1 %cmp2.i.i, label %segment_holding.exit.i, label %if.end.i2.i

if.end.i2.i:                                      ; preds = %land.lhs.true.i.i, %for.cond.i.i
  %next.i.i = getelementptr inbounds %struct.malloc_segment, %struct.malloc_segment* %sp.0.i.i, i32 0, i32 2
  %7 = load %struct.malloc_segment*, %struct.malloc_segment** %next.i.i, align 4
  br label %for.cond.i.i

segment_holding.exit.i:                           ; preds = %land.lhs.true.i.i
  %sub8.i = add i32 %div.i, -1
  %mul.i = mul i32 %sub8.i, %3
  %sflags.i = getelementptr inbounds %struct.malloc_segment, %struct.malloc_segment* %sp.0.i.i, i32 0, i32 3
  %8 = load i32, i32* %sflags.i, align 4
  %and.i5 = and i32 %8, 8
  %tobool11.i = icmp eq i32 %and.i5, 0
  br i1 %tobool11.i, label %if.else.i, label %land.lhs.true45.i

if.else.i:                                        ; preds = %segment_holding.exit.i
  %call20.i = tail call i8* @sbrk(i32 0) #1
  %9 = load i8*, i8** %base.i.i, align 4
  %10 = load i32, i32* %size.i.i, align 4
  %add.ptr.i = getelementptr inbounds i8, i8* %9, i32 %10
  %cmp21.i = icmp eq i8* %call20.i, %add.ptr.i
  br i1 %cmp21.i, label %if.then22.i, label %land.lhs.true45.i

if.then22.i:                                      ; preds = %if.else.i
  %sub19.i = sub i32 -2147483648, %3
  %cmp17.i = icmp ugt i32 %mul.i, 2147483646
  %sub19.mul.i = select i1 %cmp17.i, i32 %sub19.i, i32 %mul.i
  %sub23.i = sub i32 0, %sub19.mul.i
  %call24.i = tail call i8* @sbrk(i32 %sub23.i) #1
  %call25.i = tail call i8* @sbrk(i32 0) #1
  %cmp26.i = icmp ne i8* %call24.i, inttoptr (i32 -1 to i8*)
  %cmp28.i = icmp ult i8* %call25.i, %call20.i
  %or.cond.i = and i1 %cmp26.i, %cmp28.i
  br i1 %or.cond.i, label %if.end33.i, label %land.lhs.true45.i

if.end33.i:                                       ; preds = %if.then22.i
  %sub.ptr.lhs.cast.i = ptrtoint i8* %call20.i to i32
  %sub.ptr.rhs.cast.i = ptrtoint i8* %call25.i to i32
  %sub.ptr.sub.i = sub i32 %sub.ptr.lhs.cast.i, %sub.ptr.rhs.cast.i
  %cmp34.i = icmp eq i8* %call20.i, %call25.i
  br i1 %cmp34.i, label %land.lhs.true45.i, label %if.then35.i

if.then35.i:                                      ; preds = %if.end33.i
  %11 = load i32, i32* %size.i.i, align 4
  %sub37.i = sub i32 %11, %sub.ptr.sub.i
  store i32 %sub37.i, i32* %size.i.i, align 4
  %12 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 12), align 4
  %sub38.i = sub i32 %12, %sub.ptr.sub.i
  store i32 %sub38.i, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 12), align 4
  %13 = load %struct.malloc_chunk*, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 6), align 4
  %14 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 3), align 4
  %sub41.i = sub i32 %14, %sub.ptr.sub.i
  %15 = bitcast %struct.malloc_chunk* %13 to i8*
  %add.ptr.i3.i = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %13, i32 0, i32 2
  %16 = ptrtoint %struct.malloc_chunk** %add.ptr.i3.i to i32
  %and.i4.i = and i32 %16, 7
  %cmp.i5.i = icmp eq i32 %and.i4.i, 0
  br i1 %cmp.i5.i, label %if.end43.i, label %cond.false.i.i

cond.false.i.i:                                   ; preds = %if.then35.i
  %17 = sub i32 0, %16
  %and3.i.i = and i32 %17, 7
  br label %if.end43.i

if.end43.i:                                       ; preds = %cond.false.i.i, %if.then35.i
  %cond.i.i = phi i32 [ %and3.i.i, %cond.false.i.i ], [ 0, %if.then35.i ]
  %add.ptr4.i.i = getelementptr inbounds i8, i8* %15, i32 %cond.i.i
  %18 = bitcast i8* %add.ptr4.i.i to %struct.malloc_chunk*
  %sub5.i.i = sub i32 %sub41.i, %cond.i.i
  store %struct.malloc_chunk* %18, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 6), align 4
  store i32 %sub5.i.i, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 3), align 4
  %or.i.i = or i32 %sub5.i.i, 1
  %add.ptr4.sum.i.i = add i32 %cond.i.i, 4
  %head.i.i = getelementptr inbounds i8, i8* %15, i32 %add.ptr4.sum.i.i
  %19 = bitcast i8* %head.i.i to i32*
  store i32 %or.i.i, i32* %19, align 4
  %add.ptr6.sum.i.i = add i32 %sub41.i, 4
  %head7.i.i = getelementptr inbounds i8, i8* %15, i32 %add.ptr6.sum.i.i
  %20 = bitcast i8* %head7.i.i to i32*
  store i32 40, i32* %20, align 4
  %21 = load i32, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 4), align 4
  store i32 %21, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 7), align 4
  br label %sys_trim.exit

land.lhs.true45.i:                                ; preds = %if.end33.i, %if.then22.i, %if.else.i, %segment_holding.exit.i, %if.then.i3
  %22 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 3), align 4
  %23 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 7), align 4
  %cmp47.i = icmp ugt i32 %22, %23
  br i1 %cmp47.i, label %if.then48.i, label %sys_trim.exit

if.then48.i:                                      ; preds = %land.lhs.true45.i
  store i32 -1, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 7), align 4
  br label %sys_trim.exit

sys_trim.exit:                                    ; preds = %if.then48.i, %land.lhs.true45.i, %if.end43.i, %land.lhs.true.i, %lor.end.i
  %released.2.i = phi i32 [ 0, %if.then48.i ], [ 0, %land.lhs.true45.i ], [ 1, %if.end43.i ], [ 0, %land.lhs.true.i ], [ 0, %lor.end.i ]
  ret i32 %released.2.i
}

; Function Attrs: nounwind
define weak i32 @malloc_footprint() #0 {
entry:
  %0 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 12), align 4
  ret i32 %0
}

; Function Attrs: nounwind
define weak i32 @malloc_max_footprint() #0 {
entry:
  %0 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 13), align 4
  ret i32 %0
}

; Function Attrs: nounwind
define weak i32 @malloc_footprint_limit() #0 {
entry:
  %0 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 14), align 4
  %cmp = icmp eq i32 %0, 0
  %cond = select i1 %cmp, i32 -1, i32 %0
  ret i32 %cond
}

; Function Attrs: nounwind
define weak i32 @malloc_set_footprint_limit(i32 %bytes) #0 {
entry:
  %cmp2 = icmp eq i32 %bytes, -1
  br i1 %cmp2, label %if.end9, label %if.else

if.else:                                          ; preds = %entry
  %0 = load i32, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 2), align 4
  %sub4 = add i32 %bytes, -1
  %add5 = add i32 %sub4, %0
  %neg7 = sub i32 0, %0
  %and8 = and i32 %add5, %neg7
  br label %if.end9

if.end9:                                          ; preds = %if.else, %entry
  %result.0 = phi i32 [ %and8, %if.else ], [ 0, %entry ]
  store i32 %result.0, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 14), align 4
  ret i32 %result.0
}

; Function Attrs: nounwind
define weak void @mallinfo(%struct.mallinfo* noalias sret %agg.result) #0 {
entry:
  %0 = load i32, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 0), align 4
  %cmp.i = icmp eq i32 %0, 0
  br i1 %cmp.i, label %if.then.i.i, label %lor.end.i

if.then.i.i:                                      ; preds = %entry
  %call.i.i = tail call i32 @sysconf(i32 30) #1
  %sub.i.i = add i32 %call.i.i, -1
  %and.i.i = and i32 %sub.i.i, %call.i.i
  %cmp1.i.i = icmp eq i32 %and.i.i, 0
  br i1 %cmp1.i.i, label %init_mparams.exit.i, label %if.then5.i.i

if.then5.i.i:                                     ; preds = %if.then.i.i
  tail call void @abort() #6
  unreachable

init_mparams.exit.i:                              ; preds = %if.then.i.i
  store i32 %call.i.i, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 2), align 4
  store i32 %call.i.i, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 1), align 4
  store i32 -1, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 3), align 4
  store i32 -1, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 4), align 4
  store i32 0, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 5), align 4
  store i32 0, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 15), align 4
  %call6.i.i = tail call i32 @time(i32* null) #1
  %xor.i.i = and i32 %call6.i.i, -16
  %and7.i.i = xor i32 %xor.i.i, 1431655768
  store volatile i32 %and7.i.i, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 0), align 4
  br label %lor.end.i

lor.end.i:                                        ; preds = %init_mparams.exit.i, %entry
  %1 = load %struct.malloc_chunk*, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 6), align 4
  %cmp1.i = icmp eq %struct.malloc_chunk* %1, null
  br i1 %cmp1.i, label %internal_mallinfo.exit, label %if.then.i

if.then.i:                                        ; preds = %lor.end.i
  %2 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 3), align 4
  %add.i = add i32 %2, 40
  br label %while.body.i

while.body.i:                                     ; preds = %while.end.i, %if.then.i
  %s.013.i = phi %struct.malloc_segment* [ getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 16), %if.then.i ], [ %9, %while.end.i ]
  %sum.012.i = phi i32 [ %add.i, %if.then.i ], [ %sum.1.lcssa.i, %while.end.i ]
  %mfree.011.i = phi i32 [ %add.i, %if.then.i ], [ %mfree.1.lcssa.i, %while.end.i ]
  %nfree.010.i = phi i32 [ 1, %if.then.i ], [ %nfree.1.lcssa.i, %while.end.i ]
  %base.i = getelementptr inbounds %struct.malloc_segment, %struct.malloc_segment* %s.013.i, i32 0, i32 0
  %3 = load i8*, i8** %base.i, align 4
  %add.ptr.i = getelementptr inbounds i8, i8* %3, i32 8
  %4 = ptrtoint i8* %add.ptr.i to i32
  %and.i = and i32 %4, 7
  %cmp4.i = icmp eq i32 %and.i, 0
  br i1 %cmp4.i, label %land.lhs.true.lr.ph.i, label %cond.false.i

cond.false.i:                                     ; preds = %while.body.i
  %5 = sub i32 0, %4
  %and8.i = and i32 %5, 7
  br label %land.lhs.true.lr.ph.i

land.lhs.true.lr.ph.i:                            ; preds = %cond.false.i, %while.body.i
  %cond.i = phi i32 [ %and8.i, %cond.false.i ], [ 0, %while.body.i ]
  %add.ptr9.i = getelementptr inbounds i8, i8* %3, i32 %cond.i
  %size.i = getelementptr inbounds %struct.malloc_segment, %struct.malloc_segment* %s.013.i, i32 0, i32 1
  %6 = load i32, i32* %size.i, align 4
  %add.ptr14.i = getelementptr inbounds i8, i8* %3, i32 %6
  br label %land.lhs.true.i

land.lhs.true.i:                                  ; preds = %if.end.i, %land.lhs.true.lr.ph.i
  %q.0.in7.i = phi i8* [ %add.ptr9.i, %land.lhs.true.lr.ph.i ], [ %add.ptr31.i, %if.end.i ]
  %sum.16.i = phi i32 [ %sum.012.i, %land.lhs.true.lr.ph.i ], [ %add23.i, %if.end.i ]
  %mfree.15.i = phi i32 [ %mfree.011.i, %land.lhs.true.lr.ph.i ], [ %mfree.2.i, %if.end.i ]
  %nfree.14.i = phi i32 [ %nfree.010.i, %land.lhs.true.lr.ph.i ], [ %nfree.2.i, %if.end.i ]
  %q.0.i = bitcast i8* %q.0.in7.i to %struct.malloc_chunk*
  %cmp15.i = icmp uge i8* %q.0.in7.i, %add.ptr14.i
  %cmp18.i = icmp eq %struct.malloc_chunk* %q.0.i, %1
  %or.cond.i = or i1 %cmp15.i, %cmp18.i
  br i1 %or.cond.i, label %while.end.i, label %land.rhs.i

land.rhs.i:                                       ; preds = %land.lhs.true.i
  %head.i = getelementptr inbounds i8, i8* %q.0.in7.i, i32 4
  %7 = bitcast i8* %head.i to i32*
  %8 = load i32, i32* %7, align 4
  %cmp19.i = icmp eq i32 %8, 7
  br i1 %cmp19.i, label %while.end.i, label %while.body20.i

while.body20.i:                                   ; preds = %land.rhs.i
  %and22.i = and i32 %8, -8
  %add23.i = add i32 %and22.i, %sum.16.i
  %and25.i = and i32 %8, 3
  %cmp26.i = icmp eq i32 %and25.i, 1
  br i1 %cmp26.i, label %if.then27.i, label %if.end.i

if.then27.i:                                      ; preds = %while.body20.i
  %add28.i = add i32 %and22.i, %mfree.15.i
  %inc.i = add i32 %nfree.14.i, 1
  br label %if.end.i

if.end.i:                                         ; preds = %if.then27.i, %while.body20.i
  %nfree.2.i = phi i32 [ %nfree.14.i, %while.body20.i ], [ %inc.i, %if.then27.i ]
  %mfree.2.i = phi i32 [ %mfree.15.i, %while.body20.i ], [ %add28.i, %if.then27.i ]
  %add.ptr31.i = getelementptr inbounds i8, i8* %q.0.in7.i, i32 %and22.i
  %cmp12.i = icmp ult i8* %add.ptr31.i, %3
  br i1 %cmp12.i, label %while.end.i, label %land.lhs.true.i

while.end.i:                                      ; preds = %if.end.i, %land.rhs.i, %land.lhs.true.i
  %sum.1.lcssa.i = phi i32 [ %sum.16.i, %land.rhs.i ], [ %sum.16.i, %land.lhs.true.i ], [ %add23.i, %if.end.i ]
  %mfree.1.lcssa.i = phi i32 [ %mfree.15.i, %land.rhs.i ], [ %mfree.15.i, %land.lhs.true.i ], [ %mfree.2.i, %if.end.i ]
  %nfree.1.lcssa.i = phi i32 [ %nfree.14.i, %land.rhs.i ], [ %nfree.14.i, %land.lhs.true.i ], [ %nfree.2.i, %if.end.i ]
  %next.i = getelementptr inbounds %struct.malloc_segment, %struct.malloc_segment* %s.013.i, i32 0, i32 2
  %9 = load %struct.malloc_segment*, %struct.malloc_segment** %next.i, align 4
  %cmp2.i = icmp eq %struct.malloc_segment* %9, null
  br i1 %cmp2.i, label %while.end32.i, label %while.body.i

while.end32.i:                                    ; preds = %while.end.i
  %10 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 12), align 4
  %sub33.i = sub i32 %10, %sum.1.lcssa.i
  %11 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 13), align 4
  %sub35.i = sub i32 %10, %mfree.1.lcssa.i
  br label %internal_mallinfo.exit

internal_mallinfo.exit:                           ; preds = %while.end32.i, %lor.end.i
  %nm.sroa.6.0.i = phi i32 [ %sub35.i, %while.end32.i ], [ 0, %lor.end.i ]
  %nm.sroa.4.0.i = phi i32 [ %11, %while.end32.i ], [ 0, %lor.end.i ]
  %nm.sroa.3.0.i = phi i32 [ %sub33.i, %while.end32.i ], [ 0, %lor.end.i ]
  %nm.sroa.1.0.i = phi i32 [ %nfree.1.lcssa.i, %while.end32.i ], [ 0, %lor.end.i ]
  %nm.sroa.0.0.i = phi i32 [ %sum.1.lcssa.i, %while.end32.i ], [ 0, %lor.end.i ]
  %nm.sroa.7.0.i = phi i32 [ %mfree.1.lcssa.i, %while.end32.i ], [ 0, %lor.end.i ]
  %nm.sroa.8.0.i = phi i32 [ %2, %while.end32.i ], [ 0, %lor.end.i ]
  %nm.sroa.0.0.idx.i = getelementptr inbounds %struct.mallinfo, %struct.mallinfo* %agg.result, i32 0, i32 0
  store i32 %nm.sroa.0.0.i, i32* %nm.sroa.0.0.idx.i, align 4
  %nm.sroa.1.4.idx25.i = getelementptr inbounds %struct.mallinfo, %struct.mallinfo* %agg.result, i32 0, i32 1
  store i32 %nm.sroa.1.0.i, i32* %nm.sroa.1.4.idx25.i, align 4
  %nm.sroa.2.8.idx.i = getelementptr inbounds %struct.mallinfo, %struct.mallinfo* %agg.result, i32 0, i32 2
  %12 = bitcast i32* %nm.sroa.2.8.idx.i to i64*
  store i64 0, i64* %12, align 4
  %nm.sroa.3.16.idx31.i = getelementptr inbounds %struct.mallinfo, %struct.mallinfo* %agg.result, i32 0, i32 4
  store i32 %nm.sroa.3.0.i, i32* %nm.sroa.3.16.idx31.i, align 4
  %nm.sroa.4.20.idx33.i = getelementptr inbounds %struct.mallinfo, %struct.mallinfo* %agg.result, i32 0, i32 5
  store i32 %nm.sroa.4.0.i, i32* %nm.sroa.4.20.idx33.i, align 4
  %nm.sroa.5.24.idx35.i = getelementptr inbounds %struct.mallinfo, %struct.mallinfo* %agg.result, i32 0, i32 6
  store i32 0, i32* %nm.sroa.5.24.idx35.i, align 4
  %nm.sroa.6.28.idx37.i = getelementptr inbounds %struct.mallinfo, %struct.mallinfo* %agg.result, i32 0, i32 7
  store i32 %nm.sroa.6.0.i, i32* %nm.sroa.6.28.idx37.i, align 4
  %nm.sroa.7.32.idx39.i = getelementptr inbounds %struct.mallinfo, %struct.mallinfo* %agg.result, i32 0, i32 8
  store i32 %nm.sroa.7.0.i, i32* %nm.sroa.7.32.idx39.i, align 4
  %nm.sroa.8.36.idx41.i = getelementptr inbounds %struct.mallinfo, %struct.mallinfo* %agg.result, i32 0, i32 9
  store i32 %nm.sroa.8.0.i, i32* %nm.sroa.8.36.idx41.i, align 4
  ret void
}

; Function Attrs: nounwind
define weak void @malloc_stats() #0 {
entry:
  %0 = load i32, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 0), align 4
  %cmp.i = icmp eq i32 %0, 0
  br i1 %cmp.i, label %if.then.i.i, label %lor.end.i

if.then.i.i:                                      ; preds = %entry
  %call.i.i = tail call i32 @sysconf(i32 30) #1
  %sub.i.i = add i32 %call.i.i, -1
  %and.i.i = and i32 %sub.i.i, %call.i.i
  %cmp1.i.i = icmp eq i32 %and.i.i, 0
  br i1 %cmp1.i.i, label %init_mparams.exit.i, label %if.then5.i.i

if.then5.i.i:                                     ; preds = %if.then.i.i
  tail call void @abort() #6
  unreachable

init_mparams.exit.i:                              ; preds = %if.then.i.i
  store i32 %call.i.i, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 2), align 4
  store i32 %call.i.i, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 1), align 4
  store i32 -1, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 3), align 4
  store i32 -1, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 4), align 4
  store i32 0, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 5), align 4
  store i32 0, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 15), align 4
  %call6.i.i = tail call i32 @time(i32* null) #1
  %xor.i.i = and i32 %call6.i.i, -16
  %and7.i.i = xor i32 %xor.i.i, 1431655768
  store volatile i32 %and7.i.i, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 0), align 4
  br label %lor.end.i

lor.end.i:                                        ; preds = %init_mparams.exit.i, %entry
  %1 = load %struct.malloc_chunk*, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 6), align 4
  %cmp1.i = icmp eq %struct.malloc_chunk* %1, null
  br i1 %cmp1.i, label %internal_malloc_stats.exit, label %if.then.i

if.then.i:                                        ; preds = %lor.end.i
  %2 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 13), align 4
  %3 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 12), align 4
  %4 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 3), align 4
  %add.neg.i = add i32 %3, -40
  %sub.i = sub i32 %add.neg.i, %4
  br label %while.body.i

while.body.i:                                     ; preds = %while.end.i, %if.then.i
  %s.05.i = phi %struct.malloc_segment* [ getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 16), %if.then.i ], [ %11, %while.end.i ]
  %used.04.i = phi i32 [ %sub.i, %if.then.i ], [ %used.1.lcssa.i, %while.end.i ]
  %base.i = getelementptr inbounds %struct.malloc_segment, %struct.malloc_segment* %s.05.i, i32 0, i32 0
  %5 = load i8*, i8** %base.i, align 4
  %add.ptr.i = getelementptr inbounds i8, i8* %5, i32 8
  %6 = ptrtoint i8* %add.ptr.i to i32
  %and.i = and i32 %6, 7
  %cmp4.i = icmp eq i32 %and.i, 0
  br i1 %cmp4.i, label %land.lhs.true.lr.ph.i, label %cond.false.i

cond.false.i:                                     ; preds = %while.body.i
  %7 = sub i32 0, %6
  %and9.i = and i32 %7, 7
  br label %land.lhs.true.lr.ph.i

land.lhs.true.lr.ph.i:                            ; preds = %cond.false.i, %while.body.i
  %cond.i = phi i32 [ %and9.i, %cond.false.i ], [ 0, %while.body.i ]
  %add.ptr10.i = getelementptr inbounds i8, i8* %5, i32 %cond.i
  %size.i = getelementptr inbounds %struct.malloc_segment, %struct.malloc_segment* %s.05.i, i32 0, i32 1
  %8 = load i32, i32* %size.i, align 4
  %add.ptr15.i = getelementptr inbounds i8, i8* %5, i32 %8
  br label %land.lhs.true.i

land.lhs.true.i:                                  ; preds = %while.body21.i, %land.lhs.true.lr.ph.i
  %q.0.in3.i = phi i8* [ %add.ptr10.i, %land.lhs.true.lr.ph.i ], [ %add.ptr31.i, %while.body21.i ]
  %used.12.i = phi i32 [ %used.04.i, %land.lhs.true.lr.ph.i ], [ %used.2.i, %while.body21.i ]
  %q.0.i = bitcast i8* %q.0.in3.i to %struct.malloc_chunk*
  %cmp16.i = icmp uge i8* %q.0.in3.i, %add.ptr15.i
  %cmp19.i = icmp eq %struct.malloc_chunk* %q.0.i, %1
  %or.cond.i = or i1 %cmp16.i, %cmp19.i
  br i1 %or.cond.i, label %while.end.i, label %land.rhs.i

land.rhs.i:                                       ; preds = %land.lhs.true.i
  %head.i = getelementptr inbounds i8, i8* %q.0.in3.i, i32 4
  %9 = bitcast i8* %head.i to i32*
  %10 = load i32, i32* %9, align 4
  %cmp20.i = icmp eq i32 %10, 7
  br i1 %cmp20.i, label %while.end.i, label %while.body21.i

while.body21.i:                                   ; preds = %land.rhs.i
  %and23.i = and i32 %10, 3
  %cmp24.i = icmp eq i32 %and23.i, 1
  %and27.i = and i32 %10, -8
  %sub28.i = select i1 %cmp24.i, i32 %and27.i, i32 0
  %used.2.i = sub i32 %used.12.i, %sub28.i
  %add.ptr31.i = getelementptr inbounds i8, i8* %q.0.in3.i, i32 %and27.i
  %cmp13.i = icmp ult i8* %add.ptr31.i, %5
  br i1 %cmp13.i, label %while.end.i, label %land.lhs.true.i

while.end.i:                                      ; preds = %while.body21.i, %land.rhs.i, %land.lhs.true.i
  %used.1.lcssa.i = phi i32 [ %used.12.i, %land.rhs.i ], [ %used.12.i, %land.lhs.true.i ], [ %used.2.i, %while.body21.i ]
  %next.i = getelementptr inbounds %struct.malloc_segment, %struct.malloc_segment* %s.05.i, i32 0, i32 2
  %11 = load %struct.malloc_segment*, %struct.malloc_segment** %next.i, align 4
  %cmp2.i = icmp eq %struct.malloc_segment* %11, null
  br i1 %cmp2.i, label %internal_malloc_stats.exit, label %while.body.i

internal_malloc_stats.exit:                       ; preds = %while.end.i, %lor.end.i
  %maxfp.0.i = phi i32 [ 0, %lor.end.i ], [ %2, %while.end.i ]
  %fp.0.i = phi i32 [ 0, %lor.end.i ], [ %3, %while.end.i ]
  %used.3.i = phi i32 [ 0, %lor.end.i ], [ %used.1.lcssa.i, %while.end.i ]
  %12 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 4
  %call34.i = tail call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %12, i8* getelementptr inbounds ([26 x i8], [26 x i8]* @.str49, i32 0, i32 0), i32 %maxfp.0.i) #1
  %call35.i = tail call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %12, i8* getelementptr inbounds ([26 x i8], [26 x i8]* @.str150, i32 0, i32 0), i32 %fp.0.i) #1
  %call36.i = tail call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %12, i8* getelementptr inbounds ([26 x i8], [26 x i8]* @.str251, i32 0, i32 0), i32 %used.3.i) #1
  ret void
}

; Function Attrs: nounwind
define weak i32 @mallopt(i32 %param_number, i32 %value) #0 {
entry:
  %0 = load i32, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 0), align 4
  %cmp.i = icmp eq i32 %0, 0
  br i1 %cmp.i, label %if.then.i.i, label %lor.end.i

if.then.i.i:                                      ; preds = %entry
  %call.i.i = tail call i32 @sysconf(i32 30) #1
  %sub.i.i = add i32 %call.i.i, -1
  %and.i.i = and i32 %sub.i.i, %call.i.i
  %cmp1.i.i = icmp eq i32 %and.i.i, 0
  br i1 %cmp1.i.i, label %init_mparams.exit.i, label %if.then5.i.i

if.then5.i.i:                                     ; preds = %if.then.i.i
  tail call void @abort() #6
  unreachable

init_mparams.exit.i:                              ; preds = %if.then.i.i
  store i32 %call.i.i, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 2), align 4
  store i32 %call.i.i, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 1), align 4
  store i32 -1, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 3), align 4
  store i32 -1, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 4), align 4
  store i32 0, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 5), align 4
  store i32 0, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 15), align 4
  %call6.i.i = tail call i32 @time(i32* null) #1
  %xor.i.i = and i32 %call6.i.i, -16
  %and7.i.i = xor i32 %xor.i.i, 1431655768
  store volatile i32 %and7.i.i, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 0), align 4
  br label %lor.end.i

lor.end.i:                                        ; preds = %init_mparams.exit.i, %entry
  switch i32 %param_number, label %change_mparam.exit [
    i32 -1, label %sw.bb.i
    i32 -2, label %sw.bb2.i
    i32 -3, label %sw.bb5.i
  ]

sw.bb.i:                                          ; preds = %lor.end.i
  store i32 %value, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 4), align 4
  br label %change_mparam.exit

sw.bb2.i:                                         ; preds = %lor.end.i
  %1 = load i32, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 1), align 4
  %cmp3.i = icmp ugt i32 %1, %value
  br i1 %cmp3.i, label %change_mparam.exit, label %land.lhs.true.i

land.lhs.true.i:                                  ; preds = %sw.bb2.i
  %sub.i = add i32 %value, -1
  %and.i = and i32 %sub.i, %value
  %cmp4.i = icmp eq i32 %and.i, 0
  br i1 %cmp4.i, label %if.then.i, label %change_mparam.exit

if.then.i:                                        ; preds = %land.lhs.true.i
  store i32 %value, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 2), align 4
  br label %change_mparam.exit

sw.bb5.i:                                         ; preds = %lor.end.i
  store i32 %value, i32* getelementptr inbounds (%struct.malloc_params, %struct.malloc_params* @mparams, i32 0, i32 3), align 4
  br label %change_mparam.exit

change_mparam.exit:                               ; preds = %sw.bb5.i, %if.then.i, %land.lhs.true.i, %sw.bb2.i, %sw.bb.i, %lor.end.i
  %retval.0.i = phi i32 [ 1, %sw.bb5.i ], [ 1, %if.then.i ], [ 1, %sw.bb.i ], [ 0, %sw.bb2.i ], [ 0, %land.lhs.true.i ], [ 0, %lor.end.i ]
  ret i32 %retval.0.i
}

; Function Attrs: nounwind
define weak i32 @malloc_usable_size(i8* %mem) #0 {
entry:
  %cmp = icmp eq i8* %mem, null
  br i1 %cmp, label %return, label %if.then

if.then:                                          ; preds = %entry
  %head = getelementptr inbounds i8, i8* %mem, i32 -4
  %0 = bitcast i8* %head to i32*
  %1 = load i32, i32* %0, align 4
  %and = and i32 %1, 3
  %cmp1 = icmp eq i32 %and, 1
  br i1 %cmp1, label %return, label %if.then2

if.then2:                                         ; preds = %if.then
  %and4 = and i32 %1, -8
  %cmp7 = icmp eq i32 %and, 0
  %cond = select i1 %cmp7, i32 8, i32 4
  %sub = sub i32 %and4, %cond
  br label %return

return:                                           ; preds = %if.then2, %if.then, %entry
  %retval.0 = phi i32 [ %sub, %if.then2 ], [ 0, %if.then ], [ 0, %entry ]
  ret i32 %retval.0
}

; Function Attrs: nounwind
declare i32 @fprintf(%struct._IO_FILE* nocapture, i8* nocapture, ...) #0

declare i8* @sbrk(i32) #5

; Function Attrs: nounwind
define internal fastcc void @dispose_chunk(%struct.malloc_chunk* %p, i32 %psize) #0 {
entry:
  %0 = bitcast %struct.malloc_chunk* %p to i8*
  %add.ptr = getelementptr inbounds i8, i8* %0, i32 %psize
  %1 = bitcast i8* %add.ptr to %struct.malloc_chunk*
  %head = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p, i32 0, i32 1
  %2 = load i32, i32* %head, align 4
  %and = and i32 %2, 1
  %tobool = icmp eq i32 %and, 0
  br i1 %tobool, label %if.then, label %if.end215

if.then:                                          ; preds = %entry
  %prev_foot = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p, i32 0, i32 0
  %3 = load i32, i32* %prev_foot, align 4
  %and2 = and i32 %2, 3
  %cmp = icmp eq i32 %and2, 0
  br i1 %cmp, label %if.end649, label %if.end

if.end:                                           ; preds = %if.then
  %idx.neg = sub i32 0, %3
  %add.ptr5 = getelementptr inbounds i8, i8* %0, i32 %idx.neg
  %4 = bitcast i8* %add.ptr5 to %struct.malloc_chunk*
  %add6 = add i32 %3, %psize
  %5 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp7 = icmp ult i8* %add.ptr5, %5
  br i1 %cmp7, label %if.else213, label %if.then9

if.then9:                                         ; preds = %if.end
  %6 = load %struct.malloc_chunk*, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 5), align 4
  %cmp10 = icmp eq %struct.malloc_chunk* %4, %6
  br i1 %cmp10, label %if.else200, label %if.then12

if.then12:                                        ; preds = %if.then9
  %shr = lshr i32 %3, 3
  %cmp13 = icmp ult i32 %3, 256
  br i1 %cmp13, label %if.then15, label %if.else59

if.then15:                                        ; preds = %if.then12
  %add.ptr5.sum24 = sub i32 8, %3
  %fd = getelementptr inbounds i8, i8* %0, i32 %add.ptr5.sum24
  %7 = bitcast i8* %fd to %struct.malloc_chunk**
  %8 = load %struct.malloc_chunk*, %struct.malloc_chunk** %7, align 4
  %add.ptr5.sum25 = sub i32 12, %3
  %bk = getelementptr inbounds i8, i8* %0, i32 %add.ptr5.sum25
  %9 = bitcast i8* %bk to %struct.malloc_chunk**
  %10 = load %struct.malloc_chunk*, %struct.malloc_chunk** %9, align 4
  %shl = shl nuw nsw i32 %shr, 1
  %arrayidx = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %shl
  %11 = bitcast %struct.malloc_chunk** %arrayidx to %struct.malloc_chunk*
  %cmp17 = icmp eq %struct.malloc_chunk* %8, %11
  br i1 %cmp17, label %if.then27, label %lor.rhs

lor.rhs:                                          ; preds = %if.then15
  %12 = bitcast %struct.malloc_chunk* %8 to i8*
  %cmp20 = icmp ult i8* %12, %5
  br i1 %cmp20, label %if.else57, label %land.rhs

land.rhs:                                         ; preds = %lor.rhs
  %bk22 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %8, i32 0, i32 3
  %13 = load %struct.malloc_chunk*, %struct.malloc_chunk** %bk22, align 4
  %cmp23 = icmp eq %struct.malloc_chunk* %13, %4
  br i1 %cmp23, label %if.then27, label %if.else57

if.then27:                                        ; preds = %land.rhs, %if.then15
  %cmp28 = icmp eq %struct.malloc_chunk* %10, %8
  br i1 %cmp28, label %if.then30, label %if.else

if.then30:                                        ; preds = %if.then27
  %shl31 = shl i32 1, %shr
  %neg = xor i32 %shl31, -1
  %14 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  %and32 = and i32 %14, %neg
  store i32 %and32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  br label %if.end215

if.else:                                          ; preds = %if.then27
  %cmp36 = icmp eq %struct.malloc_chunk* %10, %11
  br i1 %cmp36, label %if.else.if.then51_crit_edge, label %lor.rhs38

if.else.if.then51_crit_edge:                      ; preds = %if.else
  %fd53.pre = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %10, i32 0, i32 2
  br label %if.then51

lor.rhs38:                                        ; preds = %if.else
  %15 = bitcast %struct.malloc_chunk* %10 to i8*
  %cmp40 = icmp ult i8* %15, %5
  br i1 %cmp40, label %if.else54, label %land.rhs42

land.rhs42:                                       ; preds = %lor.rhs38
  %fd43 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %10, i32 0, i32 2
  %16 = load %struct.malloc_chunk*, %struct.malloc_chunk** %fd43, align 4
  %cmp44 = icmp eq %struct.malloc_chunk* %16, %4
  br i1 %cmp44, label %if.then51, label %if.else54

if.then51:                                        ; preds = %land.rhs42, %if.else.if.then51_crit_edge
  %fd53.pre-phi = phi %struct.malloc_chunk** [ %fd53.pre, %if.else.if.then51_crit_edge ], [ %fd43, %land.rhs42 ]
  %bk52 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %8, i32 0, i32 3
  store %struct.malloc_chunk* %10, %struct.malloc_chunk** %bk52, align 4
  store %struct.malloc_chunk* %8, %struct.malloc_chunk** %fd53.pre-phi, align 4
  br label %if.end215

if.else54:                                        ; preds = %land.rhs42, %lor.rhs38
  tail call void @abort() #6
  unreachable

if.else57:                                        ; preds = %land.rhs, %lor.rhs
  tail call void @abort() #6
  unreachable

if.else59:                                        ; preds = %if.then12
  %17 = bitcast i8* %add.ptr5 to %struct.malloc_tree_chunk*
  %add.ptr5.sum18 = sub i32 24, %3
  %parent = getelementptr inbounds i8, i8* %0, i32 %add.ptr5.sum18
  %18 = bitcast i8* %parent to %struct.malloc_tree_chunk**
  %19 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %18, align 4
  %add.ptr5.sum19 = sub i32 12, %3
  %bk60 = getelementptr inbounds i8, i8* %0, i32 %add.ptr5.sum19
  %20 = bitcast i8* %bk60 to %struct.malloc_tree_chunk**
  %21 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %20, align 4
  %cmp61 = icmp eq %struct.malloc_tree_chunk* %21, %17
  br i1 %cmp61, label %if.else85, label %if.then63

if.then63:                                        ; preds = %if.else59
  %add.ptr5.sum23 = sub i32 8, %3
  %fd65 = getelementptr inbounds i8, i8* %0, i32 %add.ptr5.sum23
  %22 = bitcast i8* %fd65 to %struct.malloc_tree_chunk**
  %23 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %22, align 4
  %24 = bitcast %struct.malloc_tree_chunk* %23 to i8*
  %cmp68 = icmp ult i8* %24, %5
  br i1 %cmp68, label %if.else83, label %land.lhs.true

land.lhs.true:                                    ; preds = %if.then63
  %bk70 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %23, i32 0, i32 3
  %25 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %bk70, align 4
  %cmp71 = icmp eq %struct.malloc_tree_chunk* %25, %17
  br i1 %cmp71, label %land.rhs73, label %if.else83

land.rhs73:                                       ; preds = %land.lhs.true
  %fd74 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %21, i32 0, i32 2
  %26 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %fd74, align 4
  %cmp75 = icmp eq %struct.malloc_tree_chunk* %26, %17
  br i1 %cmp75, label %if.then80, label %if.else83

if.then80:                                        ; preds = %land.rhs73
  store %struct.malloc_tree_chunk* %21, %struct.malloc_tree_chunk** %bk70, align 4
  store %struct.malloc_tree_chunk* %23, %struct.malloc_tree_chunk** %fd74, align 4
  br label %if.end114

if.else83:                                        ; preds = %land.rhs73, %land.lhs.true, %if.then63
  tail call void @abort() #6
  unreachable

if.else85:                                        ; preds = %if.else59
  %add.ptr5.sum20 = sub i32 16, %3
  %child.sum = add i32 %add.ptr5.sum20, 4
  %arrayidx86 = getelementptr inbounds i8, i8* %0, i32 %child.sum
  %27 = bitcast i8* %arrayidx86 to %struct.malloc_tree_chunk**
  %28 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %27, align 4
  %cmp87 = icmp eq %struct.malloc_tree_chunk* %28, null
  br i1 %cmp87, label %lor.lhs.false, label %while.cond

lor.lhs.false:                                    ; preds = %if.else85
  %child = getelementptr inbounds i8, i8* %0, i32 %add.ptr5.sum20
  %arrayidx90 = bitcast i8* %child to %struct.malloc_tree_chunk**
  %29 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx90, align 4
  %cmp91 = icmp eq %struct.malloc_tree_chunk* %29, null
  br i1 %cmp91, label %if.end114, label %while.cond

while.cond:                                       ; preds = %lor.rhs98, %while.cond, %lor.lhs.false, %if.else85
  %RP.0 = phi %struct.malloc_tree_chunk** [ %arrayidx90, %lor.lhs.false ], [ %27, %if.else85 ], [ %arrayidx95, %while.cond ], [ %arrayidx100, %lor.rhs98 ]
  %R.0 = phi %struct.malloc_tree_chunk* [ %29, %lor.lhs.false ], [ %28, %if.else85 ], [ %30, %while.cond ], [ %31, %lor.rhs98 ]
  %arrayidx95 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.0, i32 0, i32 4, i32 1
  %30 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx95, align 4
  %cmp96 = icmp eq %struct.malloc_tree_chunk* %30, null
  br i1 %cmp96, label %lor.rhs98, label %while.cond

lor.rhs98:                                        ; preds = %while.cond
  %arrayidx100 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.0, i32 0, i32 4, i32 0
  %31 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx100, align 4
  %cmp101 = icmp eq %struct.malloc_tree_chunk* %31, null
  br i1 %cmp101, label %while.end, label %while.cond

while.end:                                        ; preds = %lor.rhs98
  %32 = bitcast %struct.malloc_tree_chunk** %RP.0 to i8*
  %cmp106 = icmp ult i8* %32, %5
  br i1 %cmp106, label %if.else111, label %if.then110

if.then110:                                       ; preds = %while.end
  store %struct.malloc_tree_chunk* null, %struct.malloc_tree_chunk** %RP.0, align 4
  br label %if.end114

if.else111:                                       ; preds = %while.end
  tail call void @abort() #6
  unreachable

if.end114:                                        ; preds = %if.then110, %lor.lhs.false, %if.then80
  %R.1 = phi %struct.malloc_tree_chunk* [ %21, %if.then80 ], [ %R.0, %if.then110 ], [ null, %lor.lhs.false ]
  %cmp115 = icmp eq %struct.malloc_tree_chunk* %19, null
  br i1 %cmp115, label %if.end215, label %if.then117

if.then117:                                       ; preds = %if.end114
  %add.ptr5.sum21 = sub i32 28, %3
  %index = getelementptr inbounds i8, i8* %0, i32 %add.ptr5.sum21
  %33 = bitcast i8* %index to i32*
  %34 = load i32, i32* %33, align 4
  %arrayidx118 = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 11, i32 %34
  %35 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx118, align 4
  %cmp119 = icmp eq %struct.malloc_tree_chunk* %17, %35
  br i1 %cmp119, label %if.then121, label %if.else130

if.then121:                                       ; preds = %if.then117
  store %struct.malloc_tree_chunk* %R.1, %struct.malloc_tree_chunk** %arrayidx118, align 4
  %cond29 = icmp eq %struct.malloc_tree_chunk* %R.1, null
  br i1 %cond29, label %if.end150.thread, label %if.then153

if.end150.thread:                                 ; preds = %if.then121
  %shl126 = shl i32 1, %34
  %neg127 = xor i32 %shl126, -1
  %36 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 1), align 4
  %and128 = and i32 %36, %neg127
  store i32 %and128, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 1), align 4
  br label %if.end215

if.else130:                                       ; preds = %if.then117
  %37 = bitcast %struct.malloc_tree_chunk* %19 to i8*
  %38 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp132 = icmp ult i8* %37, %38
  br i1 %cmp132, label %if.else148, label %if.then136

if.then136:                                       ; preds = %if.else130
  %arrayidx138 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %19, i32 0, i32 4, i32 0
  %39 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx138, align 4
  %cmp139 = icmp eq %struct.malloc_tree_chunk* %39, %17
  br i1 %cmp139, label %if.then141, label %if.else144

if.then141:                                       ; preds = %if.then136
  store %struct.malloc_tree_chunk* %R.1, %struct.malloc_tree_chunk** %arrayidx138, align 4
  br label %if.end150

if.else144:                                       ; preds = %if.then136
  %arrayidx146 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %19, i32 0, i32 4, i32 1
  store %struct.malloc_tree_chunk* %R.1, %struct.malloc_tree_chunk** %arrayidx146, align 4
  br label %if.end150

if.else148:                                       ; preds = %if.else130
  tail call void @abort() #6
  unreachable

if.end150:                                        ; preds = %if.else144, %if.then141
  %cmp151 = icmp eq %struct.malloc_tree_chunk* %R.1, null
  br i1 %cmp151, label %if.end215, label %if.then153

if.then153:                                       ; preds = %if.end150, %if.then121
  %40 = bitcast %struct.malloc_tree_chunk* %R.1 to i8*
  %41 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp155 = icmp ult i8* %40, %41
  br i1 %cmp155, label %if.else195, label %if.then159

if.then159:                                       ; preds = %if.then153
  %parent160 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.1, i32 0, i32 5
  store %struct.malloc_tree_chunk* %19, %struct.malloc_tree_chunk** %parent160, align 4
  %add.ptr5.sum22 = sub i32 16, %3
  %child161 = getelementptr inbounds i8, i8* %0, i32 %add.ptr5.sum22
  %arrayidx162 = bitcast i8* %child161 to %struct.malloc_tree_chunk**
  %42 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx162, align 4
  %cmp163 = icmp eq %struct.malloc_tree_chunk* %42, null
  br i1 %cmp163, label %if.end177, label %if.then165

if.then165:                                       ; preds = %if.then159
  %43 = bitcast %struct.malloc_tree_chunk* %42 to i8*
  %44 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp167 = icmp ult i8* %43, %44
  br i1 %cmp167, label %if.else175, label %if.then171

if.then171:                                       ; preds = %if.then165
  %arrayidx173 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.1, i32 0, i32 4, i32 0
  store %struct.malloc_tree_chunk* %42, %struct.malloc_tree_chunk** %arrayidx173, align 4
  %parent174 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %42, i32 0, i32 5
  store %struct.malloc_tree_chunk* %R.1, %struct.malloc_tree_chunk** %parent174, align 4
  br label %if.end177

if.else175:                                       ; preds = %if.then165
  tail call void @abort() #6
  unreachable

if.end177:                                        ; preds = %if.then171, %if.then159
  %child161.sum = add i32 %add.ptr5.sum22, 4
  %arrayidx179 = getelementptr inbounds i8, i8* %0, i32 %child161.sum
  %45 = bitcast i8* %arrayidx179 to %struct.malloc_tree_chunk**
  %46 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %45, align 4
  %cmp180 = icmp eq %struct.malloc_tree_chunk* %46, null
  br i1 %cmp180, label %if.end215, label %if.then182

if.then182:                                       ; preds = %if.end177
  %47 = bitcast %struct.malloc_tree_chunk* %46 to i8*
  %48 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp184 = icmp ult i8* %47, %48
  br i1 %cmp184, label %if.else192, label %if.then188

if.then188:                                       ; preds = %if.then182
  %arrayidx190 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R.1, i32 0, i32 4, i32 1
  store %struct.malloc_tree_chunk* %46, %struct.malloc_tree_chunk** %arrayidx190, align 4
  %parent191 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %46, i32 0, i32 5
  store %struct.malloc_tree_chunk* %R.1, %struct.malloc_tree_chunk** %parent191, align 4
  br label %if.end215

if.else192:                                       ; preds = %if.then182
  tail call void @abort() #6
  unreachable

if.else195:                                       ; preds = %if.then153
  tail call void @abort() #6
  unreachable

if.else200:                                       ; preds = %if.then9
  %add.ptr.sum = add i32 %psize, 4
  %head201 = getelementptr inbounds i8, i8* %0, i32 %add.ptr.sum
  %49 = bitcast i8* %head201 to i32*
  %50 = load i32, i32* %49, align 4
  %and202 = and i32 %50, 3
  %cmp203 = icmp eq i32 %and202, 3
  br i1 %cmp203, label %if.then205, label %if.end215

if.then205:                                       ; preds = %if.else200
  store i32 %add6, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 2), align 4
  %51 = load i32, i32* %49, align 4
  %and207 = and i32 %51, -2
  store i32 %and207, i32* %49, align 4
  %or = or i32 %add6, 1
  %add.ptr5.sum = sub i32 4, %3
  %head208 = getelementptr inbounds i8, i8* %0, i32 %add.ptr5.sum
  %52 = bitcast i8* %head208 to i32*
  store i32 %or, i32* %52, align 4
  %prev_foot210 = bitcast i8* %add.ptr to i32*
  store i32 %add6, i32* %prev_foot210, align 4
  br label %if.end649

if.else213:                                       ; preds = %if.end
  tail call void @abort() #6
  unreachable

if.end215:                                        ; preds = %if.else200, %if.then188, %if.end177, %if.end150, %if.end150.thread, %if.end114, %if.then51, %if.then30, %entry
  %psize.addr.0 = phi i32 [ %add6, %if.then30 ], [ %add6, %if.then51 ], [ %add6, %if.then188 ], [ %add6, %if.else200 ], [ %psize, %entry ], [ %add6, %if.end114 ], [ %add6, %if.end150.thread ], [ %add6, %if.end150 ], [ %add6, %if.end177 ]
  %p.addr.0 = phi %struct.malloc_chunk* [ %4, %if.then30 ], [ %4, %if.then51 ], [ %4, %if.then188 ], [ %4, %if.else200 ], [ %p, %entry ], [ %4, %if.end114 ], [ %4, %if.end150.thread ], [ %4, %if.end150 ], [ %4, %if.end177 ]
  %53 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp217 = icmp ult i8* %add.ptr, %53
  br i1 %cmp217, label %if.else648, label %if.then221

if.then221:                                       ; preds = %if.end215
  %add.ptr.sum1 = add i32 %psize, 4
  %head222 = getelementptr inbounds i8, i8* %0, i32 %add.ptr.sum1
  %54 = bitcast i8* %head222 to i32*
  %55 = load i32, i32* %54, align 4
  %and223 = and i32 %55, 2
  %tobool224 = icmp eq i32 %and223, 0
  br i1 %tobool224, label %if.then225, label %if.else493

if.then225:                                       ; preds = %if.then221
  %56 = load %struct.malloc_chunk*, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 6), align 4
  %cmp226 = icmp eq %struct.malloc_chunk* %1, %56
  br i1 %cmp226, label %if.then228, label %if.else240

if.then228:                                       ; preds = %if.then225
  %57 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 3), align 4
  %add229 = add i32 %57, %psize.addr.0
  store i32 %add229, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 3), align 4
  store %struct.malloc_chunk* %p.addr.0, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 6), align 4
  %or231 = or i32 %add229, 1
  %head232 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.addr.0, i32 0, i32 1
  store i32 %or231, i32* %head232, align 4
  %58 = load %struct.malloc_chunk*, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 5), align 4
  %cmp234 = icmp eq %struct.malloc_chunk* %p.addr.0, %58
  br i1 %cmp234, label %if.then236, label %if.end649

if.then236:                                       ; preds = %if.then228
  store %struct.malloc_chunk* null, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 5), align 4
  store i32 0, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 2), align 4
  br label %if.end649

if.else240:                                       ; preds = %if.then225
  %59 = load %struct.malloc_chunk*, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 5), align 4
  %cmp242 = icmp eq %struct.malloc_chunk* %1, %59
  br i1 %cmp242, label %if.then244, label %if.else252

if.then244:                                       ; preds = %if.else240
  %60 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 2), align 4
  %add246 = add i32 %60, %psize.addr.0
  store i32 %add246, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 2), align 4
  store %struct.malloc_chunk* %p.addr.0, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 5), align 4
  %or248 = or i32 %add246, 1
  %head249 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.addr.0, i32 0, i32 1
  store i32 %or248, i32* %head249, align 4
  %61 = bitcast %struct.malloc_chunk* %p.addr.0 to i8*
  %add.ptr250 = getelementptr inbounds i8, i8* %61, i32 %add246
  %prev_foot251 = bitcast i8* %add.ptr250 to i32*
  store i32 %add246, i32* %prev_foot251, align 4
  br label %if.end649

if.else252:                                       ; preds = %if.else240
  %and254 = and i32 %55, -8
  %add255 = add i32 %and254, %psize.addr.0
  %shr256 = lshr i32 %55, 3
  %cmp257 = icmp ult i32 %55, 256
  br i1 %cmp257, label %if.then259, label %if.else321

if.then259:                                       ; preds = %if.else252
  %add.ptr.sum15 = add i32 %psize, 8
  %fd261 = getelementptr inbounds i8, i8* %0, i32 %add.ptr.sum15
  %62 = bitcast i8* %fd261 to %struct.malloc_chunk**
  %63 = load %struct.malloc_chunk*, %struct.malloc_chunk** %62, align 4
  %add.ptr.sum16 = add i32 %psize, 12
  %bk263 = getelementptr inbounds i8, i8* %0, i32 %add.ptr.sum16
  %64 = bitcast i8* %bk263 to %struct.malloc_chunk**
  %65 = load %struct.malloc_chunk*, %struct.malloc_chunk** %64, align 4
  %shl266 = shl nuw nsw i32 %shr256, 1
  %arrayidx268 = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %shl266
  %66 = bitcast %struct.malloc_chunk** %arrayidx268 to %struct.malloc_chunk*
  %cmp269 = icmp eq %struct.malloc_chunk* %63, %66
  br i1 %cmp269, label %if.then285, label %lor.rhs271

lor.rhs271:                                       ; preds = %if.then259
  %67 = bitcast %struct.malloc_chunk* %63 to i8*
  %cmp273 = icmp ult i8* %67, %53
  br i1 %cmp273, label %if.else319, label %land.rhs275

land.rhs275:                                      ; preds = %lor.rhs271
  %bk276 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %63, i32 0, i32 3
  %68 = load %struct.malloc_chunk*, %struct.malloc_chunk** %bk276, align 4
  %cmp277 = icmp eq %struct.malloc_chunk* %68, %1
  br i1 %cmp277, label %if.then285, label %if.else319

if.then285:                                       ; preds = %land.rhs275, %if.then259
  %cmp286 = icmp eq %struct.malloc_chunk* %65, %63
  br i1 %cmp286, label %if.then288, label %if.else293

if.then288:                                       ; preds = %if.then285
  %shl289 = shl i32 1, %shr256
  %neg290 = xor i32 %shl289, -1
  %69 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  %and292 = and i32 %69, %neg290
  store i32 %and292, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  br label %if.end480

if.else293:                                       ; preds = %if.then285
  %cmp297 = icmp eq %struct.malloc_chunk* %65, %66
  br i1 %cmp297, label %if.else293.if.then313_crit_edge, label %lor.rhs299

if.else293.if.then313_crit_edge:                  ; preds = %if.else293
  %fd315.pre = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %65, i32 0, i32 2
  br label %if.then313

lor.rhs299:                                       ; preds = %if.else293
  %70 = bitcast %struct.malloc_chunk* %65 to i8*
  %cmp301 = icmp ult i8* %70, %53
  br i1 %cmp301, label %if.else316, label %land.rhs303

land.rhs303:                                      ; preds = %lor.rhs299
  %fd304 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %65, i32 0, i32 2
  %71 = load %struct.malloc_chunk*, %struct.malloc_chunk** %fd304, align 4
  %cmp305 = icmp eq %struct.malloc_chunk* %71, %1
  br i1 %cmp305, label %if.then313, label %if.else316

if.then313:                                       ; preds = %land.rhs303, %if.else293.if.then313_crit_edge
  %fd315.pre-phi = phi %struct.malloc_chunk** [ %fd315.pre, %if.else293.if.then313_crit_edge ], [ %fd304, %land.rhs303 ]
  %bk314 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %63, i32 0, i32 3
  store %struct.malloc_chunk* %65, %struct.malloc_chunk** %bk314, align 4
  store %struct.malloc_chunk* %63, %struct.malloc_chunk** %fd315.pre-phi, align 4
  br label %if.end480

if.else316:                                       ; preds = %land.rhs303, %lor.rhs299
  tail call void @abort() #6
  unreachable

if.else319:                                       ; preds = %land.rhs275, %lor.rhs271
  tail call void @abort() #6
  unreachable

if.else321:                                       ; preds = %if.else252
  %72 = bitcast i8* %add.ptr to %struct.malloc_tree_chunk*
  %add.ptr.sum2 = add i32 %psize, 24
  %parent324 = getelementptr inbounds i8, i8* %0, i32 %add.ptr.sum2
  %73 = bitcast i8* %parent324 to %struct.malloc_tree_chunk**
  %74 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %73, align 4
  %add.ptr.sum3 = add i32 %psize, 12
  %bk326 = getelementptr inbounds i8, i8* %0, i32 %add.ptr.sum3
  %75 = bitcast i8* %bk326 to %struct.malloc_tree_chunk**
  %76 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %75, align 4
  %cmp327 = icmp eq %struct.malloc_tree_chunk* %76, %72
  br i1 %cmp327, label %if.else353, label %if.then329

if.then329:                                       ; preds = %if.else321
  %add.ptr.sum14 = add i32 %psize, 8
  %fd331 = getelementptr inbounds i8, i8* %0, i32 %add.ptr.sum14
  %77 = bitcast i8* %fd331 to %struct.malloc_tree_chunk**
  %78 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %77, align 4
  %79 = bitcast %struct.malloc_tree_chunk* %78 to i8*
  %cmp334 = icmp ult i8* %79, %53
  br i1 %cmp334, label %if.else351, label %land.lhs.true336

land.lhs.true336:                                 ; preds = %if.then329
  %bk337 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %78, i32 0, i32 3
  %80 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %bk337, align 4
  %cmp338 = icmp eq %struct.malloc_tree_chunk* %80, %72
  br i1 %cmp338, label %land.rhs340, label %if.else351

land.rhs340:                                      ; preds = %land.lhs.true336
  %fd341 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %76, i32 0, i32 2
  %81 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %fd341, align 4
  %cmp342 = icmp eq %struct.malloc_tree_chunk* %81, %72
  br i1 %cmp342, label %if.then348, label %if.else351

if.then348:                                       ; preds = %land.rhs340
  store %struct.malloc_tree_chunk* %76, %struct.malloc_tree_chunk** %bk337, align 4
  store %struct.malloc_tree_chunk* %78, %struct.malloc_tree_chunk** %fd341, align 4
  br label %if.end389

if.else351:                                       ; preds = %land.rhs340, %land.lhs.true336, %if.then329
  tail call void @abort() #6
  unreachable

if.else353:                                       ; preds = %if.else321
  %child355.sum = add i32 %psize, 20
  %arrayidx356 = getelementptr inbounds i8, i8* %0, i32 %child355.sum
  %82 = bitcast i8* %arrayidx356 to %struct.malloc_tree_chunk**
  %83 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %82, align 4
  %cmp357 = icmp eq %struct.malloc_tree_chunk* %83, null
  br i1 %cmp357, label %lor.lhs.false359, label %while.cond366

lor.lhs.false359:                                 ; preds = %if.else353
  %add.ptr.sum4 = add i32 %psize, 16
  %child355 = getelementptr inbounds i8, i8* %0, i32 %add.ptr.sum4
  %arrayidx361 = bitcast i8* %child355 to %struct.malloc_tree_chunk**
  %84 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx361, align 4
  %cmp362 = icmp eq %struct.malloc_tree_chunk* %84, null
  br i1 %cmp362, label %if.end389, label %while.cond366

while.cond366:                                    ; preds = %lor.rhs371, %while.cond366, %lor.lhs.false359, %if.else353
  %RP354.0 = phi %struct.malloc_tree_chunk** [ %arrayidx361, %lor.lhs.false359 ], [ %82, %if.else353 ], [ %arrayidx368, %while.cond366 ], [ %arrayidx373, %lor.rhs371 ]
  %R325.0 = phi %struct.malloc_tree_chunk* [ %84, %lor.lhs.false359 ], [ %83, %if.else353 ], [ %85, %while.cond366 ], [ %86, %lor.rhs371 ]
  %arrayidx368 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R325.0, i32 0, i32 4, i32 1
  %85 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx368, align 4
  %cmp369 = icmp eq %struct.malloc_tree_chunk* %85, null
  br i1 %cmp369, label %lor.rhs371, label %while.cond366

lor.rhs371:                                       ; preds = %while.cond366
  %arrayidx373 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R325.0, i32 0, i32 4, i32 0
  %86 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx373, align 4
  %cmp374 = icmp eq %struct.malloc_tree_chunk* %86, null
  br i1 %cmp374, label %while.end379, label %while.cond366

while.end379:                                     ; preds = %lor.rhs371
  %87 = bitcast %struct.malloc_tree_chunk** %RP354.0 to i8*
  %cmp381 = icmp ult i8* %87, %53
  br i1 %cmp381, label %if.else386, label %if.then385

if.then385:                                       ; preds = %while.end379
  store %struct.malloc_tree_chunk* null, %struct.malloc_tree_chunk** %RP354.0, align 4
  br label %if.end389

if.else386:                                       ; preds = %while.end379
  tail call void @abort() #6
  unreachable

if.end389:                                        ; preds = %if.then385, %lor.lhs.false359, %if.then348
  %R325.1 = phi %struct.malloc_tree_chunk* [ %76, %if.then348 ], [ %R325.0, %if.then385 ], [ null, %lor.lhs.false359 ]
  %cmp390 = icmp eq %struct.malloc_tree_chunk* %74, null
  br i1 %cmp390, label %if.end480, label %if.then392

if.then392:                                       ; preds = %if.end389
  %add.ptr.sum12 = add i32 %psize, 28
  %index394 = getelementptr inbounds i8, i8* %0, i32 %add.ptr.sum12
  %88 = bitcast i8* %index394 to i32*
  %89 = load i32, i32* %88, align 4
  %arrayidx396 = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 11, i32 %89
  %90 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx396, align 4
  %cmp397 = icmp eq %struct.malloc_tree_chunk* %72, %90
  br i1 %cmp397, label %if.then399, label %if.else409

if.then399:                                       ; preds = %if.then392
  store %struct.malloc_tree_chunk* %R325.1, %struct.malloc_tree_chunk** %arrayidx396, align 4
  %cond30 = icmp eq %struct.malloc_tree_chunk* %R325.1, null
  br i1 %cond30, label %if.end429.thread, label %if.then432

if.end429.thread:                                 ; preds = %if.then399
  %shl404 = shl i32 1, %89
  %neg405 = xor i32 %shl404, -1
  %91 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 1), align 4
  %and407 = and i32 %91, %neg405
  store i32 %and407, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 1), align 4
  br label %if.end480

if.else409:                                       ; preds = %if.then392
  %92 = bitcast %struct.malloc_tree_chunk* %74 to i8*
  %93 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp411 = icmp ult i8* %92, %93
  br i1 %cmp411, label %if.else427, label %if.then415

if.then415:                                       ; preds = %if.else409
  %arrayidx417 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %74, i32 0, i32 4, i32 0
  %94 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx417, align 4
  %cmp418 = icmp eq %struct.malloc_tree_chunk* %94, %72
  br i1 %cmp418, label %if.then420, label %if.else423

if.then420:                                       ; preds = %if.then415
  store %struct.malloc_tree_chunk* %R325.1, %struct.malloc_tree_chunk** %arrayidx417, align 4
  br label %if.end429

if.else423:                                       ; preds = %if.then415
  %arrayidx425 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %74, i32 0, i32 4, i32 1
  store %struct.malloc_tree_chunk* %R325.1, %struct.malloc_tree_chunk** %arrayidx425, align 4
  br label %if.end429

if.else427:                                       ; preds = %if.else409
  tail call void @abort() #6
  unreachable

if.end429:                                        ; preds = %if.else423, %if.then420
  %cmp430 = icmp eq %struct.malloc_tree_chunk* %R325.1, null
  br i1 %cmp430, label %if.end480, label %if.then432

if.then432:                                       ; preds = %if.end429, %if.then399
  %95 = bitcast %struct.malloc_tree_chunk* %R325.1 to i8*
  %96 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp434 = icmp ult i8* %95, %96
  br i1 %cmp434, label %if.else476, label %if.then438

if.then438:                                       ; preds = %if.then432
  %parent441 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R325.1, i32 0, i32 5
  store %struct.malloc_tree_chunk* %74, %struct.malloc_tree_chunk** %parent441, align 4
  %add.ptr.sum13 = add i32 %psize, 16
  %child442 = getelementptr inbounds i8, i8* %0, i32 %add.ptr.sum13
  %arrayidx443 = bitcast i8* %child442 to %struct.malloc_tree_chunk**
  %97 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx443, align 4
  %cmp444 = icmp eq %struct.malloc_tree_chunk* %97, null
  br i1 %cmp444, label %if.end458, label %if.then446

if.then446:                                       ; preds = %if.then438
  %98 = bitcast %struct.malloc_tree_chunk* %97 to i8*
  %99 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp448 = icmp ult i8* %98, %99
  br i1 %cmp448, label %if.else456, label %if.then452

if.then452:                                       ; preds = %if.then446
  %arrayidx454 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R325.1, i32 0, i32 4, i32 0
  store %struct.malloc_tree_chunk* %97, %struct.malloc_tree_chunk** %arrayidx454, align 4
  %parent455 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %97, i32 0, i32 5
  store %struct.malloc_tree_chunk* %R325.1, %struct.malloc_tree_chunk** %parent455, align 4
  br label %if.end458

if.else456:                                       ; preds = %if.then446
  tail call void @abort() #6
  unreachable

if.end458:                                        ; preds = %if.then452, %if.then438
  %child442.sum = add i32 %psize, 20
  %arrayidx460 = getelementptr inbounds i8, i8* %0, i32 %child442.sum
  %100 = bitcast i8* %arrayidx460 to %struct.malloc_tree_chunk**
  %101 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %100, align 4
  %cmp461 = icmp eq %struct.malloc_tree_chunk* %101, null
  br i1 %cmp461, label %if.end480, label %if.then463

if.then463:                                       ; preds = %if.end458
  %102 = bitcast %struct.malloc_tree_chunk* %101 to i8*
  %103 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp465 = icmp ult i8* %102, %103
  br i1 %cmp465, label %if.else473, label %if.then469

if.then469:                                       ; preds = %if.then463
  %arrayidx471 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %R325.1, i32 0, i32 4, i32 1
  store %struct.malloc_tree_chunk* %101, %struct.malloc_tree_chunk** %arrayidx471, align 4
  %parent472 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %101, i32 0, i32 5
  store %struct.malloc_tree_chunk* %R325.1, %struct.malloc_tree_chunk** %parent472, align 4
  br label %if.end480

if.else473:                                       ; preds = %if.then463
  tail call void @abort() #6
  unreachable

if.else476:                                       ; preds = %if.then432
  tail call void @abort() #6
  unreachable

if.end480:                                        ; preds = %if.then469, %if.end458, %if.end429, %if.end429.thread, %if.end389, %if.then313, %if.then288
  %or481 = or i32 %add255, 1
  %head482 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.addr.0, i32 0, i32 1
  store i32 %or481, i32* %head482, align 4
  %104 = bitcast %struct.malloc_chunk* %p.addr.0 to i8*
  %add.ptr483 = getelementptr inbounds i8, i8* %104, i32 %add255
  %prev_foot484 = bitcast i8* %add.ptr483 to i32*
  store i32 %add255, i32* %prev_foot484, align 4
  %105 = load %struct.malloc_chunk*, %struct.malloc_chunk** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 5), align 4
  %cmp486 = icmp eq %struct.malloc_chunk* %p.addr.0, %105
  br i1 %cmp486, label %if.then488, label %if.end500

if.then488:                                       ; preds = %if.end480
  store i32 %add255, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 2), align 4
  br label %if.end649

if.else493:                                       ; preds = %if.then221
  %and495 = and i32 %55, -2
  store i32 %and495, i32* %54, align 4
  %or496 = or i32 %psize.addr.0, 1
  %head497 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.addr.0, i32 0, i32 1
  store i32 %or496, i32* %head497, align 4
  %106 = bitcast %struct.malloc_chunk* %p.addr.0 to i8*
  %add.ptr498 = getelementptr inbounds i8, i8* %106, i32 %psize.addr.0
  %prev_foot499 = bitcast i8* %add.ptr498 to i32*
  store i32 %psize.addr.0, i32* %prev_foot499, align 4
  br label %if.end500

if.end500:                                        ; preds = %if.else493, %if.end480
  %psize.addr.1 = phi i32 [ %psize.addr.0, %if.else493 ], [ %add255, %if.end480 ]
  %shr501 = lshr i32 %psize.addr.1, 3
  %cmp502 = icmp ult i32 %psize.addr.1, 256
  br i1 %cmp502, label %if.then504, label %if.else536

if.then504:                                       ; preds = %if.end500
  %shl508 = shl nuw nsw i32 %shr501, 1
  %arrayidx510 = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %shl508
  %107 = bitcast %struct.malloc_chunk** %arrayidx510 to %struct.malloc_chunk*
  %108 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  %shl513 = shl i32 1, %shr501
  %and514 = and i32 %108, %shl513
  %tobool515 = icmp eq i32 %and514, 0
  br i1 %tobool515, label %if.then516, label %if.else520

if.then516:                                       ; preds = %if.then504
  %or519 = or i32 %108, %shl513
  store i32 %or519, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 0), align 4
  %arrayidx510.sum.pre = add i32 %shl508, 2
  %.pre = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %arrayidx510.sum.pre
  br label %if.end531

if.else520:                                       ; preds = %if.then504
  %arrayidx510.sum11 = add i32 %shl508, 2
  %109 = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 10, i32 %arrayidx510.sum11
  %110 = load %struct.malloc_chunk*, %struct.malloc_chunk** %109, align 4
  %111 = bitcast %struct.malloc_chunk* %110 to i8*
  %112 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp523 = icmp ult i8* %111, %112
  br i1 %cmp523, label %if.else529, label %if.end531

if.else529:                                       ; preds = %if.else520
  tail call void @abort() #6
  unreachable

if.end531:                                        ; preds = %if.else520, %if.then516
  %.pre-phi = phi %struct.malloc_chunk** [ %109, %if.else520 ], [ %.pre, %if.then516 ]
  %F511.0 = phi %struct.malloc_chunk* [ %110, %if.else520 ], [ %107, %if.then516 ]
  store %struct.malloc_chunk* %p.addr.0, %struct.malloc_chunk** %.pre-phi, align 4
  %bk533 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %F511.0, i32 0, i32 3
  store %struct.malloc_chunk* %p.addr.0, %struct.malloc_chunk** %bk533, align 4
  %fd534 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.addr.0, i32 0, i32 2
  store %struct.malloc_chunk* %F511.0, %struct.malloc_chunk** %fd534, align 4
  %bk535 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.addr.0, i32 0, i32 3
  store %struct.malloc_chunk* %107, %struct.malloc_chunk** %bk535, align 4
  br label %if.end649

if.else536:                                       ; preds = %if.end500
  %113 = bitcast %struct.malloc_chunk* %p.addr.0 to %struct.malloc_tree_chunk*
  %shr540 = lshr i32 %psize.addr.1, 8
  %cmp541 = icmp eq i32 %shr540, 0
  br i1 %cmp541, label %if.end571, label %if.else544

if.else544:                                       ; preds = %if.else536
  %cmp545 = icmp ugt i32 %psize.addr.1, 16777215
  br i1 %cmp545, label %if.end571, label %if.else548

if.else548:                                       ; preds = %if.else544
  %sub = add i32 %shr540, 1048320
  %shr549 = lshr i32 %sub, 16
  %and550 = and i32 %shr549, 8
  %shl551 = shl i32 %shr540, %and550
  %sub552 = add i32 %shl551, 520192
  %shr553 = lshr i32 %sub552, 16
  %and554 = and i32 %shr553, 4
  %add555 = or i32 %and554, %and550
  %shl556 = shl i32 %shl551, %and554
  %sub557 = add i32 %shl556, 245760
  %shr558 = lshr i32 %sub557, 16
  %and559 = and i32 %shr558, 2
  %add560 = or i32 %add555, %and559
  %sub561 = sub i32 14, %add560
  %shl562 = shl i32 %shl556, %and559
  %shr563 = lshr i32 %shl562, 15
  %add564 = add i32 %sub561, %shr563
  %shl565 = shl nsw i32 %add564, 1
  %add566 = add i32 %add564, 7
  %shr567 = lshr i32 %psize.addr.1, %add566
  %and568 = and i32 %shr567, 1
  %add569 = or i32 %and568, %shl565
  br label %if.end571

if.end571:                                        ; preds = %if.else548, %if.else544, %if.else536
  %I539.0 = phi i32 [ %add569, %if.else548 ], [ 0, %if.else536 ], [ 31, %if.else544 ]
  %arrayidx573 = getelementptr inbounds %struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 11, i32 %I539.0
  %index574 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.addr.0, i32 1, i32 3
  %I539.0.c = inttoptr i32 %I539.0 to %struct.malloc_chunk*
  store %struct.malloc_chunk* %I539.0.c, %struct.malloc_chunk** %index574, align 4
  %arrayidx576 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.addr.0, i32 1, i32 1
  store i32 0, i32* %arrayidx576, align 4
  %114 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.addr.0, i32 1, i32 0
  store i32 0, i32* %114, align 4
  %115 = load i32, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 1), align 4
  %shl580 = shl i32 1, %I539.0
  %and581 = and i32 %115, %shl580
  %tobool582 = icmp eq i32 %and581, 0
  br i1 %tobool582, label %if.then583, label %if.else590

if.then583:                                       ; preds = %if.end571
  %or586 = or i32 %115, %shl580
  store i32 %or586, i32* getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 1), align 4
  store %struct.malloc_tree_chunk* %113, %struct.malloc_tree_chunk** %arrayidx573, align 4
  %parent587 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.addr.0, i32 1, i32 2
  %.c = bitcast %struct.malloc_tree_chunk** %arrayidx573 to %struct.malloc_chunk*
  store %struct.malloc_chunk* %.c, %struct.malloc_chunk** %parent587, align 4
  %bk588 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.addr.0, i32 0, i32 3
  store %struct.malloc_chunk* %p.addr.0, %struct.malloc_chunk** %bk588, align 4
  %fd589 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.addr.0, i32 0, i32 2
  store %struct.malloc_chunk* %p.addr.0, %struct.malloc_chunk** %fd589, align 4
  br label %if.end649

if.else590:                                       ; preds = %if.end571
  %116 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx573, align 4
  %cmp592 = icmp eq i32 %I539.0, 31
  br i1 %cmp592, label %cond.end, label %cond.false

cond.false:                                       ; preds = %if.else590
  %shr594 = lshr i32 %I539.0, 1
  %sub597 = sub i32 25, %shr594
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %if.else590
  %cond = phi i32 [ %sub597, %cond.false ], [ 0, %if.else590 ]
  %head59932 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %116, i32 0, i32 1
  %117 = load i32, i32* %head59932, align 4
  %and60033 = and i32 %117, -8
  %cmp60134 = icmp eq i32 %and60033, %psize.addr.1
  br i1 %cmp60134, label %if.else624, label %if.then603.lr.ph

if.then603.lr.ph:                                 ; preds = %cond.end
  %shl598 = shl i32 %psize.addr.1, %cond
  br label %if.then603

for.cond:                                         ; preds = %if.then603
  %shl608 = shl i32 %K591.036, 1
  %head599 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %119, i32 0, i32 1
  %118 = load i32, i32* %head599, align 4
  %and600 = and i32 %118, -8
  %cmp601 = icmp eq i32 %and600, %psize.addr.1
  br i1 %cmp601, label %if.else624, label %if.then603

if.then603:                                       ; preds = %for.cond, %if.then603.lr.ph
  %K591.036 = phi i32 [ %shl598, %if.then603.lr.ph ], [ %shl608, %for.cond ]
  %T.035 = phi %struct.malloc_tree_chunk* [ %116, %if.then603.lr.ph ], [ %119, %for.cond ]
  %shr604 = lshr i32 %K591.036, 31
  %arrayidx607 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %T.035, i32 0, i32 4, i32 %shr604
  %119 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %arrayidx607, align 4
  %cmp609 = icmp eq %struct.malloc_tree_chunk* %119, null
  br i1 %cmp609, label %if.else612, label %for.cond

if.else612:                                       ; preds = %if.then603
  %120 = bitcast %struct.malloc_tree_chunk** %arrayidx607 to i8*
  %121 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp614 = icmp ult i8* %120, %121
  br i1 %cmp614, label %if.else622, label %if.then618

if.then618:                                       ; preds = %if.else612
  store %struct.malloc_tree_chunk* %113, %struct.malloc_tree_chunk** %arrayidx607, align 4
  %parent619 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.addr.0, i32 1, i32 2
  %T.0.c8 = bitcast %struct.malloc_tree_chunk* %T.035 to %struct.malloc_chunk*
  store %struct.malloc_chunk* %T.0.c8, %struct.malloc_chunk** %parent619, align 4
  %bk620 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.addr.0, i32 0, i32 3
  store %struct.malloc_chunk* %p.addr.0, %struct.malloc_chunk** %bk620, align 4
  %fd621 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.addr.0, i32 0, i32 2
  store %struct.malloc_chunk* %p.addr.0, %struct.malloc_chunk** %fd621, align 4
  br label %if.end649

if.else622:                                       ; preds = %if.else612
  tail call void @abort() #6
  unreachable

if.else624:                                       ; preds = %for.cond, %cond.end
  %T.0.lcssa = phi %struct.malloc_tree_chunk* [ %116, %cond.end ], [ %119, %for.cond ]
  %fd626 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %T.0.lcssa, i32 0, i32 2
  %122 = load %struct.malloc_tree_chunk*, %struct.malloc_tree_chunk** %fd626, align 4
  %123 = bitcast %struct.malloc_tree_chunk* %T.0.lcssa to i8*
  %124 = load i8*, i8** getelementptr inbounds (%struct.malloc_state, %struct.malloc_state* @_gm_, i32 0, i32 4), align 4
  %cmp628 = icmp ult i8* %123, %124
  br i1 %cmp628, label %if.else644, label %land.rhs630

land.rhs630:                                      ; preds = %if.else624
  %125 = bitcast %struct.malloc_tree_chunk* %122 to i8*
  %cmp632 = icmp ult i8* %125, %124
  br i1 %cmp632, label %if.else644, label %if.then638

if.then638:                                       ; preds = %land.rhs630
  %bk639 = getelementptr inbounds %struct.malloc_tree_chunk, %struct.malloc_tree_chunk* %122, i32 0, i32 3
  store %struct.malloc_tree_chunk* %113, %struct.malloc_tree_chunk** %bk639, align 4
  store %struct.malloc_tree_chunk* %113, %struct.malloc_tree_chunk** %fd626, align 4
  %fd641 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.addr.0, i32 0, i32 2
  %.c7 = bitcast %struct.malloc_tree_chunk* %122 to %struct.malloc_chunk*
  store %struct.malloc_chunk* %.c7, %struct.malloc_chunk** %fd641, align 4
  %bk642 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.addr.0, i32 0, i32 3
  %T.0.c = bitcast %struct.malloc_tree_chunk* %T.0.lcssa to %struct.malloc_chunk*
  store %struct.malloc_chunk* %T.0.c, %struct.malloc_chunk** %bk642, align 4
  %parent643 = getelementptr inbounds %struct.malloc_chunk, %struct.malloc_chunk* %p.addr.0, i32 1, i32 2
  store %struct.malloc_chunk* null, %struct.malloc_chunk** %parent643, align 4
  br label %if.end649

if.else644:                                       ; preds = %land.rhs630, %if.else624
  tail call void @abort() #6
  unreachable

if.else648:                                       ; preds = %if.end215
  tail call void @abort() #6
  unreachable

if.end649:                                        ; preds = %if.then638, %if.then618, %if.then583, %if.end531, %if.then488, %if.then244, %if.then236, %if.then228, %if.then205, %if.then
  ret void
}

declare i32 @sysconf(i32) #5

declare i32 @time(i32*) #5

define weak i8* @_Znwj(i32 %size) #5 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %cmp = icmp eq i32 %size, 0
  %.size = select i1 %cmp, i32 1, i32 %size
  br label %invoke.cont

invoke.cont:                                      ; preds = %if.then3, %entry
  %call = tail call i8* @malloc(i32 %.size)
  %cmp1 = icmp eq i8* %call, null
  br i1 %cmp1, label %while.body, label %while.end

while.body:                                       ; preds = %invoke.cont
  %0 = atomicrmw add i32* bitcast (void ()** @_ZL13__new_handler to i32*), i32 0 seq_cst
  %tobool = icmp eq i32 %0, 0
  br i1 %tobool, label %if.else, label %if.then3

if.then3:                                         ; preds = %while.body
  %1 = inttoptr i32 %0 to void ()*
  invoke void %1()
          to label %invoke.cont unwind label %lpad.loopexit

lpad.loopexit:                                    ; preds = %if.then3
  %lpad.loopexit4 = landingpad { i8*, i32 }
          filter [1 x i8*] [i8* bitcast ({ i8*, i8*, i8* }* @_ZTISt9bad_alloc to i8*)]
  br label %lpad

lpad.nonloopexit:                                 ; preds = %if.else
  %lpad.nonloopexit5 = landingpad { i8*, i32 }
          filter [1 x i8*] [i8* bitcast ({ i8*, i8*, i8* }* @_ZTISt9bad_alloc to i8*)]
  br label %lpad

lpad:                                             ; preds = %lpad.nonloopexit, %lpad.loopexit
  %lpad.phi = phi { i8*, i32 } [ %lpad.loopexit4, %lpad.loopexit ], [ %lpad.nonloopexit5, %lpad.nonloopexit ]
  %2 = extractvalue { i8*, i32 } %lpad.phi, 1
  %ehspec.fails = icmp slt i32 %2, 0
  br i1 %ehspec.fails, label %ehspec.unexpected, label %eh.resume

ehspec.unexpected:                                ; preds = %lpad
  %3 = extractvalue { i8*, i32 } %lpad.phi, 0
  tail call void @__cxa_call_unexpected(i8* %3) #8
  unreachable

if.else:                                          ; preds = %while.body
  %exception = tail call i8* @__cxa_allocate_exception(i32 4) #1
  %4 = bitcast i8* %exception to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ([5 x i8*], [5 x i8*]* @_ZTVSt9bad_alloc, i32 0, i32 2) to i32 (...)**), i32 (...)*** %4, align 4
  unreachable

while.end:                                        ; preds = %invoke.cont
  ret i8* %call

eh.resume:                                        ; preds = %lpad
  resume { i8*, i32 } %lpad.phi

unreachable:                                      ; preds = %if.else
  unreachable
}

declare i32 @__gxx_personality_v0(...)

declare i8* @__cxa_allocate_exception(i32)

declare void @__cxa_throw(i8*, i8*, i8*)

declare void @__cxa_call_unexpected(i8*)

; Function Attrs: nounwind
define weak noalias i8* @_ZnwjRKSt9nothrow_t(i32 %size, %"struct.std::nothrow_t"*) #0 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %call = invoke noalias i8* @_Znwj(i32 %size)
          to label %try.cont unwind label %lpad

lpad:                                             ; preds = %entry
  %1 = landingpad { i8*, i32 }
          catch i8* null
  %2 = extractvalue { i8*, i32 } %1, 0
  %3 = tail call i8* @__cxa_begin_catch(i8* %2) #1
  invoke void @__cxa_end_catch()
          to label %try.cont unwind label %lpad1

try.cont:                                         ; preds = %lpad, %entry
  %p.0 = phi i8* [ null, %lpad ], [ %call, %entry ]
  ret i8* %p.0

lpad1:                                            ; preds = %lpad
  %4 = landingpad { i8*, i32 }
          filter [0 x i8*] zeroinitializer
  %5 = extractvalue { i8*, i32 } %4, 0
  tail call void @__cxa_call_unexpected(i8* %5) #6
  unreachable
}

declare i8* @__cxa_begin_catch(i8*)

declare void @__cxa_end_catch()

define weak noalias i8* @_Znaj(i32 %size) #5 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %call = invoke noalias i8* @_Znwj(i32 %size)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  ret i8* %call

lpad:                                             ; preds = %entry
  %0 = landingpad { i8*, i32 }
          filter [1 x i8*] [i8* bitcast ({ i8*, i8*, i8* }* @_ZTISt9bad_alloc to i8*)]
  %1 = extractvalue { i8*, i32 } %0, 1
  %ehspec.fails = icmp slt i32 %1, 0
  br i1 %ehspec.fails, label %ehspec.unexpected, label %eh.resume

ehspec.unexpected:                                ; preds = %lpad
  %2 = extractvalue { i8*, i32 } %0, 0
  tail call void @__cxa_call_unexpected(i8* %2) #8
  unreachable

eh.resume:                                        ; preds = %lpad
  resume { i8*, i32 } %0
}

; Function Attrs: nounwind
define weak noalias i8* @_ZnajRKSt9nothrow_t(i32 %size, %"struct.std::nothrow_t"*) #0 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %call = invoke noalias i8* @_Znaj(i32 %size)
          to label %try.cont unwind label %lpad

lpad:                                             ; preds = %entry
  %1 = landingpad { i8*, i32 }
          catch i8* null
  %2 = extractvalue { i8*, i32 } %1, 0
  %3 = tail call i8* @__cxa_begin_catch(i8* %2) #1
  invoke void @__cxa_end_catch()
          to label %try.cont unwind label %lpad1

try.cont:                                         ; preds = %lpad, %entry
  %p.0 = phi i8* [ null, %lpad ], [ %call, %entry ]
  ret i8* %p.0

lpad1:                                            ; preds = %lpad
  %4 = landingpad { i8*, i32 }
          filter [0 x i8*] zeroinitializer
  %5 = extractvalue { i8*, i32 } %4, 0
  tail call void @__cxa_call_unexpected(i8* %5) #6
  unreachable
}

; Function Attrs: nounwind
define weak void @_ZdlPv(i8* %ptr) #0 {
entry:
  %tobool = icmp eq i8* %ptr, null
  br i1 %tobool, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  tail call void @free(i8* %ptr)
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

; Function Attrs: nounwind
define weak void @_ZdlPvRKSt9nothrow_t(i8* %ptr, %"struct.std::nothrow_t"*) #0 {
entry:
  tail call void @_ZdlPv(i8* %ptr) #1
  ret void
}

; Function Attrs: nounwind
define weak void @_ZdaPv(i8* %ptr) #0 {
entry:
  tail call void @_ZdlPv(i8* %ptr) #1
  ret void
}

; Function Attrs: nounwind
define weak void @_ZdaPvRKSt9nothrow_t(i8* %ptr, %"struct.std::nothrow_t"*) #0 {
entry:
  tail call void @_ZdaPv(i8* %ptr) #1
  ret void
}


; Function Attrs: nounwind
declare void @_ZNSt9exceptionD2Ev(%"class.std::exception"*) #0


declare i32 @isspace(i32) #5

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1


; Function Attrs: nounwind readnone
declare double @copysignl(double, double) #3

declare double @fmodl(double, double) #5

; Function Attrs: nounwind readnone
declare double @fabs(double) #3


; Function Attrs: nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture, i8, i64, i32, i1) #1

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1


declare i32 @tolower(i32) #5


attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { nounwind readonly "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { noreturn "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { noreturn nounwind }
attributes #7 = { nounwind readnone }
attributes #8 = { noreturn }

