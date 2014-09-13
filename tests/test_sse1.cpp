// This file tests full SSE1 compatibility.
// Compile with something like 'clang -msse test_sse1.cpp -o a.out' to build natively, and run './a.out'.
// To test with Emscripten, compile with 'emcc test_sse1.cpp -o a.js' (or -o a.html) and run in SpiderMonkey/node.js/browser.

#include <xmmintrin.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <inttypes.h>

// Throughout the file, the notation { a,b,c,d } refers to in-memory array notation, i.e. a is in the lowest memory address, and d is in the highest.
// The notation [a,b,c,d] refers to SIMD channels, where d is the lowest channel 0 (the scalar channel), and a is the highest channel 3.

// Tests if m == [v3, v2, v1, v0]
bool __attribute__((noinline)) aeq(__m128 m, float v3, float v2, float v1, float v0, bool abortOnFailure = true)
{
	float val[4];
	_mm_storeu_ps(val, m);
	bool eq = fabs(val[0]-v0) < 1e-5f && fabs(val[1]-v1) < 1e-5f && fabs(val[2]-v2) < 1e-5f && fabs(val[3]-v3) < 1e-5f;
	if (!eq && abortOnFailure)
	{
		fprintf(stderr, "[%g, %g, %g, %g] != [%g, %g, %g, %g]\n", val[3], val[2], val[1], val[0], v3, v2, v1, v0);
		assert(false);
	}
	return eq;
}

// Tests if m == [v3, v2, v1, v0] but where vx are integers.
bool __attribute__((noinline)) aeqi(__m128 m, uint32_t v3, uint32_t v2, uint32_t v1, uint32_t v0, bool abortOnFailure = true)
{
	uint32_t val[4];
	_mm_storeu_ps((float*)val, m);
	bool eq = val[0] == v0 && val[1] == v1 && val[2] == v2 && val[3] == v3;
	if (!eq && abortOnFailure)
	{
		fprintf(stderr, "[0x%08x, 0x%08x, 0x%08x, 0x%08x] != [0x%08x, 0x%08x, 0x%08x, 0x%08x]\n", val[3], val[2], val[1], val[0], v3, v2, v1, v0);
		assert(false);
	}
	return eq;
}

// Recasts floating point representation of f to an integer.
uint32_t fcastu(float f) { return *(uint32_t*)&f; }
float ucastf(uint32_t t) { return *(float*)&t; }

// Data used in test. Store them global and access via a getter to confuse optimizer to not "solve" the whole test suite at compile-time,
// so that the operation will actually be performed at runtime, and not at compile-time. (Testing the capacity of the compiler to perform
// SIMD ops at compile-time would be interesting as well, but that's for another test)
float arr_[9];
float *uarr_ = arr_+1; // Create an unaligned address to test unaligned loads.
float arr2_[9]; // A target for testing aligned stores.
float *uarr2_ = arr2_+1; // An unaligned address for testing unaligned stores.
__m128 a_ = _mm_set_ps(8.f, 6.f, 4.f, 2.f);
__m128 b_ = _mm_set_ps(1.f, 2.f, 3.f, 4.f);
__m128 c_ = _mm_set_ps(1.5f, 2.5f, 3.5f, 4.5f);
__m128 d_ = _mm_set_ps(8.5f, 6.5f, 4.5f, 2.5f);
__m128 nan1_ = _mm_set_ps(NAN, 0,  0, NAN); // All combinations pairwise with nan1 & nan2.
__m128 nan2_ = _mm_set_ps(NAN, NAN, 0, 0);

bool always_true() { return time(NULL) != 0; } // This function always returns true, but the compiler should not know this.

// Accessors to the test data in a way that the compiler can't optimize at compile-time.
__attribute__((noinline)) float *get_arr()
{
	float *a = (float*)(((uintptr_t)arr_ + 0xF) & ~0xF);
	a[0] = 1.f;
	a[1] = 2.f;
	a[2] = 3.f;
	a[3] = 4.f;
	a[4] = 5.f;

	return always_true() ? a : 0;
}

