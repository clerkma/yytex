rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

c:\lint\lint  +v  -ic:\lint\  std.lnt  %1 %2 %3 %4 %5 %6 %7 %8 %9 >_lint.tmp
type _lint.tmp | more
@echo off
echo ---
echo PC-lint for C output placed in _LINT.TMP
