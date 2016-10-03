#!/usr/bin/perl
$connect_count = 1000;
$node_count = 100;
$pin_count = 500;


open(OUTFILE, ">dfout.txt") or die "Can't open logfile: $!";  

for ($i=0;$i<$connect_count;$i++)
{

		$src_node = int( rand()*$node_count);
		$src_pin = int( rand()*$pin_count);
		$dst_node = int( rand()*$node_count);
		$dst_pin = int( rand()*$pin_count);

		print OUTFILE "$i: $src_node $src_pin $dst_node $dst_pin\n";

}

