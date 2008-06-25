#!/usr/bin/perl -w
#
# Parses encoding tables and writes glyphlist.i and enc_*.i include files.
#
# Last modified: Wed Jun 25 10:07:20 MSD 2008
#

use strict;
use warnings FATAL => 'uninitialized';
#use CGI::Carp 'fatalsToBrowser';

my(%unicode, %std, %mac, %win, %pdf);

open F, "< glyphlist.txt" or die "Can't open glyphlist.txt: $!";
while(<F>) {
	s#^\s+##s; s#\s+$##s;
	next if /^#/ || /^$/;
	next unless /^(.*?);([0-9a-fA-F]+)/; # XXX skip multichars
	my($name, $code) = ($1, $2);
	$unicode{$name} = hex($code);
#	printf "\t{\"%s\", 0x%s},\n", $name, $code;
}
close F;

open F, "< pdf_encodings.txt" or die "Can't open pdf_encodings.txt: $!";
while(<F>) {
	chomp;
	next unless /^\w/;
	my($n, $std, $mac, $win, $pdf) = unpack("A15A4A4A4A4", $_);
	$std{oct($std)} = $n if $std=~/\S/;
	$mac{oct($mac)} = $n if $mac=~/\S/;
	$win{oct($win)} = $n if $win=~/\S/;
	$pdf{oct($pdf)} = $n if $pdf=~/\S/;
#	printf("%20s: %02X %02X %02X %02X\n", $n, oct($std), oct($mac), oct($win), oct($pdf));
}
close F;

#dump_enc(\%win);
#make_array(\%win);
write_glyphlist_table();
write_encoding_table('win', \%win);
write_encoding_table('mac', \%mac);
write_encoding_table('std', \%std);

sub dump_enc
{
	my($e) = @_;
	foreach(sort { $a <=> $b } keys(%$e)) {
		my $n = $e->{$_};
		my $u = $unicode{$n} or warn("Can't translate $_ ($n) to unicode"), next;
		printf(" %02X %20s %04X\n", $_, $n, $u);
	}
}
sub make_array
{
	my($e) = @_;
	for(0 .. 255) {
		my $n = $e->{$_} || '- unused -';
		my $u = $unicode{$n} || $_;
		printf("\t0x%04X, // %02X: %s\n", $u, $_, $n);
	}
}
sub write_encoding_table
{
	my($name, $e) = @_;
	open E, "> enc_$name.i" or die "Can't open enc_$name.i for writing: $!";
	select E;
	make_array($e);
	select STDOUT;
	close E or die "Can't write to enc_$name.i: $!";
}

sub write_glyphlist_table
{
	open E, "> glyphlist.i" or die "Can't open glyphlist.i for writing: $!";
	foreach(sort keys %unicode) {
		printf E "\t{\"%s\", 0x%04X},\n", $_, $unicode{$_};
	}
	close E or die "Can't write to glyphlist.i: $!";
}

