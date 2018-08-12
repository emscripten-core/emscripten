from __future__ import print_function
from __future__ import division
# compute arctangent table for CORDIC computations in fttrigon.c
from builtins import range
from past.utils import old_div
import sys, math

#units  = 64*65536.0   # don't change !!
units  = 256
scale  = old_div(units,math.pi)
shrink = 1.0
comma  = ""

def calc_val( x ):
    global units, shrink
    angle  = math.atan(x)
    shrink = shrink * math.cos(angle)
    return angle/math.pi * units

def  print_val( n, x ):
    global comma

    lo  = int(x)
    hi  = lo + 1
    alo = math.atan(lo)
    ahi = math.atan(hi)
    ax  = math.atan(2.0**n)

    errlo = abs( alo - ax )
    errhi = abs( ahi - ax )

    if ( errlo < errhi ):
      hi = lo

    sys.stdout.write( comma + repr( int(hi) ) )
    comma = ", "


print("")
print("table of arctan( 1/2^n ) for PI = " + repr(old_div(units,65536.0)) + " units")

# compute range of "i"
r = [-1]
r = r + list(range(32))

for n in r:

    if n >= 0:
        x = old_div(1.0,(2.0**n))    # tangent value
    else:
        x = 2.0**(-n)

    angle  = math.atan(x)    # arctangent
    angle2 = angle*scale     # arctangent in FT_Angle units

    # determine which integer value for angle gives the best tangent
    lo  = int(angle2)
    hi  = lo + 1
    tlo = math.tan(old_div(lo,scale))
    thi = math.tan(old_div(hi,scale))

    errlo = abs( tlo - x )
    errhi = abs( thi - x )

    angle2 = hi
    if errlo < errhi:
        angle2 = lo

    if angle2 <= 0:
        break

    sys.stdout.write( comma + repr( int(angle2) ) )
    comma = ", "

    shrink = shrink * math.cos( old_div(angle2,scale))


print()
print("shrink factor    = " + repr( shrink ))
print("shrink factor 2  = " + repr( shrink * (2.0**32) ))
print("expansion factor = " + repr(old_div(1,shrink)))
print("")

