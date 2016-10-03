#!/usr/bin/perl

$file = "test.txt";
open(FILE, $file) || die "ERROR: Can't open $file";
open(BINFILE, ">binned.txt") or die "Can't open logfile: $!";  
@line_list = <FILE>;


$i=0;
$j=0;
foreach $line (@line_list)
{
	chomp($line);

	if($line =~ /clock/)
	{		
		@item_list = split / /, $line; 
		$time = $item_list[1];
	}

	if($line =~ /spike/)
	{
		@item_list = split / /, $line; 
		$spike1 = $item_list[6];

		$diff = $spike1 - $spike2;
		print "$diff\n";

		$spike2 = $spike1;

	}

	

	$num_cols = @item_list;
#	print "$num_cols\n";

	
	
}




# Local Variables:
# mode:perl
# End:
