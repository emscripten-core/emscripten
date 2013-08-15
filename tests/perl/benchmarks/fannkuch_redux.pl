# The Computer Language Benchmarks Game
# http://benchmarksgame.alioth.debian.org/
# initial fannkuch port from C by Steve Clark
#   rewrite by Kalev Soikonen
#   modified by Kuang-che Wu
#   modified by David Golden
# updated for fannkuch-redux by Jonathan DePeri
#   permutations generated using Mike Pall's approach

use integer;

sub fannkuchredux {
    my ($n) = shift;
    my ($m, $checksum, $maxflips, $flips, $sign) = ($n-1, 0, 0, 0, 1);
    my ($p, $q, $f, $i, @count);
    
    @count = (0..$m); 
    $p = pack "c*", @count;

    do {
        if (ord(substr($p,0))) {
            $q = $p;
            $flips = 0;
            while ($f = ord(substr($q,0))) {
                $flips++;
                substr($q, 0, $f+1, reverse(substr($q,0,$f+1)));
            }
            $maxflips = $flips if ($flips > $maxflips);
            $checksum += ($sign * $flips);
        }
        
        return if ($n <= 1);
        if ($sign == 1) {
            $sign = -1;
            substr $p, 1, 0, (substr($p,0,1,""));
        } else {
            return if ($n <= 2);
            $sign = 1;
            substr $p, 1, 0, (substr($p,2,1,""));
            for $i (2..$m) {
	            if ($count[$i]) { $count[$i]--; last; }
	            return ($checksum, $maxflips) if ($i == $m);
	            $count[$i] = $i;
	            substr $p, $i+1, 0, (substr($p,0,1,""));
            }
        }
    } while (1);
}

$n = 9;
my ($checksum, $maxflips) = fannkuchredux($n);
print "$checksum\n";
print "Pfannkuchen($n) = $maxflips\n";
