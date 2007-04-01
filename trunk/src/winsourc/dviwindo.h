/* Copyright 1991, 1992 Y&Y, Inc.
   Copyright 2007 TeX Users Group

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

#define ID_SPIN			421
#define ID_REGULAR		451
#define ID_BOLD   		423
#define ID_ITALIC 		422
#define ID_BOLDITALIC	452
#define ID_TYPEFACE   	420
#define ID_SPINVALUE	424
#define ID_SPINTEXT		426
#define ID_SHOWBOXES	425
#define ID_ENCODED 		427
#define ID_ENCONAME		428
#define ID_DISMISS		435
#define ID_WRAP			436
#define ID_ABOUT		437

#define ID_TYPE1		470
#define ID_TRUETYPE		471
#define ID_OTHER		472
#define ID_DONTSTOP		473
#define ID_SYMBOL		474
#define ID_FILENAME		475
#define ID_ENCODING		476
#define ID_TEXTFONT		477
#define ID_NEWONLY		478
#define ID_USEFONTNAME	479
#define ID_USEFILENAME	480
#define ID_NAMEGROUP	481

#define IDC_EDIT  		401
#define IDC_FILES  		402
#define IDC_PATH  		403
#define IDC_LISTBOX   	404
#define IDC_DIRECTORY 	405
#define IDC_FONTLIST	406
#define IDC_FILENAME  	407
#define IDC_DESTINATION	408
#define IDC_DEVICE		411
#define IDC_SOURCE		409
#define IDC_USEDLIST	410

#define IPC_EDIT		415
#define IPC_OLDTEXT		416
#define IPC_VARIABLE	417
#define IPC_VALUE		418

#define IPS_EDIT		430
#define IPS_CASE		431
#define IPS_WRAP		432
#define IPS_FILE		433
#define IPS_COMMENT		434
#define IPS_QUESTION	438
#define IPS_HELP		439
#define IPS_PROMPT		452

#define IPR_FROM		440
#define IPR_TO			441
#define IPR_COPIES		442
#define IPR_USEPSONE	443
#define IPR_RUNMIN		444
#define IPR_PRINTSETUP  445
#define IPR_PORTSPEC    446
#define IPR_REVERSE     447
#define IPR_ALTERNATE   448
#define IPR_ODDONLY     449
#define IPR_EVENONLY    450
#define IPR_PRINTFILE	451

#define IAI_KEY			460
#define IAI_VALUE		461
#define IAI_DELETE		462
#define IAI_REPLACE		463
#define IAI_SEPARATOR	464
#define IAI_ADD			465
#define IAI_NEXT		466

#define IDM_ABOUT 		103
#define IDM_BORDER		111
#define IDM_CHARWIDTHS	125
#define IDM_CLOSE 		102
#define IDM_COLORFONT 	132
#define IDM_COUNTZERO 	127
#define IDM_DRAWVISIBLE	118
#define IDM_DVITYPE   	154
#define IDM_EXIT  		104
#define IDM_HELP  		131
#define IDM_ESCAPE		134
#define IDM_SPACE		138
#define IDM_BACKSPACE	139
#define IDM_DELETE		195
#define IDM_JUMPBACK	204
#define IDM_REVERT		205

#define IDM_NEWPAGESCALE 136
#define IDM_OPEN  		101
#define IDM_SOURCEFILE	105
#define IDM_SOURCELINE	107
#define IDM_PRESPACING	122
#define IDM_PRINTALL	168
#define IDM_PRINTVIEW	123
#define IDM_PRINTSETUP	106
#define IDM_RESETSCALE	116
#define IDM_RESETPAGE	137
#define IDM_RULEFILL  	120
#define IDM_SEARCH		133
#define IDM_SELECTFONT	129
#define IDM_SELECTPAGE	126
#define IDM_SHOWHXW   	117
#define IDM_SHOWINFO  	109
#define IDM_SHOWSCALE 	124
#define IDM_SHOWMETRICS	135
#define IDM_SNAPTO		128
#define IDM_TEXTOUT   	119
#define IDM_TRACEFLAG 	121
#define IDM_SPREAD		140
#define IDM_HOME  		142
#define IDM_END   		143
#define IDM_SCROLLLEFT	144
#define IDM_SCROLLRIGHT	145
#define IDM_SCROLLUP  	146
#define IDM_SCROLLDOWN	147
#define IDM_SAVEPREFER	149
#define IDM_PREVIOUS  	150
#define IDM_NEXT  		151
#define IDM_UNMAGNIFY 	152
#define IDM_MAGNIFY   	153
#define IDM_DEFAULT   	155
#define IDM_RESEARCH  	156
#define IDM_PRINTFRAME	157
#define IDM_LANDSCAPE	158
#define IDM_GREYTEXT	159
#define IDM_GREYPLUS	160
#define IDM_USER		161
#define IDM_DATE		162
#define IDM_VERSION		163
#define IDM_FONTSUSED	164
#define IDM_ZOOM		165
#define IDM_OLDVIEW		166
#define IDM_COMPLAINMISSING	167
#define IDM_COMPLAINSPECIAL	169
#define IDM_COMPLAINENCODING 190
#define IDM_COMPLAINFILES	189
#define IDM_IGNORESPECIAL   171
#define IDM_BOTTOM		170
#define IDM_TOP			174 
#define IDM_FILESPECS	172
#define IDM_STACKUSED	173
#define IDM_PRINTSPECS	175
#define IDM_WRITEAFM	176
#define IDM_WRITETFM	188
#define IDM_WRITEAFMS	201
#define IDM_WRITETFMS	202
#define IDM_DEBUGSTRING	177
#define IDM_PASSTHROUGH 178
#define IDM_SHOWPREVIEW 186
#define IDM_CLIPBOARD	179
#define IDM_MAXDRIFT	180
#define IDM_STRINGLIMIT 181
#define IDM_TAGGEDCHAR  182
#define IDM_SHOWBUTTONS	183
#define IDM_TRUEINCH	184
#define IDM_TESTFLAG    187
#define IDM_READPREFER	191
#define IDM_SHOWCALLS	206
#define IDM_PAUSECALLS	207
#define IDM_FACTORY		192
#define IDM_OUTLINERULE 193
#define IDM_IGNORESELECT 194
#define IDM_SYSTEMINFO  196
#define IDM_VIEWEXTRAS	197
#define IDM_COPYRIGHT	198
#define IDM_SAVENOW		199
#define IDM_USEWORKDIR  200
#define IDM_SHOWCOUNTER 203
#define IDM_MAGNIFICATION 231
#define IDM_SHOWVIEWPORTS 232
#define IDM_CURRENTPAGE 208
#define IDM_SYSFLAGS	209
#define IDM_SHOWTIFF	214
#define IDM_SHOWWMF		215
#define IDM_CALLBACKPASS 235
#define IDM_CONSOLEOPEN 236
#define IDM_OPENCLOSECHANNEL 237
#define IDM_OLDPASSTHROUGH 238
#define IDM_NEWPASSTHROUGH 239
#define IDM_FORCEPASSBACK 240
#define IDM_DONTASKFILE 241
#define IDM_FORCEOLDPASSTHROUGH 242
#define IDM_USEDLLS		243
#define IDM_OPENCLOSEPRINTER	244
#define IDM_BACK		245
#define IDM_FORWARD		246

#define IDM_DUPLEX		210
#define IDM_SIMPLEX		211
#define IDM_VERTICAL	212
#define IDM_HORIZONTAL	213

#define IDM_TEXFLAGS	216
#define IDM_DVIFLAGS	217
#define IDM_DVIDISFLAGS 219
#define IDM_TFMFLAGS	218

#define IDM_TEXNANSI	221
#define IDM_TEX256		222
#define IDM_ANSINEW		223
#define IDM_STANDARD	224
#define IDM_TEXTEXT		225
#define IDM_CUSTOMENCODING		226

#define IDM_PGDN		227
#define IDM_PGUP		228
#define IDM_INSERT		229
#define IDM_CTRLP		230

#define IDM_FIRSTPAGE	233
#define IDM_LASTPAGE	234

#define IDM_CUSTOMSIZE	(550 + 0)
#define IDM_LETTER		(550 + 1)
#define IDM_LETTERSMALL		(550 + 2)
#define IDM_TABLOID		(550 + 3)
#define IDM_LEDGER		(550 + 4)
#define IDM_LEGAL 		(550 + 5)
#define IDM_STATEMENT	(550 + 6)
#define IDM_EXECUTIVE	(550 + 7)
#define IDM_ATHREE 		(550 + 8)
#define IDM_AFOUR 		(550 + 9)
#define IDM_AFOURSMALL	(550 + 10)
#define IDM_AFIVE 		(550 + 11)
#define IDM_BFOUR 		(550 + 12)
#define IDM_BFIVE 		(550 + 13)
#define IDM_FOLIO 		(550 + 14) 
#define IDM_QUARTO 		(550 + 15)

#define IDM_SCALEDPOINT	(600 + 0)
#define IDM_POINT		(600 + 1)
#define IDM_BIGPOINT	(600 + 2)
#define IDM_DIDOT		(600 + 3)
#define IDM_MILLIMETER	(600 + 4)
#define IDM_PICA		(600 + 5)
#define IDM_CICERO		(600 + 6)
#define IDM_CENTIMETER	(600 + 7)
#define IDM_INCH		(600 + 8)
#define IDM_PICAPOINT	(600 + 9)

#define IDM_APPLICATION 1000
#define IDM_DOCUMENTS   1200
#define IDM_ENVIRONMENT 1300

#define IDMB_BYTES		509
#define IDMB_DEN		502
#define IDMB_FONTS  	508
#define IDMB_HPLUSD 	504
#define IDMB_MAG		503
#define IDMB_NUM		501
#define IDMB_PAGES  	507
#define IDMB_STACK  	506
#define IDMB_VERSION	510
#define IDMB_WIDTH  	505
#define IDMB_FILE		514

#define IDR_WIDTH		520
#define IDR_HEIGHT		521

#define IDD_TEXT1   	511
#define IDD_SETUP   	512
#define IDD_PRINTERS	513

#define ICN_LISTBOX		525
#define ICN_COPY		526
#define ICN_RESET		527
#define ICN_ADDTEXT		528
#define ICN_SETTITLE	529
#define ICN_DONE		530
#define ICN_CLEAR		531
#define ICN_GETTEXT		532

#define IDS_ERROR		20000

#define PRN_ERROR		21000

#define EXE_ERROR		22000

#define EPS_POSTAMBLE	23000
#define EPS_PREAMBLE1	23001
#define EPS_PREAMBLE2	23002
#define EPS_MAGIC		23003

#define IDS_FILTERSTRING 23010
#define IDS_DVIFILTERSTR 23011

#define ENC_ERROR		 23020
#define T1INSTALL_ERR	 23021
#define W95INSTALL_ERR	 23022
#define TTSETUP_ERR		 23023
#define BAD_STYLE		 23024
#define DVI_ERR			 23025
