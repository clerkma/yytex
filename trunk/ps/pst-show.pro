%!
% This is pst-show.pro (tuned for DVIPSONE)
% Copyright 2007 TeX Users Group.
% You may freely use, modify and/or distribute this file.
%
% Patch for DVIPSONE to get PSTricks' charpath and textpath to work.
%
% Include this as a PS header / prolog using -w=.. on DVIPSONE command line,
% or via \special{header=...} in your TeX source file.
%
% We unbind show:
dvidict begin
/s {show} def
/p {currentpoint 3 -1 roll show moveto} def
/S {show dup 0 rmoveto} def
/T {show 2 index 0 rmoveto} def
end

% end of pst-show.pro
