@echo off
rem
rem make local version of lucida stuff
rem assuming we want tex text modified encoding
rem
copy c:\lucidabr\lbr.tf3 c:\pctex\textfms\lbrx.tfm
copy c:\lucidabr\lbr.pf3 c:\psfonts\pfm\lbrx.pfm
copy c:\lucidabr\lbr.pf3 d:\pfm\lbrx.pfm
reencode -c=texannew c:\psfonts\lbr.pfb
copy lbrx.pfb c:\psfonts
del lbrx.pfb
rem
copy c:\lucidabr\lbd.tf3 c:\pctex\textfms\lbdx.tfm
copy c:\lucidabr\lbd.pf3 c:\psfonts\pfm\lbdx.pfm
copy c:\lucidabr\lbd.pf3 d:\pfm\lbdx.pfm
reencode -c=texannew c:\psfonts\lbd.pfb
copy lbdx.pfb c:\psfonts
del lbdx.pfb
rem
copy c:\lucidabr\lbi.tf3 c:\pctex\textfms\lbix.tfm
copy c:\lucidabr\lbi.pf3 c:\psfonts\pfm\lbix.pfm
copy c:\lucidabr\lbi.pf3 d:\pfm\lbix.pfm
reencode -c=texannew c:\psfonts\lbi.pfb
copy lbix.pfb c:\psfonts
del lbix.pfb
rem
copy c:\lucidabr\lbdi.tf3 c:\pctex\textfms\lbdix.tfm
copy c:\lucidabr\lbdi.pf3 c:\psfonts\pfm\lbdix.pfm
copy c:\lucidabr\lbdi.pf3 d:\pfm\lbdix.pfm
reencode -c=texannew c:\psfonts\lbdi.pfb
copy lbdix.pfb c:\psfonts
del lbdix.pfb
rem
copy c:\lucidabr\lsr.tf3 c:\pctex\textfms\lsrx.tfm
copy c:\lucidabr\lsr.pf3 c:\psfonts\pfm\lsrx.pfm
copy c:\lucidabr\lsr.pf3 d:\pfm\lsrx.pfm
reencode -c=texannew c:\psfonts\lsr.pfb
copy lsrx.pfb c:\psfonts
del lsrx.pfb
rem
copy c:\lucidabr\lsd.tf3 c:\pctex\textfms\lsdx.tfm
copy c:\lucidabr\lsd.pf3 c:\psfonts\pfm\lsdx.pfm
copy c:\lucidabr\lsd.pf3 d:\pfm\lsdx.pfm
reencode -c=texannew c:\psfonts\lsd.pfb
copy lsdx.pfb c:\psfonts
del lsdx.pfb
rem
copy c:\lucidabr\lsi.tf3 c:\pctex\textfms\lsix.tfm
copy c:\lucidabr\lsi.pf3 c:\psfonts\pfm\lsix.pfm
copy c:\lucidabr\lsi.pf3 d:\pfm\lsix.pfm
reencode -c=texannew c:\psfonts\lsi.pfb
copy lsix.pfb c:\psfonts
del lsix.pfb
rem
copy c:\lucidabr\lsdi.tf3 c:\pctex\textfms\lsdix.tfm
copy c:\lucidabr\lsdi.pf3 c:\psfonts\pfm\lsdix.pfm
copy c:\lucidabr\lsdi.pf3 d:\pfm\lsdix.pfm
reencode -c=texannew c:\psfonts\lsdi.pfb
copy lsdix.pfb c:\psfonts
del lsdix.pfb
rem
copy c:\lucidabr\lstr.tf3 c:\pctex\textfms\lstrx.tfm
copy c:\lucidabr\lstr.pf3 c:\psfonts\pfm\lstrx.pfm
copy c:\lucidabr\lstr.pf3 d:\pfm\lstrx.pfm
reencode -c=texannew c:\psfonts\lstr.pfb
copy lstrx.pfb c:\psfonts
del lstrx.pfb
rem
copy c:\lucidabr\lstb.tf3 c:\pctex\textfms\lstbx.tfm
copy c:\lucidabr\lstb.pf3 c:\psfonts\pfm\lstbx.pfm
copy c:\lucidabr\lstb.pf3 d:\pfm\lstbx.pfm
reencode -c=texannew c:\psfonts\lstb.pfb
copy lstbx.pfb c:\psfonts
del lstbx.pfb
rem
copy c:\lucidabr\lbsl.tf3 c:\pctex\textfms\lbslx.tfm
copy c:\lucidabr\lbsl.pf3 c:\psfonts\pfm\lbslx.pfm
copy c:\lucidabr\lbsl.pf3 d:\pfm\lbslx.pfm
reencode -c=texannew c:\psfonts\lbsl.pfb
copy lbslx.pfb c:\psfonts
del lbslx.pfb
rem
copy c:\lucidabr\lbc.tf3 c:\pctex\textfms\lbcx.tfm
copy c:\lucidabr\lbc.pf3 c:\psfonts\pfm\lbcx.pfm
copy c:\lucidabr\lbc.pf3 d:\pfm\lbcx.pfm
reencode -c=texannew c:\psfonts\lbc.pfb
copy lbcx.pfb c:\psfonts
del lbcx.pfb
rem
rem math fonts should be OK as is
rem
@echo on