__attribute__((noinline)) float *get_uarr() { return always_true() ? get_arr()+1 : 0; }
__attribute__((noinline)) float *get_arr2() { return always_true() ? (float*)(((uintptr_t)arr2_ + 0xF) & ~0xF) : 0; }
__attribute__((noinline)) float *get_uarr2() { return always_true() ? get_arr2()+1 : 0; }
__attribute__((noinline)) __m128 get_a() { return always_true() ? a_ : __m128(); }
__attribute__((noinline)) __m128 get_b() { return always_true() ? b_ : __m128(); }
__attribute__((noinline)) __m128 get_c() { return always_true() ? c_ : __m128(); }
__attribute__((noinline)) __m128 get_d() { return always_true() ? d_ : __m128(); }
__attribute__((noinline)) __m128 get_i1() { return always_true() ? _mm_set_ps(ucastf(0x87654321), ucastf(0x0FEDCBA9), ucastf(0x87654321), ucastf(0xFFEDCBA9)) : __m128(); }
__attribute__((noinline)) __m128 get_i2() { return always_true() ? _mm_set_ps(ucastf(0xBBAA9988), ucastf(0xFFEEDDCC), ucastf(0xF02468BD), ucastf(0x13579ACE)) : __m128(); }
__attribute__((noinline)) __m128 get_nan1() { return always_true() ? nan1_ : __m128(); }
__attribute__((noinline)) __m128 get_nan2() { return always_true() ? nan2_ : __m128(); }

