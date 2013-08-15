# The Computer Language Benchmarks game
# http://benchmarksgame.alioth.debian.org/
#
# contributed by Barry Walsh

# port of fasta.rb #6 

use strict;
use warnings;
use feature 'say';

use constant IM => 139968;
use constant IA => 3877;
use constant IC => 29573;

my $LAST = 42;

my $alu =
    'GGCCGGGCGCGGTGGCTCACGCCTGTAATCCCAGCACTTTGG' .
    'GAGGCCGAGGCGGGCGGATCACCTGAGGTCAGGAGTTCGAGA' .
    'CCAGCCTGGCCAACATGGTGAAACCCCGTCTCTACTAAAAAT' .
    'ACAAAAATTAGCCGGGCGTGGTGGCGCGCGCCTGTAATCCCA' .
    'GCTACTCGGGAGGCTGAGGCAGGAGAATCGCTTGAACCCGGG' .
    'AGGCGGAGGTTGCAGTGAGCCGAGATCGCGCCACTGCACTCC' .
    'AGCCTGGGCGACAGAGCGAGACTCCGTCTCAAAAA';

my $iub = [
    [ 'a', 0.27 ],
    [ 'c', 0.12 ],
    [ 'g', 0.12 ],
    [ 't', 0.27 ],
    [ 'B', 0.02 ],
    [ 'D', 0.02 ],
    [ 'H', 0.02 ],
    [ 'K', 0.02 ],
    [ 'M', 0.02 ],
    [ 'N', 0.02 ],
    [ 'R', 0.02 ],
    [ 'S', 0.02 ],
    [ 'V', 0.02 ],
    [ 'W', 0.02 ],
    [ 'Y', 0.02 ]
];

my $homosapiens = [
    [ 'a', 0.3029549426680 ],
    [ 'c', 0.1979883004921 ],
    [ 'g', 0.1975473066391 ],
    [ 't', 0.3015094502008 ]
];

sub make_repeat_fasta {
    my ($src, $n) = @_;
    my $width = qr/(.{1,60})/;
    my $l     = length $src;
    my $s     = $src x (($n / $l) + 1);
    substr($s, $n, $l) = '';

    while ($s =~ m/$width/g) { say $1 }
#   say for unpack '(a60)*', $s;   # slower than above over larger strings
}

sub make_random_fasta {
    my ($table, $n) = @_;
    my $rand   = undef;
    my $width  = 60;
    my $prob   = 0.0;

    $_->[1] = ($prob += $_->[1]) for @$table;

    my $collector = '$rand = ($LAST = ($LAST * IA + IC) % IM) / IM;';
    $collector .= "print('$_->[0]') && next if $_->[1] > \$rand;\n" for @$table;

    my $code = q{
        for (1..($n / $width)) {
            for (1..$width) { !C! }
            print "\n";
        }
        if ($n % $width != 0) {
            for (1 .. $n % $width) { !C! }
            print "\n";
        }
    };
    $code =~ s/!C!/$collector/g;
    eval $code;
}



my $n = $ARGV[0] || 27;

say ">ONE Homo sapiens alu";
make_repeat_fasta($alu, $n*2);

say ">TWO IUB ambiguity codes";
make_random_fasta($iub, $n*3);

say ">THREE Homo sapiens frequency";
make_random_fasta($homosapiens, $n*5);

