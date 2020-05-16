#!/usr/bin/perl
my $dbg = 0;
my @a = grep { $_ ne '-d' } @ARGV;
$dbg = 1 if ($#a != $#ARGV);
my $f = $a[0];
my $o = $a[1];
my $cmdh = "sparc-elf-objdump -x $f";
my $cmds = "sparc-elf-objdump -s $f";
printf("Extracting data from $f\nheader info: $cmdh\ndata       : $cmds\n");
my $h = `$cmdh`;
my $s = `$cmds`;
print ($h.$s) if $dbg;

my $srec = "";
$a = "0000".join("",map { sprintf("%02x",ord($_)) } split("",$o));
$l = length($a) / 2;
$a = sprintf("%02x",$l+1).$a;
$chk = chksum($a);
$srec = uc("S0".$a.sprintf("%02x",($chk ^ -1) & 0xff))."\r\n";

while ($s =~ m/Contents of section ([a-zA-Z0-9_\.]+):[\n\r]+((?: [^\n]*\n)*)/s) {
    my ($n,$p) = ($1,$2); 
    $s = substr($s,length($`.$&));
    $p =~ s/[\s\n]*$//s;
    print "Found section $n";
    if ($n =~ /(?:^\.text|^\.data)/) {
	($h =~ /$n\s+[a-fA-F0-9]+\s+([a-fA-F0-9]+)\s+([a-fA-F0-9]+)\s+/) or die("Cannot extract LMV from of $n from $h\n");
	my ($vma,$lma) = (hex($1),hex($2));
	print (" VMA:$1 LMA:$2");
	foreach my $e (split("\n",$p)) {
	    ($e =~ m/^ ([0-9A-Fa-f]+)/) or die ("Mismatched address at \"$e\"\n");
	    $addr = hex($1) - $vma + $lma;
	    $data = substr($e,length($&),4*(8+1));
	    $data =~ s/\s*//g; $l = (length($data) / 2)+5;
	    $a = sprintf("%08x",$addr); my $at = "S3";
	    if (substr($a,0,2) eq '00') {
		$a = sprintf("%06x",$addr); $at = "S2"; $l--;
	    }
	    my $line = sprintf("%02x%s",$l,$a.$data);
	    my $chk = chksum($line);
	    $srec.= uc($at.$line.sprintf("%02x",($chk ^ -1) & 0xff))."\r\n";
	}
	print ("\n");
    } else {
	print " (ignore)\n";
    }
}

($h =~ m/start address 0x([a-fA-F0-9]+)/s) or die("Cant find start address\n");
my $start = hex($1);
$a = sprintf("%08x",$start); my $at = "S7"; $l = 4;
if (substr($a,0,2) eq '00') {
    $a = sprintf("%06x",$start); $at = "S8"; $l = 3;
}
my $line = sprintf("%02x%s",$l+1,$a);
my $chk = chksum($line);
$srec.= uc($at.$line.sprintf("%02x",($chk ^ -1) & 0xff))."\r\n";

open OUT, ">$o" or die $!;
print OUT ($srec);
close OUT;

sub chksum {
    my ($line) = @_;
    my $chk = 0;
    for (my $i = 0;$i < length($line)/2;$i++) {
	$chk += hex(substr($line,$i*2,2));
    }
    return $chk;
}
