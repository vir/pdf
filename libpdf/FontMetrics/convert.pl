#!(perl*

use strict;
use warnings;
use FindBin;
use Data::Dumper;

my %afms;
my %unicode;

load_glyphs_list();

foreach my $fn(glob "$FindBin::Bin/*.afm") {
	warn "Loading $fn\n";
	process_afm_file($fn);
}

# print Dumper(\%afms);

foreach my $fontname(sort keys %afms) {
	my $n = $fontname;
	$n =~ s#[^a-zA-Z]##sg;
	printf "static struct StdFontChar stdfont_%s_chars[] = {\n", $n;
	foreach my $row(@{ $afms{$fontname}{CharMetrics} }) {
		printf "\t{ \"%s\", 0x%04X, %d, %d, \"%s\" },\n", $row->{N}, $unicode{$row->{N}} // 0, $row->{C}, $row->{WX}, $row->{B};
	}
	print "};\n";
}
print "static struct StdFontMetric fonts[] = {\n";
foreach my $fontname(sort keys %afms) {
	my $n = $fontname;
	$n =~ s#[^a-zA-Z]##sg;
	printf "\t{ \"%s\", %s, %d, stdfont_%s_chars },\n", $fontname, $afms{$fontname}{EncodingScheme} // 'Unknown', scalar @{ $afms{$fontname}{CharMetrics} }, $n;
}
print "};\n";

sub load_glyphs_list
{
	open F, '<', $FindBin::Bin.'/../glyphlist.txt' or die "Error opening glyphlist.txt: $!";
	while(<F>) {
		s#^\s+##s; s#\s+$##s;
		next if /^#/ || /^$/;
		next unless /^(.*?);([0-9a-fA-F]+)/; # XXX skip multichars
		my($name, $code) = ($1, $2);
		$unicode{$name} = hex($code);
	}
	close F;
}

sub process_afm_file
{
	my($fn) = @_;
	my @where = qw( top );
	my $data = { };
	open F, '<', $fn or die;
	while(<F>) {
		chomp;
		if(/^(Start|End)(\w+)/) {
			if($1 eq 'Start') {
				push @where, $2;
			} else {
				pop @where;
			}
			next;
		}
		if($where[$#where] eq 'FontMetrics') {
			if(/^(\w+)\s+(.*)/) {
				$data->{$1} = $2;
			} else {
				die "Can't parse <<$_>>";
			}
		} elsif($where[$#where] eq 'CharMetrics') {
			my %char;
			foreach my $tuple(split(/\s*;\s*/, $_)) {
				if($tuple =~ /^([a-zA-Z]+)\s+(.*)/) {
					$char{$1} = $2;
				} else {
					die "Bad chat info tuple <<$tuple>>";
				}
			}
			push @{ $data->{$where[$#where]} }, \%char;
		} elsif($where[$#where] eq 'KernPairs') {
			# XXX ignoring kerning pairs
		} else {
			die "Nowhere @where";
		}
	}
	close F;
	$afms{$data->{FontName}} = $data;
}

