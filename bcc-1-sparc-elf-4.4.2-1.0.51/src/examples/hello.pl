#!/usr/bin/perl
use Getopt::Long;
Getopt::Long::Configure(qw(bundling));
GetOptions(\%OPT,qw{
    quite|q+
    verbose|v+
    name=s
},@g_more) or usage(\*STDERR);


if (scalar(@ARGV) != 1) {
    die("Error: $0 <file.info>, called with ".join(" ",@ARGV));
}
$m = "";
while(<>) { $m .= $_; }
#print $m;

sub fail {
    print $OPT{'name'}.":".$_[0]." => failed\n".$m;
    exit(1);
}


if (!($m =~ m/Hello/ms)) {
    fail("Cannot find \"Hello\" output");
}

print $OPT{'name'}.": \"Hello\" outputted "." => succeeded\n";
exit(0);

__DATA__

N1 floating point      -1.12398255667391900         5.513               0.495
N2 floating point      -1.12187079889284425         5.428               3.516
N3 if then else         1.00000000000000000                  24.980     0.588     1.000
N4 fixed point         12.00000000000000000                 149.841     0.299    12.000
N5 sin,cos etc.         0.49902937281518273                   0.280    42.209     0.499
N6 floating point       0.99999987890802811         4.222              18.141
N7 assignments          3.00000000000000000                   4.542     5.778     3.000
N8 exp,sqrt etc.        0.75100163018458932                   0.188    28.140     0.751

MWIPS                                              14.319              99.166

Results  to  load  to  spreadsheet                  MWIPS   Mflops1   Mflops2   Mflops3   Cosmops   Expmops  Fixpmops    Ifmops    Eqmops
Results  to  load  to  spreadsheet                 14.319     5.513     5.428     4.222     0.280     0.188   149.841    24.980     4.542

