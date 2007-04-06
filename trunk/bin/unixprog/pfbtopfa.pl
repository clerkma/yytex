#! /usr/local/bin/perl

$sccsid = "@(#)pfbtopfa	1.2 10/16/94";

# A Perl script to do .pfb to .pfa font conversion.
# Written, and put into the public domain with no warranty of any kind,
# by Peter Ilieve (peter@memex.co.uk), 16 Oct 94.

# Usage:
# either as a filter, with no filename arguments
# or: pfbtopfa file file ...
# where each file, assumed to be a .pfb file, is converted into a new
# file in the current directory with .pfb replaced by .pfa

$flags = shift if ($ARGV[0] =~ /^-/);
if ($flags =~ /[^-hnv]/) {
	print STDERR "Usage: pfbtopfa [-hnv] [file file ...]
where -v is verbose about filenames and sizes,
-n does not map return to newline in the ascii input sections
and -h uses lower case in the hex output and 64 rather than 78 chars per line.
If files are given, it creates files with suffix .pfa in the current directory.
If no files are given, it acts as a filter.\n";
	exit 1;
}
$verbose = 1 if ($flags =~ /v/);
$nflag = 1 if ($flags =~ /n/);
$hflag = 1 if ($flags =~ /h/);

$bytesperline = 39; # binary bytes in one line of output
$bytesperline = 32 if ($hflag);
$hexbytesperline = $bytesperline * 2; # hex bytes to output per line

if ($#ARGV == -1) {
	&process(STDIN, STDOUT, "stdin");
}
else {
	foreach $arg (@ARGV) {
		($file = $arg) =~ s-.*/--; # strip leading path
		if ($file !~ /(.+)\.pfb/i) {
			print STDERR "\"$arg\" doesn't look like a PFB filename, skipping it.
Use pfbtopfa as a filter to process arbitrary filenames.\n";
			next;
		}
		$base = $1;
		open(IN, $arg) || die "can't open input file \"$arg\": $!\n";
		open(OUT, ">$base.pfa") || die "can't open output file \"$base.pfa\": $!\n";
		if (!&process(IN, OUT, $arg)) {
			# it failed, unlink the output file we just created
			close OUT;
			unlink "$base.pfa";
		}
	}
}

# process one file, given open filehandles to input and output
sub process {
	local($infh, $outfh, $name) = @_[0..2];
	local(@code, $len, $buf, $eof, $sectionlen, $count, $offset,
				$template, $binary, $hex);

	print STDERR "File: $name\n" if ($verbose);
	$len = 0;
	$eof = 0;
	while (!$eof) {
	$len = read($infh, $buf, 2);
		if ($len != 2) {
			print STDERR "can't read section type bytes: $!, skipping\n";
			return 0;
		}
		@code[0..1] = unpack("CC", $buf);
		if ($code[0] != 0x80) {
			printf STDERR "bad code byte 0x%x, should be 0x80, skipping\n",
					$code[0];
			return 0;
		}
		if ($code[1] == 1 || $code[1] == 2) {
			$len = read($infh, $buf, 4);
			if ($len != 4) {
				print STDERR "can't read section length bytes: $!, skipping\n";
				return 0;
			}
			$sectionlen = unpack("V", $buf);
		}
		if ($code[1] == 1) {
			# ascii section, just read and write it out, doing \r to \n.
			print STDERR "ascii section, $sectionlen bytes\n" if ($verbose);
			$count = $sectionlen;
			while ($count > 0) {
				$len = read($infh, $buf, $count);
				if (!$len) {
					print STDERR "read error in ascii section: $!, skipping\n";
					return 0;
				}
				$buf =~ s/\r/\n/g if (!$nflag);
				print $outfh $buf;
				$count -= $len;
			}
		}
		elsif ($code[1] == 2) {
			# binary section, read it all in
			print STDERR "binary section, $sectionlen bytes\n" if ($verbose);
			$count = $sectionlen;
			$binary = "";
			while ($count > 0) {
				$len = read($infh, $buf, $count);
				if (!$len) {
					print STDERR "read error in binary section: $!, skipping\n";
					return 0;
				}
				$binary .= $buf;
				$count -= $len;
			}
			# now convert and split into lines
			$count = $sectionlen;
			$offset = 0;
			$template = "H$hexbytesperline";
			while ($count > 0) {
				$hex = unpack($template, substr($binary, $offset, $bytesperline));
				$hex =~ tr/a-f/A-F/ if (!$hflag);
				print $outfh $hex, "\n";
				$count -= $bytesperline;
				$offset += $bytesperline;
			}
		}
		elsif ($code[1] == 3) {
			# eof mark, exit loop.
			$eof = 1;
		}
		else {
			printf STDERR "bad code byte %d, should be 1, 2 or 3, skipping\n",
					$code[1];
			return 0;
		}
	}
	1;
}

