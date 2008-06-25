#!/usr/bin/perl -w
#
# (c) vir
#
# Last modified: Wed Jun 25 17:04:17 MSD 2008
#

use strict;
use warnings FATAL => 'uninitialized';

my($curfont, @all_fonts);

open STDIN, "-|", "tar xOf Core14_AFMs.tar" or die "Can't untar font metrix: $!";

my $flag = 0;
while(<STDIN>) {
	s/^\s+//; s/\s+$//; next if /^$/;
	if(/^C\s+(\d+)\s*;\s*WX\s+(\d+)/) { # <<  C 123 ; WX 348 ; N braceleft ; B 5 -187 436 686 ;  >>
		$flag = 1;
		$curfont->{W}{$1} = $2;
		next;
	}
	if($flag) {
		undef $curfont;
		$flag = 0;
	}
	unless($curfont) {
		$curfont = {};
		push(@all_fonts, $curfont);
	}
	if(/^FontName\s+(.*)/) {
		$curfont->{N} = $1;
		$curfont->{CN} = 'widths_'.lc($1);
		$curfont->{CN} =~ s#\W+#_#sg;
	}
}

foreach my $f(@all_fonts) {
	next unless $f->{N};
	print "const static short ".$f->{CN}."[] = {\n\t";
	my $pos = 0;
	for(my $i = 0; $i < 256; $i++) {
		if($pos > 65) {
			$pos = 0;
			print "\n\t";
		}
		my $l = ($f->{W}{$i} || 0).", ";
		print $l;
		$pos += length($l);
	}
	print "\n};\n";
}

print "static struct width_table standard_font_widths_table[] = {\n";
foreach my $f(@all_fonts) {
	next unless $f->{N};
	print "\t{ \"".$f->{N}."\", ".$f->{CN}.", sizeof(".$f->{CN}.")/sizeof(".$f->{CN}."[0]) },\n";
}
print "};\n";





