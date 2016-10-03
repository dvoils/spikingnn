#!/usr/bin/perl

#$file = "test.txt";
#open(FILE, $file) || die "ERROR: Can't open $file";
#open(BINFILE, ">binned.txt") or die "Can't open logfile: $!";  
#@line_list = <FILE>;

$a = 0;
for ($i=0;$i<10;$i++)
{
	$a = $a + 409.6;
	$hexval = sprintf("%x", $a);

	print "$i $a $hexval\n";

}






# Local Variables:
# mode:perl
# End:
