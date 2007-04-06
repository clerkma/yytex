rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

echo %0 %1 %2 %3
if "%1" == "" %0 c: 

if not "%2" == "" goto argtwook
if exist r:\50mz %0 %1 r:
if exist f:\50mz %0 %1 f:
if exist i:\50mz %0 %1 i:
if exist e:\50mz %0 %1 e:
echo Can't find 50MZ in f:, i: or e:
goto end

:argtwook
if "%3" == "" %0 %1 %2 U

if "%3" == "U" echo NOW DO UPDATES

if "%3" == "A" echo NOW DO ADDITIONS

echo %0 %1 %2 %3
XCOPY %1\yyinstal\*.* %2\yyinstal /%3/P/D
XCOPY %1\yyinstal\doc-misc\*.* %2\yyinstal\doc-misc /%3/P/D
XCOPY %1\yyinstal\dvipsone\*.* %2\yyinstal\dvipsone /%3/P/D
XCOPY %1\yyinstal\dvipsone\debug\*.* %2\yyinstal\dvipsone\debug /%3/P/D
XCOPY %1\yyinstal\dvipsone\doc\*.* %2\yyinstal\dvipsone\doc /%3/P/D
XCOPY %1\yyinstal\dvipsone\sub\*.* %2\yyinstal\dvipsone\sub /%3/P/D
XCOPY %1\yyinstal\dviwindo\*.* %2\yyinstal\dviwindo /%3/P/D
XCOPY %1\yyinstal\dviwindo\doc\*.* %2\yyinstal\dviwindo\doc /%3/P/D
XCOPY %1\yyinstal\dviwindo\fnt\*.* %2\yyinstal\dviwindo\fnt /%3/P/D
XCOPY %1\yyinstal\fonts\*.* %2\yyinstal\fonts /%3/P/D
XCOPY %1\yyinstal\fonts\afm\*.* %2\yyinstal\fonts\afm /%3/P/D
XCOPY %1\yyinstal\fonts\encoding\*.* %2\yyinstal\fonts\encoding /%3/P/D
XCOPY %1\yyinstal\fonts\tfm\*.* %2\yyinstal\fonts\tfm /%3/P/D
XCOPY %1\yyinstal\pfe\*.* %2\yyinstal\pfe /%3/P/D
XCOPY %1\yyinstal\ps\*.* %2\yyinstal\ps /%3/P/D
XCOPY %1\yyinstal\tex\*.* %2\yyinstal\tex /%3/P/D
XCOPY %1\yyinstal\tex\base\*.* %2\yyinstal\tex\base /%3/P/D
XCOPY %1\yyinstal\tex\base\amsfonts\*.* %2\yyinstal\tex\base\amsfonts /%3/P/D
XCOPY %1\yyinstal\tex\base\amsfonts\doc\*.* %2\yyinstal\tex\base\amsfonts\doc /%3/P/D
XCOPY %1\yyinstal\tex\base\amstex\*.* %2\yyinstal\tex\base\amstex /%3/P/D
XCOPY %1\yyinstal\tex\base\amstex\contrib\*.* %2\yyinstal\tex\base\amstex\contrib /%3/P/D
XCOPY %1\yyinstal\tex\base\amstex\contrib\aexam\*.* %2\yyinstal\tex\base\amstex\contrib\aexam /%3/P/D
XCOPY %1\yyinstal\tex\base\amstex\doc\*.* %2\yyinstal\tex\base\amstex\doc /%3/P/D
XCOPY %1\yyinstal\tex\latex209\*.* %2\yyinstal\tex\latex209 /%3/P/D
XCOPY %1\yyinstal\tex\latex209\amslatex\*.* %2\yyinstal\tex\latex209\amslatex /%3/P/D
XCOPY %1\yyinstal\tex\latex209\doc\*.* %2\yyinstal\tex\latex209\doc /%3/P/D
XCOPY %1\yyinstal\tex\latex209\misc\*.* %2\yyinstal\tex\latex209\misc /%3/P/D
XCOPY %1\yyinstal\tex\latex209\slitex\*.* %2\yyinstal\tex\latex209\slitex /%3/P/D
XCOPY %1\yyinstal\tex\latex2e\*.* %2\yyinstal\tex\latex2e /%3/P/D
XCOPY %1\yyinstal\tex\latex2e\amsfonts\*.* %2\yyinstal\tex\latex2e\amsfonts /%3/P/D
XCOPY %1\yyinstal\tex\latex2e\amslatex\*.* %2\yyinstal\tex\latex2e\amslatex /%3/P/D
XCOPY %1\yyinstal\tex\latex2e\amslatex\classes\*.* %2\yyinstal\tex\latex2e\amslatex\classes /%3/P/D
XCOPY %1\yyinstal\tex\latex2e\amslatex\inputs\*.* %2\yyinstal\tex\latex2e\amslatex\inputs /%3/P/D
XCOPY %1\yyinstal\tex\latex2e\amslatex\math\*.* %2\yyinstal\tex\latex2e\amslatex\math /%3/P/D
XCOPY %1\yyinstal\tex\latex2e\babel\*.* %2\yyinstal\tex\latex2e\babel /%3/P/D
XCOPY %1\yyinstal\tex\latex2e\base\*.* %2\yyinstal\tex\latex2e\base /%3/P/D
XCOPY %1\yyinstal\tex\latex2e\eepic\*.* %2\yyinstal\tex\latex2e\eepic /%3/P/D
XCOPY %1\yyinstal\tex\latex2e\eepic\fig2eepi\*.* %2\yyinstal\tex\latex2e\eepic\fig2eepi /%3/P/D
XCOPY %1\yyinstal\tex\latex2e\epic\*.* %2\yyinstal\tex\latex2e\epic /%3/P/D
XCOPY %1\yyinstal\tex\latex2e\graphics\*.* %2\yyinstal\tex\latex2e\graphics /%3/P/D
XCOPY %1\yyinstal\tex\latex2e\hyperref\*.* %2\yyinstal\tex\latex2e\hyperref /%3/P/D
XCOPY %1\yyinstal\tex\latex2e\misc\*.* %2\yyinstal\tex\latex2e\misc /%3/P/D
XCOPY %1\yyinstal\tex\latex2e\psfrag\*.* %2\yyinstal\tex\latex2e\psfrag /%3/P/D
XCOPY %1\yyinstal\tex\latex2e\psnfss\*.* %2\yyinstal\tex\latex2e\psnfss /%3/P/D
XCOPY %1\yyinstal\tex\latex2e\revtex\*.* %2\yyinstal\tex\latex2e\revtex /%3/P/D
XCOPY %1\yyinstal\tex\latex2e\seminar\*.* %2\yyinstal\tex\latex2e\seminar /%3/P/D
XCOPY %1\yyinstal\tex\latex2e\psnfss\em\*.* %2\yyinstal\tex\latex2e\psnfss\em /%3/P/D
XCOPY %1\yyinstal\tex\latex2e\psnfss\ly1\*.* %2\yyinstal\tex\latex2e\psnfss\ly1 /%3/P/D
XCOPY %1\yyinstal\tex\latex2e\psnfss\yyadd\*.* %2\yyinstal\tex\latex2e\psnfss\yyadd /%3/P/D
XCOPY %1\yyinstal\tex\latex2e\tools\*.* %2\yyinstal\tex\latex2e\tools /%3/P/D
XCOPY %1\yyinstal\texfmt\*.* %2\yyinstal\texfmt /%3/P/D
XCOPY %1\yyinstal\texfmt\latex209\*.* %2\yyinstal\texfmt\latex209 /%3/P/D
XCOPY %1\yyinstal\texfmt\latex2e\*.* %2\yyinstal\texfmt\latex2e /%3/P/D
XCOPY %1\yyinstal\texinput\*.* %2\yyinstal\texinput /%3/P/D
XCOPY %1\yyinstal\texinput\mt\*.* %2\yyinstal\texinput\mt /%3/P/D
XCOPY %1\yyinstal\util\*.* %2\yyinstal\util /%3/P/D
XCOPY %1\yyinstal\yandytex\*.* %2\yyinstal\yandytex /%3/P/D
XCOPY %1\yyinstal\yandytex\epsilon\*.* %2\yyinstal\yandytex\epsilon /%3/P/D
XCOPY %1\yyinstal\yandytex\fmt\*.* %2\yyinstal\yandytex\fmt /%3/P/D
XCOPY %1\yyinstal\yandytex\keyboard\*.* %2\yyinstal\yandytex\keyboard /%3/P/D

if "%3" == "A" goto end

if "%3" == "U" %0 %1 %2 A

:end
echo ALL DONE!
