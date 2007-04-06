@echo off
rem Copyright 2007 TeX Users Group.
rem You may freely use, modify and/or distribute this file.

cls
rem FILE: UPDTFAQ.BAT
rem Dependencies: (i) Assumed to be on PATH, (ii) Copy of xtrans.exe assumed to
rem to be on PATH, (iii) translation table faqprep.x resides in "\internal"
rem subdirectory of local Web directory image, (iv) also in "\internal" are
rem files faq-header and faq-footer, plus all-text versions of faq lists,
rem (v) see also syntax, below.
rem
rem This batch file converts a modified "all-text" version of a 
rem faq list to HTML format. Requirements:
rem
rem (i) All paragraphs delimited by double carriage-return/line-feeds.
rem (Important: Exactly two; no more, no less!)
rem
rem (ii) Each FAQ item starts with a paragraph beginning with "Q:" and
rem has a second paragraph beginning with "A:".
rem
rem (iii) Any internal effects (ie, bold for emphasis within a paragraph)
rem coded manually (except for bold "Q:" and "A:", which are done
rem automatically).
rem
rem (iv) All constituent files (see below) reside in a subdirectory "internal"
rem of the local image of the Web directory.
rem
rem Syntax is
rem 	updtfaq  faq-location list-choice
rem                     %1        %2 
rem where faq-location is the fully qualified name of the local Web directory
rem (eg d:\www) and list-choice (currently) is "fonts" or "sys". 
rem The "all-text" files to modify are "faqfonts.txt" and "faqsys.txt, and
rem reside in faq-location\internal. (These files must have 8.3 names;
rem long names are OK elsewhere).
rem
rem
rem Help display
if %1==? goto help
rem Syntax check
if /%1==/ goto help
if /%2==/ goto help
set location=%1
set source=faq%2.txt
set dest=faq-%2.htm
if not exist "%location%\NUL" goto error
if not exist "%location%\internal\%source%" goto error
if not exist "%location%\%dest%" goto error
if exist "%location%\internal\html.tmp" del "%location%\internal\html.tmp"
rem
rem Make backup copies of the target faq list
rem
if exist "%location%\%dest%-bak" del "%location%\%dest%-bak"
copy "%location%\%dest%" "%location%\%dest%-bak"
del "%location%\%dest%"
rem
rem Run the source file through exchange that codes it for HTML.
rem
xtrans %location%\internal\faqprep.x %location%\internal\%source% %location%\internal\html.tmp
rem
rem Now append headers and footers to the HTML file.
rem
copy /b "%location%\internal\faq-header" + "%location%\internal\html.tmp" + "%location%\internal\faq-footer" "%location%\%dest%"
del "%location%\internal\html.tmp"
cls
echo FAQ file %location%\%dest% is ready for uploading
echo.
pause
goto end
:help
cls
echo Syntax: 
echo. 
echo 	updtfaq  faq-location list-choice
echo                     %1        %2 
echo where faq-location is the fully qualified name of the local Web directory
echo (eg d:\www) and list-choice (currently) is "fonts" or "sys". 
echo The "all-text" files to modify are "faqfonts.txt" and "faqsys.txt, and
echo reside in faq-location\internal. (These files must have 8.3 names;
echo long names are OK elsewhere).
echo.
pause
goto end
:error
cls
echo File(s) and/or directory not present.
echo.
pause
goto end
:end
