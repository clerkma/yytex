#! /usr/local/bin/perl

$sccsid = "@(#)pfatopfb	1.2 10/16/94";

# A Perl script to do .pfa to .pfb font conversion.
# Written, and put into the public domain with no warranty of any kind,
# by Peter Ilieve (peter@memex.co.uk), 16 Oct 94.

# Usage:
# either as a filter, with no filename arguments
# or: pfatopfb file file ...
# where each file, assumed to be a .pfa file, is converted into a new
# file in the current directory with .pfa replaced by .pfb

$eol = "\r"; # default end of line char
$flags = shift if ($ARGV[0] =~ /^-/);
if ($flags =~ /[^-nv]/) {
	print STDERR "Usage: pfatopfb [-nv] [file file ...]
where -v is verbose about filenames and sizes
and -n uses newline rather than return to mark output lines.
If files are given, it creates files with suffix .pfb in the current directory.
If no files are given, it acts as a filter.\n";
	exit 1;
}
$verbose = 1 if ($flags =~ /v/);
$eol = "\n" if ($flags =~ /n/);

if ($#ARGV == -1) {
	&process(STDIN, STDOUT, "stdin");
}
else {
	foreach $arg (@ARGV) {
		($file = $arg) =~ s-.*/--; # strip leading path
		if ($file !~ /(.+)\.pfa/i) {
			print STDERR "\"$arg\" doesn't look like a PFA filename, skipping it.
Use pfatopfb as a filter to process arbitrary filenames.\n";
			next;
		}
		$base = $1;
		open(IN, $arg) || die "can't open input file \"$arg\": $!\n";
		open(OUT, ">$base.pfb") || die "can't open output file \"$base.pfb\": $!\n";
		if (!&process(IN, OUT, $arg)) {
			# it failed, unlink the output file we just created
			close OUT;
			unlink "$base.pfb";
		}
	}
}

# process one file, given open filehandles to input and output
# go through whole file first, saving the lines, so we can calculate the
# segment lengths
sub process {
	local($infh, $outfh, $name) = @_[0..2];
	local(@header, @body, @trailer, $len, $inheader, $inbody,
				$headerlen, $bodylen, $trailerlen);

	print STDERR "File: $name\n" if ($verbose);
	$len = 0;
	$inheader = 1;
	$inbody = 0;
	@header = @body = @trailer = ();
	while (<$infh>) {
		# deal with PC and unix end of line conventions
		# we strip off any line end marker and add our own on later for
		# the ascii sections
		s/\r?\n$//;
		$len += length;
		if ($inheader) {
			$len++; # to allow for the \r (or \n) we will add later
			push(@header, $_);
			if (/ eexec$/) {
				# last line of header, at least we assume it is
				# setup for binary stuff starting on next line
				$inheader = 0;
				$inbody = 1;
				$headerlen = $len;
				$len = 0;
			}
		}
		elsif ($inbody) {
			if (/^0{16,}/) {
				# first line of final ascii section, so don't put this
				# in binary part
				$inbody = 0;
				$len = length; # as this line is part of trailer
				$len++;
				push(@trailer, $_);
			}
			else {
				# check that there are pairs of hex digits as we assume
				# this later
				if (length($_) % 2) {
					print STDERR "$name: line $.: hex data line has odd length, skipping this file\n";
					return 0;
				}
				# always update body length, waiting til we see the 00000
				# is too late as $len will have been updated by then
				$bodylen = $len;
				push(@body, $_);
			}
		}
		else {
			$len++;
			push(@trailer, $_);
		}
	}
	$trailerlen = $len;

	# now put it out in .pfb form
	print STDERR "ascii header: $headerlen bytes\n" if ($verbose);
	print $outfh "\x80\x01", pack("V", $headerlen);
	foreach (@header) {
		print $outfh "$_$eol";
	}
	$bodylen /= 2; # as we are compressing 2:1
	print STDERR "binary body: $bodylen bytes\n" if ($verbose);
	print $outfh "\x80\x02", pack("V", $bodylen);
	foreach (@body) {
		$len = length;
		print $outfh pack("H$len", $_);
	}
	print STDERR "ascii trailer: $trailerlen bytes\n" if ($verbose);
	print $outfh "\x80\x01", pack("V", $trailerlen);
	foreach (@trailer) {
		print $outfh "$_$eol";
	}
	print STDERR "EOF\n" if ($verbose);
	print $outfh "\x80\x03";
	1; # say we succeeded
}

