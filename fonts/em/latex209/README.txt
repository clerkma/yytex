Copyright 2007 TeX Users Group.
You may freely use, modify and/or distribute this file.

You can use the EM fonts with an existing LaTeX 2.09 format.
Just add \input emlatex to your source file between the
\documentstyle{...} and the \begin{document}.

In this directory you will find lplain-e.tex and lfonts-e.tex used
for making a new LaTeX 2.09 format which refers to the EM font set.

For this you also need latex.ltx in order to make a new LaTeX 2.09 format.
You should be able to get this from your existing LaTeX 2.09 installation.

Then run ini-TeX on lplain-e.tex to create a lplain-e.fmt file.
