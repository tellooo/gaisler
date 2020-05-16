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



if (!($m =~ m/N1 floating point\s*-1\.12398255667391900\s*/ms || $m =~ m/N1 floating point\s*-1\.12475013732910156\s*/ms)) {
    fail(1);
}
if (!($m =~ m/N2 floating point\s*-1\.12187079889284425\s*/ms || $m =~ m/N2 floating point\s*-1\.12274742126464844\s*/ms)) {
    fail(1);
}
if (!($m =~ m/N3 if then else\s*1.00000000000000000\s*/ms)) {
    fail(1);
}
if (!($m =~ m/N4 fixed point\s*12.00000000000000000\s*/ms)) {
    fail(1);
}
if (!($m =~ m/N5 sin,cos etc\.\s*0\.49902937281518273\s*/ms || $m =~ m/N5 sin,cos etc\.\s*0\.49911010265350342\s*/ms)) {
    fail(1);
}
if (!($m =~ m/N6 floating point\s*0.99999987890802811\s*/ms || $m =~ m/N6 floating point\s*0\.99999982118606567\s*/ms)) {
    fail(1);
}
if (!($m =~ m/N7 assignments\s*3.00000000000000000\s*/ms)) {
    fail(1);
}
if (!($m =~ m/N8 exp,sqrt etc.\s*0.75100163018458932\s*/ms || $m =~ m/N8 exp,sqrt etc.\s*0.75110864639282227\s*/ms)) {
    fail(1);
}
if (!($m =~ m/Results  to  load  to  spreadsheet\s*([0-9\.]+)\s*([0-9\.]+)\s*([0-9\.]+)\s*([0-9\.]+)\s*([0-9\.]+)\s*([0-9\.]+)\s*([0-9\.]+)\s*([0-9\.]+)\s*([0-9\.]+)/ms)) {
    fail(1);
}

print $OPT{'name'}.": MWIPS $1 "." => succeeded\n";

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