int main()
{
	float *arr = get_arr(); // [4, 3, 2, 1]
	float *uarr = get_uarr(); // [5, 4, 3, 2]
	float *arr2 = get_arr2(); // [4, 3, 2, 1]
	float *uarr2 = get_uarr2(); // [5, 4, 3, 2]
	__m128 a = get_a(); // [8, 6, 4, 2]
	__m128 b = get_b(); // [1, 2, 3, 4]

	// Check that test data is like expected.
	assert(((uintptr_t)arr & 0xF) == 0); // arr must be aligned by 16.
	assert(((uintptr_t)uarr & 0xF) != 0); // uarr must be unaligned.
	assert(((uintptr_t)arr2 & 0xF) == 0); // arr must be aligned by 16.
	assert(((uintptr_t)uarr2 & 0xF) != 0); // uarr must be unaligned.

	// Test that aeq itself works and does not trivially return true on everything.
	assert(aeq(_mm_load_ps(arr), 4.f, 3.f, 2.f, 0.f, false) == false);

	// SSE1 Load instructions:	
	aeq(_mm_load_ps(arr), 4.f, 3.f, 2.f, 1.f); // 4-wide load from aligned address.
	aeq(_mm_load_ps1(uarr), 2.f, 2.f, 2.f, 2.f); // Load scalar from unaligned address and populate 4-wide.
	aeq(_mm_load_ss(uarr), 0.f, 0.f, 0.f, 2.f); // Load scalar from unaligned address to lowest, and zero all highest.
	aeq(_mm_load1_ps(uarr), 2.f, 2.f, 2.f, 2.f); // _mm_load1_ps == _mm_load_ps1
	aeq(_mm_loadh_pi(a, (__m64*)uarr), 3.f, 2.f, 4.f, 2.f); // Load two highest addresses, preserve two lowest.
	aeq(_mm_loadl_pi(a, (__m64*)uarr), 8.f, 6.f, 3.f, 2.f); // Load two lowest addresses, preserve two highest.
	aeq(_mm_loadr_ps(arr), 1.f, 2.f, 3.f, 4.f); // 4-wide load from an aligned address, but reverse order.
	aeq(_mm_loadu_ps(uarr), 5.f, 4.f, 3.f, 2.f); // 4-wide load from an unaligned address.

	// SSE1 Set instructions:
	aeq(_mm_set_ps(uarr[3], 2.f, 3.f, 4.f), 5.f, 2.f, 3.f, 4.f); // 4-wide set by specifying four immediate or memory operands.
	aeq(_mm_set_ps1(uarr[3]), 5.f, 5.f, 5.f, 5.f); // 4-wide set by specifying one scalar that is expanded.
	aeq(_mm_set_ss(uarr[3]), 0.f, 0.f, 0.f, 5.f); // Set scalar at lowest index, zero all higher.
	aeq(_mm_set1_ps(uarr[3]), 5.f, 5.f, 5.f, 5.f); // _mm_set1_ps == _mm_set_ps1
	aeq(_mm_setr_ps(uarr[3], 2.f, 3.f, 4.f), 4.f, 3.f, 2.f, 5.f); // 4-wide set by specifying four immediate or memory operands, but reverse order.
	aeq(_mm_setzero_ps(), 0.f, 0.f, 0.f, 0.f); // Returns a new zero register.

	// SSE1 Move instructions:
	aeq(_mm_move_ss(a, b), 8.f, 6.f, 4.f, 4.f); // Copy three highest elements from a, and lowest from b.
	aeq(_mm_movehl_ps(a, b), 8.f, 6.f, 1.f, 2.f); // Copy two highest elements from a, and take two highest from b and place them to the two lowest in output.
	aeq(_mm_movelh_ps(a, b), 3.f, 4.f, 4.f, 2.f); // Copy two lowest elements from a, and take two lowest from b and place them to the two highest in output.

	// SSE1 Store instructions:
	// TODO: _mm_maskmove_si64
	// TODO: _m_maskmovq
	_mm_store_ps(arr2, a); aeq(_mm_load_ps(arr2), 8.f, 6.f, 4.f, 2.f); // _mm_store_ps: 4-wide store to aligned memory address.
	_mm_store_ps1(arr2, a); aeq(_mm_load_ps(arr2), 2.f, 2.f, 2.f, 2.f); // _mm_store_ps1: Store lowest scalar to aligned address, duplicating the element 4 times. 
	_mm_storeu_ps(uarr2, _mm_set1_ps(100.f)); _mm_store_ss(uarr2, b); aeq(_mm_loadu_ps(uarr2), 100.f, 100.f, 100.f, 4.f); // _mm_store_ss: Store lowest scalar to unaligned address. Don't adjust higher addresses in memory.
	_mm_store_ps(arr2, _mm_set1_ps(100.f)); _mm_store1_ps(arr2, a); aeq(_mm_load_ps(arr2), 2.f, 2.f, 2.f, 2.f); // _mm_store1_ps == _mm_store_ps1
	_mm_storeu_ps(uarr2, _mm_set1_ps(100.f)); _mm_storeh_pi((__m64*)uarr2, a); aeq(_mm_loadu_ps(uarr2), 100.f, 100.f, 8.f, 6.f); // _mm_storeh_pi: Store two highest elements to memory.
	_mm_storeu_ps(uarr2, _mm_set1_ps(100.f)); _mm_storel_pi((__m64*)uarr2, a); aeq(_mm_loadu_ps(uarr2), 100.f, 100.f, 4.f, 2.f); // _mm_storel_pi: Store two lowest elements to memory.
	_mm_storer_ps(arr2, a); aeq(_mm_load_ps(arr2), 2.f, 4.f, 6.f, 8.f); // _mm_storeu_ps: 4-wide store to aligned memory address, but reverse the elements on output.
	_mm_storeu_ps(uarr2, a); aeq(_mm_loadu_ps(uarr2), 8.f, 6.f, 4.f, 2.f); // _mm_storeu_ps: 4-wide store to unaligned memory address.
	// TODO: _mm_stream_pi
	_mm_store_ps(arr2, _mm_set1_ps(100.f)); _mm_stream_ps(arr2, a); aeq(_mm_load_ps(arr2), 8.f, 6.f, 4.f, 2.f); // _mm_stream_ps: 4-wide store, but with a non-temporal memory cache hint.

	// SSE1 Arithmetic instructions:
	aeq(_mm_add_ps(a, b), 9.f, 8.f, 7.f, 6.f); // 4-wide add.
	aeq(_mm_add_ss(a, b), 8.f, 6.f, 4.f, 6.f); // Add lowest element, preserve three highest unchanged from a.
	aeq(_mm_div_ps(a, _mm_set_ps(2.f, 3.f, 8.f, 2.f)), 4.f, 2.f, 0.5f, 1.f); // 4-wide div.
	aeq(_mm_div_ss(a, _mm_set_ps(2.f, 3.f, 8.f, 8.f)), 8.f, 6.f, 4.f, 0.25f); // Div lowest element, preserve three highest unchanged from a.
	aeq(_mm_mul_ps(a, b), 8.f, 12.f, 12.f, 8.f); // 4-wide mul.
	aeq(_mm_mul_ss(a, b), 8.f, 6.f, 4.f, 8.f); // Mul lowest element, preserve three highest unchanged from a.
	// TODO: _mm_mulhi_pu16
	// TODO: _m_pmulhuw
	// TODO: _m_psadbw
	// TODO: _mm_sad_pu8
	aeq(_mm_sub_ps(a, b), 7.f, 4.f, 1.f, -2.f); // 4-wide sub.
	aeq(_mm_sub_ss(a, b), 8.f, 6.f, 4.f, -2.f); // Sub lowest element, preserve three highest unchanged from a.

	// SSE1 Elementary Math functions:
	aeq(_mm_rcp_ps(a), 0.124969f, 0.166626f, 0.249939f, 0.499878f); // Compute 4-wide 1/x.
	aeq(_mm_rcp_ss(a), 8.f, 6.f, 4.f, 0.499878f); // Compute 1/x of lowest element, pass higher elements unchanged.
	aeq(_mm_rsqrt_ps(a), 0.353455f, 0.408203f, 0.499878f, 0.706909f); // Compute 4-wide 1/sqrt(x).
	aeq(_mm_rsqrt_ss(a), 8.f, 6.f, 4.f, 0.706909f); // Compute 1/sqrt(x) of lowest element, pass higher elements unchanged.
	aeq(_mm_sqrt_ps(a), 2.82843f, 2.44949f, 2.f, 1.41421f); // Compute 4-wide sqrt(x).
	aeq(_mm_sqrt_ss(a), 8.f, 6.f, 4.f, 1.41421f); // Compute sqrt(x) of lowest element, pass higher elements unchanged.

	__m128 i1 = get_i1();
	__m128 i2 = get_i2();

	// SSE1 Logical instructions:
	aeqi(_mm_and_ps(i1, i2), 0x83200100, 0x0fecc988, 0x80244021, 0x13458a88); // 4-wide binary AND
	aeqi(_mm_andnot_ps(i1, i2), 0x388a9888, 0xf0021444, 0x7000289c, 0x00121046); // 4-wide binary (!i1) & i2
	aeqi(_mm_or_ps(i1, i2), 0xbfefdba9, 0xffefdfed, 0xf7656bbd, 0xffffdbef); // 4-wide binary OR
	aeqi(_mm_xor_ps(i1, i2), 0x3ccfdaa9, 0xf0031665, 0x77412b9c, 0xecba5167); // 4-wide binary XOR

	// SSE1 Compare instructions:
	// a = [8, 6, 4, 2], b = [1, 2, 3, 4]
	aeqi(_mm_cmpeq_ps(a, _mm_set_ps(8.f, 0.f, 4.f, 0.f)), 0xFFFFFFFF, 0, 0xFFFFFFFF, 0); // 4-wide cmp ==
	aeqi(_mm_cmpeq_ss(a, _mm_set_ps(8.f, 0.f, 4.f, 2.f)), fcastu(8.f), fcastu(6.f), fcastu(4.f), 0xFFFFFFFF); // scalar cmp ==, pass three highest unchanged.
	aeqi(_mm_cmpge_ps(a, _mm_set_ps(8.f, 7.f, 3.f, 5.f)), 0xFFFFFFFF, 0, 0xFFFFFFFF, 0); // 4-wide cmp >=
	aeqi(_mm_cmpge_ss(a, _mm_set_ps(8.f, 7.f, 3.f, 0.f)), fcastu(8.f), fcastu(6.f), fcastu(4.f), 0xFFFFFFFF); // scalar cmp >=, pass three highest unchanged.
	aeqi(_mm_cmpgt_ps(a, _mm_set_ps(8.f, 7.f, 3.f, 5.f)), 0, 0, 0xFFFFFFFF, 0); // 4-wide cmp >
	aeqi(_mm_cmpgt_ss(a, _mm_set_ps(8.f, 7.f, 3.f, 2.f)), fcastu(8.f), fcastu(6.f), fcastu(4.f), 0); // scalar cmp >, pass three highest unchanged.
	aeqi(_mm_cmple_ps(a, _mm_set_ps(8.f, 7.f, 3.f, 5.f)), 0xFFFFFFFF, 0xFFFFFFFF, 0, 0xFFFFFFFF); // 4-wide cmp <=
	aeqi(_mm_cmple_ss(a, _mm_set_ps(8.f, 7.f, 3.f, 0.f)), fcastu(8.f), fcastu(6.f), fcastu(4.f), 0); // scalar cmp <=, pass three highest unchanged.
	aeqi(_mm_cmplt_ps(a, _mm_set_ps(8.f, 7.f, 3.f, 5.f)), 0, 0xFFFFFFFF, 0, 0xFFFFFFFF); // 4-wide cmp <
	aeqi(_mm_cmplt_ss(a, _mm_set_ps(8.f, 7.f, 3.f, 2.f)), fcastu(8.f), fcastu(6.f), fcastu(4.f), 0); // scalar cmp <, pass three highest unchanged.

	aeqi(_mm_cmpneq_ps(a, _mm_set_ps(8.f, 0.f, 4.f, 0.f)), 0, 0xFFFFFFFF, 0, 0xFFFFFFFF); // 4-wide cmp !=
	aeqi(_mm_cmpneq_ss(a, _mm_set_ps(8.f, 0.f, 4.f, 2.f)), fcastu(8.f), fcastu(6.f), fcastu(4.f), 0); // scalar cmp !=, pass three highest unchanged.
	aeqi(_mm_cmpnge_ps(a, _mm_set_ps(8.f, 7.f, 3.f, 5.f)), 0, 0xFFFFFFFF, 0, 0xFFFFFFFF); // 4-wide cmp not >=
	aeqi(_mm_cmpnge_ss(a, _mm_set_ps(8.f, 7.f, 3.f, 0.f)), fcastu(8.f), fcastu(6.f), fcastu(4.f), 0); // scalar cmp not >=, pass three highest unchanged.
	aeqi(_mm_cmpngt_ps(a, _mm_set_ps(8.f, 7.f, 3.f, 5.f)), 0xFFFFFFFF, 0xFFFFFFFF, 0, 0xFFFFFFFF); // 4-wide cmp not >
	aeqi(_mm_cmpngt_ss(a, _mm_set_ps(8.f, 7.f, 3.f, 2.f)), fcastu(8.f), fcastu(6.f), fcastu(4.f), 0xFFFFFFFF); // scalar cmp not >, pass three highest unchanged.
	aeqi(_mm_cmpnle_ps(a, _mm_set_ps(8.f, 7.f, 3.f, 5.f)), 0, 0, 0xFFFFFFFF, 0); // 4-wide cmp not <=
	aeqi(_mm_cmpnle_ss(a, _mm_set_ps(8.f, 7.f, 3.f, 0.f)), fcastu(8.f), fcastu(6.f), fcastu(4.f), 0xFFFFFFFF); // scalar cmp not <=, pass three highest unchanged.
	aeqi(_mm_cmpnlt_ps(a, _mm_set_ps(8.f, 7.f, 3.f, 5.f)), 0xFFFFFFFF, 0, 0xFFFFFFFF, 0); // 4-wide cmp not <
	aeqi(_mm_cmpnlt_ss(a, _mm_set_ps(8.f, 7.f, 3.f, 2.f)), fcastu(8.f), fcastu(6.f), fcastu(4.f), 0xFFFFFFFF); // scalar cmp not <, pass three highest unchanged.

	__m128 nan1 = get_nan1(); // [NAN, 0, 0, NAN]
	__m128 nan2 = get_nan2(); // [NAN, NAN, 0, 0]
	aeqi(_mm_cmpord_ps(nan1, nan2), 0, 0, 0xFFFFFFFF, 0); // 4-wide test if both operands are not nan.
	aeqi(_mm_cmpord_ss(nan1, nan2), fcastu(NAN), 0, 0, 0); // scalar test if both operands are not nan, pass three highest unchanged.
	// Intel Intrinsics Guide documentation is wrong on _mm_cmpunord_ps and _mm_cmpunord_ss. MSDN is right: http://msdn.microsoft.com/en-us/library/khy6fk1t(v=vs.90).aspx
	aeqi(_mm_cmpunord_ps(nan1, nan2), 0xFFFFFFFF, 0xFFFFFFFF, 0, 0xFFFFFFFF); // 4-wide test if one of the operands is nan.
	aeqi(_mm_cmpunord_ss(nan1, nan2), fcastu(NAN), 0, 0, 0xFFFFFFFF); // scalar test if one of the operands is nan, pass three highest unchanged.

	assert(_mm_comieq_ss(a, b) == 0); assert(_mm_comieq_ss(a, a) == 1); // Scalar cmp == of lowest element, return int.
	assert(_mm_comige_ss(a, b) == 0); assert(_mm_comige_ss(a, a) == 1); // Scalar cmp >= of lowest element, return int.
	assert(_mm_comigt_ss(b, a) == 1); assert(_mm_comigt_ss(a, a) == 0); // Scalar cmp > of lowest element, return int.
	assert(_mm_comile_ss(b, a) == 0); assert(_mm_comile_ss(a, a) == 1); // Scalar cmp <= of lowest element, return int.
	assert(_mm_comilt_ss(a, b) == 1); assert(_mm_comilt_ss(a, a) == 0); // Scalar cmp < of lowest element, return int.
	assert(_mm_comineq_ss(a, b) == 1); assert(_mm_comineq_ss(a, a) == 0); // Scalar cmp != of lowest element, return int.

	// The ucomi versions are identical to comi, except that ucomi signal a FP exception only if one of the input operands is a SNaN, whereas the comi versions signal a FP
	// exception when one of the input operands is either a QNaN or a SNaN.
	assert(_mm_ucomieq_ss(a, b) == 0); assert(_mm_ucomieq_ss(a, a) == 1); assert(_mm_ucomieq_ss(a, nan1) == 1);
	assert(_mm_ucomige_ss(a, b) == 0); assert(_mm_ucomige_ss(a, a) == 1); assert(_mm_ucomige_ss(a, nan1) == 0);
	assert(_mm_ucomigt_ss(b, a) == 1); assert(_mm_ucomigt_ss(a, a) == 0); assert(_mm_ucomigt_ss(a, nan1) == 0);
	assert(_mm_ucomile_ss(b, a) == 0); assert(_mm_ucomile_ss(a, a) == 1); assert(_mm_ucomile_ss(a, nan1) == 1);
	assert(_mm_ucomilt_ss(a, b) == 1); assert(_mm_ucomilt_ss(a, a) == 0); assert(_mm_ucomilt_ss(a, nan1) == 1);
	assert(_mm_ucomineq_ss(a, b) == 1); assert(_mm_ucomineq_ss(a, a) == 0); assert(_mm_ucomineq_ss(a, nan1) == 0);

	// SSE1 Convert instructions:
	// TODO: _mm_cvt_pi2ps
	// TODO: _mm_cvt_ps2pi
	// TODO: _mm_cvt_si2ss
	// TODO: _mm_cvt_ss2si
	// TODO: _mm_cvtpi16_ps
	// TODO: _mm_cvtpi32_ps
	// TODO: _mm_cvtpi32x2_ps
	// TODO: _mm_cvtpi8_ps
	// TODO: _mm_cvtps_pi16
	// TODO: _mm_cvtps_pi32
	// TODO: _mm_cvtps_pi8
	// TODO: _mm_cvtpu16_ps
	// TODO: _mm_cvtpu8_ps
	// TODO: _mm_cvtsi32_ss
	// TODO: _mm_cvtsi64_ss
	// TODO: _mm_cvtss_f32
	// TODO: _mm_cvtss_si32
	// TODO: _mm_cvtss_si64
	// TODO: _mm_cvtt_ps2pi
	// TODO: _mm_cvtt_ss2si
	// TODO: _mm_cvttps_pi32
	// TODO: _mm_cvttss_si32
	// TODO: _mm_cvttss_si64

	// SSE1 General support:
	unsigned int mask = _MM_GET_EXCEPTION_MASK();
	_MM_SET_EXCEPTION_MASK(mask);
	unsigned int flushZeroMode = _MM_GET_FLUSH_ZERO_MODE();
	_MM_SET_FLUSH_ZERO_MODE(flushZeroMode);
	unsigned int roundingMode = _MM_GET_ROUNDING_MODE();
	_MM_SET_ROUNDING_MODE(roundingMode);
	unsigned int csr = _mm_getcsr();
	_mm_setcsr(csr);
	unsigned char dummyData[4096];
	_mm_prefetch(dummyData, _MM_HINT_T0);
	_mm_prefetch(dummyData, _MM_HINT_T1);
	_mm_prefetch(dummyData, _MM_HINT_T2);
	_mm_prefetch(dummyData, _MM_HINT_NTA);
	_mm_sfence();

	// SSE1 Misc instructions:
	// TODO: _mm_movemask_pi8
	assert(_mm_movemask_ps(_mm_set_ps(-1.f, 0.f, 1.f, NAN)) == 8); assert(_mm_movemask_ps(_mm_set_ps(-INFINITY, -0.f, INFINITY, -INFINITY)) == 13); // Return int with four lowest bits set depending on the highest bits of the input channels.
	// TODO: _m_pmovmskb
	// TODO: _m_psadbw
	// TODO: _mm_sad_pu8

	// SSE1 Probability/Statistics instructions:
	// TODO: _mm_avg_pu16
	// TODO: _mm_avg_pu8
	// TODO: _m_pavgb
	// TODO: _m_pavgw

	// SSE1 Special Math instructions:
	// TODO: _mm_max_pi16
	// a = [8, 6, 4, 2], b = [1, 2, 3, 4]
	aeq(_mm_max_ps(a, b), 8.f, 6.f, 4.f, 4.f); // 4-wide max.
	// TODO: _mm_max_pu8
	aeq(_mm_max_ss(a, _mm_set1_ps(100.f)), 8.f, 6.f, 4.f, 100.f); // Scalar max, pass three highest unchanged.
	// TODO: _mm_min_pi16
	aeq(_mm_min_ps(a, b), 1.f, 2.f, 3.f, 2.f); // 4-wide min.
	// TODO: _mm_min_pu8
	aeq(_mm_min_ss(a, _mm_set1_ps(-100.f)), 8.f, 6.f, 4.f, -100.f); // Scalar min, pass three highest unchanged.
	// TODO: _m_pmaxsw
	// TODO: _m_pmaxub
	// TODO: _m_pminsw
	// TODO: _m_pminub

	// SSE1 Swizzle instructions:
	// TODO: _mm_extract_pi16
	// TODO: _mm_insert_pi16
	// TODO: _m_pextrw
	// TODO: _m_pinsrw
	// TODO: _m_pshufw
	// TODO: _mm_shuffle_pi16
	aeq(_mm_shuffle_ps(a, b, _MM_SHUFFLE(1, 0, 3, 2)), 3.f, 4.f, 8.f, 6.f);
	aeq(_mm_unpackhi_ps(a, b), 1.f , 8.f, 2.f, 6.f);
	aeq(_mm_unpacklo_ps(a, b), 3.f , 4.f, 4.f, 2.f);

	// Transposing a matrix via the xmmintrin.h-provided intrinsic.
	__m128 c0 = a; // [8, 6, 4, 2]
	__m128 c1 = b; // [1, 2, 3, 4]
	__m128 c2 = get_c(); // [1.5, 2.5, 3.5, 4.5]
	__m128 c3 = get_d(); // [8.5, 6.5, 4.5, 2.5]
	_MM_TRANSPOSE4_PS(c0, c1, c2, c3);
	aeq(c0, 2.5f, 4.5f, 4.f, 2.f);
	aeq(c1, 4.5f, 3.5f, 3.f, 4.f);
	aeq(c2, 6.5f, 2.5f, 2.f, 6.f);
	aeq(c3, 8.5f, 1.5f, 1.f, 8.f);

	// All done!
	printf("Success!\n");
}
