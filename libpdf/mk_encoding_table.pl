#!/usr/bin/perl
#

use strict;
while(<>) {
	s#^\s+##s; s#\s+$##s;
	next if /^#/ || /^$/;
#	my($name, $code) = split(';', $_);
	next unless /^(.*?);([0-9a-fA-F]+)/;
	my($name, $code) = ($1, $2);
	printf "\t{\"%s\", 0x%s},\n", $name, $code;
}

