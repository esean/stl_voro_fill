#!/usr/bin/perl -w
`_script_counter.sh $0 @ARGV`;

#---------------------------------------------------------------
# $Copyright$
#---------------------------------------------------------------
# minmax.pl
#
# Finds the minimum, maximum, delta, average and stddev of provided dataset.
#
# Flags:
#	-i	find didt min/max (uses averaging)
#	-d	turn on debug
#---------------------------------------------------------------

use strict;
use Getopt::Std;

$ENV{PATH} = "$ENV{PATH}:../../../bin";

my $debug = 0;
my %opt = ();
getopts( 'hf:idcn', \%opt );
$debug = 1 if ($opt{'d'});

&usage if ( $opt{'h'} );
my $filein = 1;
if ( !( $opt{'f'} ) ) {

    # read in from <STDIN>
    $filein = 0;

    #die "Provide a dataset filename (with -f [filename] option) in which to find min and max values!";
}
if ( $filein == 1 ) {
    die "Could not find dataset file!" if ( !-f $opt{'f'} );
}

# TODO: if data is like ' 0.98' it thinks it has more than one column... 
my @data = ();
my $line;
if ( $filein == 1 ) {
    open T2, "<$opt{'f'}" or die "Could not open input dataset file:$opt{'f'}!";
    while ( $line = <T2> ) {
        chomp($line);
        next if ( $line =~ /^$/ );
        next if ( $line =~ /^#/ );
        if ( $line =~ /^(.*) (.*)$/ ) {
            die "This script can only process single-column dataset files!";
        }
        push @data, $line;

        #print "DATA:$line\n";
    }
    close T2;
} else {
    while ( $line = <STDIN> ) {
        chomp($line);
        next if ( $line =~ /^$/ );
        next if ( $line =~ /^#/ );
        if ( $line =~ /^(.*) (.*)$/ ) {
            die "This script can only process single-column dataset files!";
        }
        push @data, $line;

        #print "DATA:$line\n";
    }
}

my ( $min, $max, $avg, $stddev, $cnt, $total ) = 0;

if ($opt{'i'}) {
	( $min, $max, $avg, $stddev, $cnt, $total ) = &min_and_max_didt(@data);
} else {
	( $min, $max, $avg, $stddev, $cnt, $total ) = &min_and_max(@data);
}
if ($opt{'c'}) {
    if (!$opt{'n'}) {
        print "# count,min,max,delta,avg,stddev,total\n";
    }
    my $delta = $max-$min;
    print "$cnt,$min,$max,$delta,$avg,$stddev,$total\n";
} else {
    print "COUNT:$cnt MIN:$min MAX:$max DELTA:" . ( $max - $min ) . " AVG:$avg STDDEV:$stddev TOTAL:$total\n";
}

exit(0);

######################

sub min_and_max {
    my (@numbers);

    @numbers = @_;

    my ( $min, $max, $mean, $stddev );
    my ( $total, $cnt ) = (0,0);

    $min = $numbers[0];
    $max = $numbers[0];

    foreach my $i (@numbers) {
        $total += $i;
        $cnt++;
        if ( $i > $max ) {
            $max = $i;
        } elsif ( $i < $min ) {
            $min = $i;
        }
    }

    $mean = $total / $cnt;
    my $avg_mean_total = 0;
    foreach my $i (@numbers) {
        $avg_mean_total += ( $i - $mean )**2;
    }
    $stddev = sqrt($avg_mean_total / $cnt);

    return ( $min, $max, $mean, $stddev, $cnt, $total );
}

sub min_and_max_didt {
    my (@numbers) = @_;

    my ( $min, $max, $mean, $stddev );
    my ( $total, $cnt ) = (0,0);

    $min = $numbers[0];
    $max = $numbers[0];

    #print "min_and_max DEBUG: " if ( $main::cmd_opt{debug} );
    foreach my $i (@numbers) {
        $total += $i;
        $cnt++;
        if ( $i > $max ) {
            $max = $i;
        } elsif ( $i < $min ) {
            $min = $i;
        }
    }

    # find midpoint, then we can sort the numbers into two buckets, those above and below midpoint
    my $pt = ( $min + $max ) / 2;

    # stuff our @amin and @amax arrays so avg is weighted
    my @amin = ();
    my @amax = ();
    foreach my $i (@numbers) {
        if ( $i < $pt ) {
            # LOWER
	    push @amin, $i;
        } else {
            # HIGHER
	    push @amax, $i;
        }
    }

    # take the avg of the high section, then low section
    my ( $maxmin, $maxmax, $maxavg, $maxstddev, $maxcnt, $maxtotal ) = &min_and_max(@amax);
    print "INFO:max COUNT:$maxcnt MIN:$maxmin MAX:$maxmax DELTA:" . ( $maxmax - $maxmin ) . " AVG:$maxavg STDDEV:$maxstddev TOTAL:$maxtotal\n" if $debug>0;
    my ( $minmin, $minmax, $minavg, $minstddev, $mincnt, $mintotal ) = &min_and_max(@amin);
    print "INFO:min COUNT:$mincnt MIN:$minmin MAX:$minmax DELTA:" . ( $minmax - $minmin ) . " AVG:$minavg STDDEV:$minstddev TOTAL:$mintotal\n" if $debug>0;

    $mean = $total / $cnt;
    my $avg_mean_total = 0;
    foreach my $i (@numbers) {
        $avg_mean_total += ( $i - $mean )**2;
    }
    $stddev = sqrt($avg_mean_total / $cnt);
    return ( $minavg, $maxavg, $mean, $stddev, $cnt, $total );
}

sub usage {
    print "\n";
    print "$0 {[FLAGS]} {-f [dataset filename]}\n";
    print "\n";
    print "\tFinds min/max/mean/stddev for provided dataset.\n";
    print "\tDataset can be provided either with '-f' option, or\n";
    print "\tfrom data on command-line.\n";
    print "\n";
    print "FLAGS:\n";
    print "\n\t\t-i\tUse didt min/max algorithm\n";
    print "\n\t\t-d\tTurn on debug msgs\n";
    print "\n\t\t-c\tOutput in CSV format\n";
    print "\n\t\t-n\tSuppress outputting CSV header, just dump data\n";
    print "\n";
    exit(0);
}

