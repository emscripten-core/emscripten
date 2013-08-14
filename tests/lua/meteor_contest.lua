-- The Computer Language Benchmarks Game
-- http://benchmarksgame.alioth.debian.org/
-- contributed by Mike Pall

-- Generate a decision tree based solver for the meteor puzzle.
local function generatesolver(countinit)
  local pairs, ipairs, format = pairs, ipairs, string.format
  local byte, min, sort = string.byte, math.min, table.sort

  -- Cached position to distance lookup.
  local dist = setmetatable({}, { __index = function(t, xy)
    local x = xy%10; local y = (xy-x)/10
    if (x+y)%2 == 1 then y = y + 1; x = 10 - x end
    local d = xy + 256*x*x + 1024*y*y; t[xy] = d; return d
  end})

  -- Generate an optimized decision tree (within 4% of a hand-tuned tree).
  local dtree = {}
  local rot = { nil, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {} }
  for k=0,9 do
    -- Generate 10 initial pieces from line noise. :-)
    local t = { 60, 62, byte("@BMBIK@KT@GPIKR@IKIKT@GK@KM@BG", k*3+1, k*3+3) }
    rot[1] = t
    for i,xy in ipairs(t) do
      local x = xy%10; local y = (xy-x-60)/10
      -- Add 11 more variations by rotating and flipping.
      for j=2,12 do
	if j == 7 then y = -y else x,y = (x+3*y)/2, (y-x)/2 end
	rot[j][i] = x+10*y
      end
    end
    for i,v in ipairs(rot) do
      -- Exploit symmetry and leave out half of the orientations of one piece.
      -- The selected piece gives the best reduction of the solution space.
      if k ~= 3 or i%2 == 0 then
	-- Normalize to origin, add distance, sort by distance from origin.
	local m = min(v[1], v[2], v[3], v[4], v[5])
	for i=1,5 do v[i] = dist[v[i]-m] end
	sort(v)
	-- Insert into decision tree in distance order.
	local tt = dtree
	for i=2,4 do
	  local xy = v[i]%256
	  local tn = tt[xy]
	  if not tn then tn = {}; tt[xy] = tn end -- Create nodes as needed.
	  tt = tn
	end
	tt[v[5]%256] = k -- Leaves hold the piece numbers.
      end
    end
  end

  -- Lookup table to validate a cell and to find its successor.
  local ok = {}
  for i=0,150 do ok[i] = false end
  for i=99,0,-1 do
    local x = i%10
    if ((i-x)/10+x)%2 == 0 then
      ok[i] = i + (ok[i+1] and 1 or (ok[i+2] and 2 or 3))
    end
  end

  local s = "local u0,u1,u2,u3,u4,u5,u6,u7,u8,u9" -- Piece use flags.
  for p=0,99 do if ok[p] then s = s..",b"..p end end -- Board cells.
  s = s.."\n"..[[
local countinit = ...
local count = countinit
local bmin, bmax, pcs = 9, 0, {}
local smin, smax
local write, reverse = io.write, string.reverse

-- Print min/max boards.
local function printboard(s)
  local flip = true
  for x in string.gmatch(string.gsub(s, ".", "%1 "), "..........") do
    write(x, flip and "\n " or "\n")
    flip = not flip
  end
  write("\n")
end

-- Print result.
local function printresult()
  write(countinit-count, " solutions found\n\n")
  printboard(smin)
  printboard(smax)
end

-- Generate piece lookup array from the order of use.
local function genp()
  local p = pcs
  p[u0] = "0" p[u1] = "1" p[u2] = "2" p[u3] = "3" p[u4] = "4"
  p[u5] = "5" p[u6] = "6" p[u7] = "7" p[u8] = "8" p[u9] = "9"
  return p
end

-- Goal function.
local function f91(k)
  if k ~= 10 then return end
  count = count - 2 -- Need to count the symmetric solution, too.
  repeat
    -- Quick precheck before constructing the string.
    local b0, b99 = b0, b99
    if b0 <= bmin then bmin = b0 elseif b0 >= bmax then bmax = b0
    elseif b99 <= bmin then bmin = b99 elseif b99 >= bmax then bmax = b99
    else break end
    -- Translate the filled board to a string.
    local p = genp()
    local s = p[b0] ]]
  for p=2,99 do if ok[p] then s = s.."..p[b"..p.."]" end end
  s = s..[[
    -- Remember min/max boards, dito for the symmetric board.
    if not smin then smin = s; smax = s
    elseif s < smin then smin = s elseif s > smax then smax = s end
    s = reverse(s)
    if s < smin then smin = s elseif s > smax then smax = s end
  until true
  if count <= 0 then error("") end -- Early abort if max count given.
end
local f93 = f91
]]

  -- Recursively prune the decision tree and convert it to Lua code.
  local function codetree(tree, d, p, pn)
    local found, s = false, ""
    d = d + 1
    for a,t in pairs(tree) do
      local b = p+a
      local pp = ok[b]
      if pp then -- Prune the tree on-the-fly.
	if b ~= pn then pp = pn end -- Find maximum successor function.
	if d == 5 then -- Try to place the last cell of a piece and advance.
	  found = true
	  s = format("%sif not u%d and not b%d then b%d=k u%d=k f%d(k) u%d=N b%d=N end\n",
		     s, t, b, b, t, pp, t, b)
	else -- Try to place an intermediate cell.
	  local st = codetree(t, d, p, pp)
	  if st then -- But only if the subtree is not empty.
	    found = true
	    s = format("%sif not b%d then b%d=k\n%sb%d=N end\n", s, b, b, st, b)
	  end
	end
      end
    end
    return found and s
  end

  -- Embed the decision tree into a function hierarchy.
  for p=88,0,-1 do
    local pn = ok[p]
    if pn then
      s = format("%slocal function f%d(k)\nlocal N if b%d then return f%d(k) end k=k+1 b%d=k\n%sb%d=N end\n",
	    s, p, p, pn, p, codetree(dtree, 1, p, pn), p)
    end
  end

  -- Compile and return solver function and result getter.
  return loadstring(s.."return f0, printresult\n", "solver")(countinit)
end

-- Run the solver protected to get partial results (max count or ctrl-c).
local solver, printresult = generatesolver(10000)
pcall(solver, 0)
printresult()
