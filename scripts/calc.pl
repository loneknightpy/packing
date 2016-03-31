#! /usr/bin/perl

for ($i = 0; $i <= 15; $i++)
{
	open FILE, $ARGV[0].$i;
	@lines = <FILE>;
	close(FILE);
	$line = $lines[-3];
	#print $line;
	@items = split ' ', $line;
	($rate1) = split /%/, $items[2];
	($rate2) = split /%/, $items[3];
	($rate3) = split /%/, $items[4];
	$count = $items[5];
	$time = $items[7];

	#printf("%d\t%.2f%%\t%.2f%%\t%.2f%%\t%d\t%s\n", 
	#	$i, $rate1, $rate2, $rate3, $count, $time);
	#print "$i $rate1 $rate2 $rate3 $count $time\n"
	printf("%.2f%% ", $rate3);
}
printf("\n");
