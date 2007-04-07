#!/bin/csh
#
# The AMS fonts are set up in such a way that the PS FontName is 
# just the upper case version of the original file name.
#
rm -f psfonts.foo
foreach i (*.pfa)
set a=`basename $i .pfa`
set b=`basename $i .pfa | tr a-z A-Z`
mkdir $b.font
cp $a.pfa $b.font/$b
cp $a.afm $b.font/$b.afm
echo $a $b >> psfonts.foo
end
 