/* Copyright 2007 TeX Users Group

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301 USA.  */


	total = 0; 
	s = ?;
	while(;;) {
		if ((n = getc(input)) < 128) {
			for(k=0; k < n+1; k++) *s++ = getc(input); 
			total += n+1;
		}
		else if (n > 128)
			c = getc(input);
			for(k=0; k < (257 - n); k++) *s++ = c; 			
			total += (257 - n);
		}
		if (total >= bytesperrow) break;
	}
	
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** 

		Appendix C: Data Compression - Scheme 32773 -  "PackBits"
          
          
          Abstract
          
          This document  describes a  simple compression scheme for bilevel
          scanned and paint type files.
          
          
          Motivation
          
          The TIFF  specification defines  a number of compression schemes.
          Compression type  1 is  really no  compression, other  than basic
          pixel  packing.     Compression   type  2,   based  on  CCITT  1D
          compression,  is   powerful,  but   not  trivial   to  implement.
          Compression type  5 is  typically very effective for most bilevel
          images, as  well as  many deeper images such as palette color and
          grayscale images, but is also not trivial to implement.  PackBits
          is a simple but often effective alternative.
          
          
          Description
          
          Several good schemes were already in use in various settings.  We
          somewhat arbitrarily picked the Macintosh PackBits scheme.  It is
          byte oriented,  so there  is no problem with word alignment.  And
          it has a good worst case behavior (at most 1 extra byte for every
          128 input  bytes).    For  Macintosh  users,  there  are  toolbox
          utilities PackBits  and UnPackBits that will do the work for you,
          but it is easy to implement your own routines.
          
          A pseudo code fragment to unpack might look like this:
          
          Loop  until  you  get  the  number  of  unpacked  bytes  you  are
          expecting:
               Read the next source byte into n.
               If n is between 0 and 127 inclusive, copy the next n+1 bytes
          literally.
               Else if  n is  between -127  and -1 inclusive, copy the next
          byte -n+1 times.
               Else if n is 128, noop.
          Endloop
          
          In the  inverse routine,  it's best to encode a 2-byte repeat run
          as a replicate run except when preceded and followed by a literal
          run, in  which case it's best to merge the three into one literal
          run.  Always encode 3-byte repeats as replicate runs.
          
          So that's the algorithm.  Here are some other rules:
          
          o    Each row  must be packed separately.  Do not compress across
          row boundaries.

          o    The number  of uncompressed  bytes per  row is defined to be
          (ImageWidth +  7) / 8.  If the uncompressed bitmap is required to
          have an  even number  of bytes  per row,  decompress  into  word-
          aligned buffers.
          o    If a  run is  larger  than  128  bytes,  simply  encode  the
          remainder of the run as one or more additional replicate runs.
          
          When  PackBits   data  is  uncompressed,  the  result  should  be
          interpreted as per compression type 1 (no compression).
          
*** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
