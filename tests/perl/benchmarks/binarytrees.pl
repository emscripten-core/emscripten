# The Computer Language Benchmarks Game
# http://benchmarksgame.alioth.debian.org/
# 
# contributed by Emanuele Zeppieri

sub bottomup_tree {
    my ($value, $depth) = @_;
    return $value unless $depth;
    my $value2 = $value * 2; $depth--;
    [ bottomup_tree($value2-1, $depth), bottomup_tree($value2, $depth), $value ]
}

sub check_tree {
    my ($left, $right, $value) = @{ $_[0] };
    $value + (
        ref $left ? check_tree($left) - check_tree($right) : $left - $right
    )
}

my $max_depth = 6;
my $min_depth = 4;

$max_depth = $min_depth + 2 if $min_depth + 2 > $max_depth;

my $stretch_depth = $max_depth + 1;
my $stretch_tree = bottomup_tree(0, $stretch_depth);
print "stretch tree of depth $stretch_depth\t check: ",
    check_tree($stretch_tree), "\n";
undef $stretch_tree;

my $longlived_tree = bottomup_tree(0, $max_depth);

for ( my $depth = $min_depth; $depth <= $max_depth; $depth += 2 ) {
    my $iterations = 2 << $max_depth - $depth + $min_depth - 1;
    my $check = 0;
    
    foreach (1..$iterations) {
        $check += check_tree( bottomup_tree(0, $depth) );
        $check += check_tree( bottomup_tree(0, $depth) )
    }
    
    print 2*$iterations, "\t trees of depth $depth\t check: ", $check, "\n"
}

print "long lived tree of depth $max_depth\t check: ",
    check_tree($longlived_tree), "\n"

