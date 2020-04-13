------------------------------------------------------------------------------
-- Lua SciMark (2010-12-20).
--
-- A literal translation of SciMark 2.0a, written in Java and C.
-- Credits go to the original authors Roldan Pozo and Bruce Miller.
-- See: http://math.nist.gov/scimark2/
------------------------------------------------------------------------------
-- Copyright (C) 2006-2010 Mike Pall. All rights reserved.
--
-- Permission is hereby granted, free of charge, to any person obtaining
-- a copy of this software and associated documentation files (the
-- "Software"), to deal in the Software without restriction, including
-- without limitation the rights to use, copy, modify, merge, publish,
-- distribute, sublicense, and/or sell copies of the Software, and to
-- permit persons to whom the Software is furnished to do so, subject to
-- the following conditions:
--
-- The above copyright notice and this permission notice shall be
-- included in all copies or substantial portions of the Software.
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
-- EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
-- MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
-- IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
-- CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
-- TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
-- SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
--
-- [ MIT license: http://www.opensource.org/licenses/mit-license.php ]
------------------------------------------------------------------------------

local SCIMARK_VERSION = "2010-12-10"
local SCIMARK_COPYRIGHT = "Copyright (C) 2006-2010 Mike Pall"

local MIN_TIME = 0.8
local RANDOM_SEED = 101009 -- Must be odd.
local SIZE_SELECT = "small"

local benchmarks = {
  "FFT", "SOR", "MC", "SPARSE", "LU",
  small = {
    FFT		= { 1024 },
    SOR		= { 100 },
    MC		= { },
    SPARSE	= { 1000, 5000 },
    LU		= { 100 },
  },
  large = {
    FFT		= { 1048576 },
    SOR		= { 1000 },
    MC		= { },
    SPARSE	= { 100000, 1000000 },
    LU		= { 1000 },
  },
}

local abs, log, sin, floor = math.abs, math.log, math.sin, math.floor
local pi, clock = math.pi, os.clock
local format = string.format

------------------------------------------------------------------------------
-- Select array type: Lua tables or native (FFI) arrays
------------------------------------------------------------------------------

local darray, iarray

local function array_init()
  if jit and jit.status and jit.status() then
    local ok, ffi = pcall(require, "ffi")
    if ok then
      darray = ffi.typeof("double[?]")
      iarray = ffi.typeof("int[?]")
      return
    end
  end
  function darray(n) return {} end
  iarray = darray
end

------------------------------------------------------------------------------
-- This is a Lagged Fibonacci Pseudo-random Number Generator with
-- j, k, M = 5, 17, 31. Pretty weak, but same as C/Java SciMark.
------------------------------------------------------------------------------

local rand, rand_init

if jit and jit.status and jit.status() then
  -- LJ2 has bit operations and zero-based arrays (internally).
  local bit = require("bit")
  local band, sar = bit.band, bit.arshift
  function rand_init(seed)
    local Rm, Rj, Ri = iarray(17), 16, 11
    for i=0,16 do Rm[i] = 0 end
    for i=16,0,-1 do
      seed = band(seed*9069, 0x7fffffff)
      Rm[i] = seed
    end
    function rand()
      local i = band(Ri+1, sar(Ri-16, 31))
      local j = band(Rj+1, sar(Rj-16, 31))
      Ri, Rj = i, j
      local k = band(Rm[i] - Rm[j], 0x7fffffff)
      Rm[j] = k
      return k * (1.0/2147483647.0)
    end
  end
else
  -- Better for standard Lua with one-based arrays and without bit operations.
  function rand_init(seed)
    local Rm, Rj = {}, 1
    for i=1,17 do Rm[i] = 0 end
    for i=17,1,-1 do
      seed = (seed*9069) % (2^31)
      Rm[i] = seed
    end
    function rand()
      local j, m = Rj, Rm
      local h = j - 5
      if h < 1 then h = h + 17 end
      local k = m[h] - m[j]
      if k < 0 then k = k + 2147483647 end
      m[j] = k
      if j < 17 then Rj = j + 1 else Rj = 1 end
      return k * (1.0/2147483647.0)
    end
  end
end

local function random_vector(n)
  local v = darray(n+1)
  for x=1,n do v[x] = rand() end
  return v
end

local function random_matrix(m, n)
  local a = {}
  for y=1,m do
    local v = darray(n+1)
    a[y] = v
    for x=1,n do v[x] = rand() end
  end
  return a
end

------------------------------------------------------------------------------
-- FFT: Fast Fourier Transform.
------------------------------------------------------------------------------

local function fft_bitreverse(v, n)
  local j = 0
  for i=0,2*n-4,2 do
    if i < j then
      v[i+1], v[i+2], v[j+1], v[j+2] = v[j+1], v[j+2], v[i+1], v[i+2]
    end
    local k = n
    while k <= j do j = j - k; k = k / 2 end
    j = j + k
  end
end

local function fft_transform(v, n, dir)
  if n <= 1 then return end
  fft_bitreverse(v, n)
  local dual = 1
  repeat
    local dual2 = 2*dual
    for i=1,2*n-1,2*dual2 do
      local j = i+dual2
      local ir, ii = v[i], v[i+1]
      local jr, ji = v[j], v[j+1]
      v[j], v[j+1] = ir - jr, ii - ji
      v[i], v[i+1] = ir + jr, ii + ji
    end
    local theta = dir * pi / dual
    local s, s2 = sin(theta), 2.0 * sin(theta * 0.5)^2
    local wr, wi = 1.0, 0.0
    for a=3,dual2-1,2 do
      wr, wi = wr - s*wi - s2*wr, wi + s*wr - s2*wi
      for i=a,a+2*(n-dual2),2*dual2 do
	local j = i+dual2
	local jr, ji = v[j], v[j+1]
	local dr, di = wr*jr - wi*ji, wr*ji + wi*jr
	local ir, ii = v[i], v[i+1]
	v[j], v[j+1] = ir - dr, ii - di
	v[i], v[i+1] = ir + dr, ii + di
      end
    end
    dual = dual2
  until dual >= n
end

function benchmarks.FFT(n)
  local l2n = log(n)/log(2)
  if l2n % 1 ~= 0 then
    io.stderr:write("Error: FFT data length is not a power of 2\n")
    os.exit(1)
  end
  local v = random_vector(n*2)
  return function(cycles)
    local norm = 1.0 / n
    for p=1,cycles do
      fft_transform(v, n, -1)
      fft_transform(v, n, 1)
      for i=1,n*2 do v[i] = v[i] * norm end
    end
    return ((5*n-2)*l2n + 2*(n+1)) * cycles
  end
end

------------------------------------------------------------------------------
-- SOR: Jacobi Successive Over-Relaxation.
------------------------------------------------------------------------------

local function sor_run(mat, m, n, cycles, omega)
  local om4, om1 = omega*0.25, 1.0-omega
  m = m - 1
  n = n - 1
  for i=1,cycles do
    for y=2,m do
      local v, vp, vn = mat[y], mat[y-1], mat[y+1]
      for x=2,n do
	v[x] = om4*((vp[x]+vn[x])+(v[x-1]+v[x+1])) + om1*v[x]
      end
    end
  end
end

function benchmarks.SOR(n)
  local mat = random_matrix(n, n)
  return function(cycles)
    sor_run(mat, n, n, cycles, 1.25)
    return (n-1)*(n-1)*cycles*6
  end
end

------------------------------------------------------------------------------
-- MC: Monte Carlo Integration.
------------------------------------------------------------------------------

local function mc_integrate(cycles)
  local under_curve = 0
  local rand = rand
  for i=1,cycles do
    local x = rand()
    local y = rand()
    if x*x + y*y <= 1.0 then under_curve = under_curve + 1 end
  end
  return (under_curve/cycles) * 4
end

function benchmarks.MC()
  return function(cycles)
    local res = mc_integrate(cycles)
    assert(math.sqrt(cycles)*math.abs(res-math.pi) < 5.0, "bad MC result")
    return cycles * 4 -- Way off, but same as SciMark in C/Java.
  end
end

------------------------------------------------------------------------------
-- Sparse Matrix Multiplication.
------------------------------------------------------------------------------

local function sparse_mult(n, cycles, vy, val, row, col, vx)
  for p=1,cycles do
    for r=1,n do
      local sum = 0
      for i=row[r],row[r+1]-1 do sum = sum + vx[col[i]] * val[i] end
      vy[r] = sum
    end
  end
end

function benchmarks.SPARSE(n, nz)
  local nr = floor(nz/n)
  local anz = nr*n
  local vx = random_vector(n)
  local val = random_vector(anz)
  local vy, col, row = darray(n+1), iarray(nz+1), iarray(n+2)
  row[1] = 1
  for r=1,n do
    local step = floor(r/nr)
    if step < 1 then step = 1 end
    local rr = row[r]
    row[r+1] = rr+nr
    for i=0,nr-1 do col[rr+i] = 1+i*step end
  end
  return function(cycles)
    sparse_mult(n, cycles, vy, val, row, col, vx)
    return anz*cycles*2
  end
end

------------------------------------------------------------------------------
-- LU: Dense Matrix Factorization.
------------------------------------------------------------------------------

local function lu_factor(a, pivot, m, n)
  local min_m_n = m < n and m or n
  for j=1,min_m_n do
    local jp, t = j, abs(a[j][j])
    for i=j+1,m do
      local ab = abs(a[i][j])
      if ab > t then
	jp = i
	t = ab
      end
    end
    pivot[j] = jp
    if a[jp][j] == 0 then error("zero pivot") end
    if jp ~= j then a[j], a[jp] = a[jp], a[j] end
    if j < m then
      local recp = 1.0 / a[j][j]
      for k=j+1,m do
	local v = a[k]
	v[j] = v[j] * recp
      end
    end
    if j < min_m_n then
      for i=j+1,m do
	local vi, vj = a[i], a[j]
	local eij = vi[j]
	for k=j+1,n do vi[k] = vi[k] - eij * vj[k] end
      end
    end
  end
end

local function matrix_alloc(m, n)
  local a = {}
  for y=1,m do a[y] = darray(n+1) end
  return a
end

local function matrix_copy(dst, src, m, n)
  for y=1,m do
    local vd, vs = dst[y], src[y]
    for x=1,n do vd[x] = vs[x] end
  end
end

function benchmarks.LU(n)
  local mat = random_matrix(n, n)
  local tmp = matrix_alloc(n, n)
  local pivot = iarray(n+1)
  return function(cycles)
    for i=1,cycles do
      matrix_copy(tmp, mat, n, n)
      lu_factor(tmp, pivot, n, n)
    end
    return 2.0/3.0*n*n*n*cycles
  end
end

------------------------------------------------------------------------------
-- Main program.
------------------------------------------------------------------------------

local function printf(...)
  io.write(format(...))
end

local function fmtparams(p1, p2)
  if p2 then return format("[%d, %d]", p1, p2)
  elseif p1 then return format("[%d]", p1) end
  return ""
end

local function measure(min_time, name, ...)
  array_init()
  rand_init(RANDOM_SEED)
  local run = benchmarks[name](...)
  local cycles = 1
  repeat
    local tm = clock()
    local flops = run(cycles, ...)
    tm = clock() - tm
    if tm >= min_time then
      local res = flops / tm * 1.0e-6
      local p1, p2 = ...
      printf("%-7s %8.2f  %s\n", name, res, fmtparams(...))
      return res
    end
    cycles = cycles * 2
  until false
end

--printf("Lua SciMark %s based on SciMark 2.0a. %s.\n\n",
--       SCIMARK_VERSION, SCIMARK_COPYRIGHT)

--while arg and arg[1] do
--  local a = table.remove(arg, 1)
--  if a == "-noffi" then
--    package.preload.ffi = nil
--  elseif a == "-small" then
--    SIZE_SELECT = "small"
--  elseif a == "-large" then
--    SIZE_SELECT = "large"
--  elseif benchmarks[a] then
--    local p = benchmarks[SIZE_SELECT][a]
--    measure(MIN_TIME, a, tonumber(arg[1]) or p[1], tonumber(arg[2]) or p[2])
--    return
--  else
--    printf("Usage: scimark [-noffi] [-small|-large] [BENCH params...]\n\n")
--    printf("BENCH   -small         -large\n")
--    printf("---------------------------------------\n")
--    for _,name in ipairs(benchmarks) do
--      printf("%-7s %-13s %s\n", name,
--	     fmtparams(unpack(benchmarks.small[name])),
--	     fmtparams(unpack(benchmarks.large[name])))
--    end
--    printf("\n")
--    os.exit(1)
--  end
--end

local params = benchmarks[SIZE_SELECT]
local sum = 0
for _,name in ipairs(benchmarks) do
  sum = sum + measure(MIN_TIME, name, unpack(params[name]))
end
printf("\nSciMark %8.2f  [%s problem sizes]\n", sum / #benchmarks, SIZE_SELECT)
io.flush()


