# This is a replacement for the old BEGIN preamble which heads (or
# should head) up every core test program to prepare it for running.
# Now instead of:
#
# BEGIN {
#   chdir 't' if -d 't';
#   @INC = '../lib';
# }
#
# Its primary purpose is to clear @INC so core tests don't pick up
# modules from an installed Perl.
#
# t/TEST will use -MTestInit.  You may "use TestInit" in the test
# programs but it is not required.
#
# P.S. This documentation is not in POD format in order to avoid
# problems when there are fundamental bugs in perl.

package TestInit;

$VERSION = 1.04;

# Let tests know they're running in the perl core.  Useful for modules
# which live dual lives on CPAN.
# Don't interfere with the taintedness of %ENV, this could perturbate tests.
# This feels like a better solution than the original, from
# http://www.xray.mpe.mpg.de/mailing-lists/perl5-porters/2003-07/msg00154.html
$ENV{PERL_CORE} = $^X;

$0 =~ s/\.dp$//; # for the test.deparse make target

sub import {
    my $self = shift;
    my @up_2_t = ('../../lib', '../../t');
    my ($abs, $chdir, $setopt);
    foreach (@_) {
	if ($_ eq 'U2T') {
	    @INC = @up_2_t;
	    $setopt = 1;
	} elsif ($_ eq 'U1') {
	    @INC = '../lib';
	    $setopt = 1;
	} elsif ($_ eq 'NC') {
	    delete $ENV{PERL_CORE}
	} elsif ($_ eq 'A') {
	    $abs = 1;
	} elsif ($_ eq 'T') {
	    $chdir = '..'
		unless -f 't/TEST' && -f 'MANIFEST' && -d 'lib' && -d 'ext';
	    @INC = 'lib';
	    $setopt = 1;
	} else {
	    die "Unknown option '$_'";
	}
    }

    # Need to default. This behaviour is consistent with previous behaviour,
    # as the equivalent of this code used to be run at the top level, hence
    # would happen (unconditionally) before import() was called.
    unless ($setopt) {
	if (-f 't/TEST' && -f 'MANIFEST' && -d 'lib' && -d 'ext') {
	    # We're being run from the top level. Try to change directory, and
	    # set things up correctly. This is a 90% solution, but for
	    # hand-running tests, that's good enough
	    if ($0 =~ s!^((?:ext|dist|cpan)[\\/][^\\/]+)[\\/](.*\.t)$!$2!) {
		# Looks like a test in ext.
		$chdir = $1;
		@INC = @up_2_t;
		$setopt = 1;
		$^X =~ s!^\.([\\/])!..$1..$1!;
	    } else {
		$chdir = 't';
		@INC = '../lib';
		$setopt = $0 =~ m!^lib/!;
	    }
	} else {
	    # (likely) we're being run by t/TEST or t/harness, and we're a test
	    # in t/
	    @INC = '../lib';
	}
    }

    if (defined $chdir) {
	chdir $chdir or die "Can't chdir '$chdir': $!";
    }

    if ($abs) {
	require File::Spec::Functions;
	# Forcibly untaint this.
	@INC = map { $_ = File::Spec::Functions::rel2abs($_); /(.*)/; $1 } @INC;
	$^X = File::Spec::Functions::rel2abs($^X);
    }

    if ($setopt) {
	my $sep;
	if ($^O eq 'VMS') {
	    $sep = '|';
	} elsif ($^O eq 'MSWin32') {
	    $sep = ';';
	} else {
	    $sep = ':';
	}

	my $lib = join $sep, @INC;
	if (exists $ENV{PERL5LIB}) {
	    $ENV{PERL5LIB} = $lib . substr $ENV{PERL5LIB}, 0, 0;
	} else {
	    $ENV{PERL5LIB} = $lib;
	}
    }

    push @INC, '.' unless ${^TAINT};
}

1;
