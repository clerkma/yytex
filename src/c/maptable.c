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

STATIC const UNICODE_MAP unicodeMap[] =
{
	//NOTE:  * indicates that this line differs from the unicodeMap from Sairus.
	{ "A",                    0x0041 },
	{ "AE",                   0x00C6 },
	{ "AEsmall",              0xF8E6 },
	{ "Aacute",               0x00C1 },
	{ "Aacutesmall",          0xF8E1 },
	{ "Abreve",               0x0102 },
	{ "Acircumflex",          0x00C2 },
	{ "Acircumflexsmall",     0xF8E2 },
	{ "Acute",                0xF7B0 },
	{ "Acutesmall",           0xF8B4 },
	{ "Adieresis",            0x00C4 },
	{ "Adieresissmall",       0xF8E4 },
	{ "Agrave",               0x00C0 },
	{ "Agravesmall",          0xF8E0 },
	{ "Alpha",                0x0391 },
	{ "Alphatonos",           0x0386 },
	{ "Amacron",              0x0100 },
	{ "Aogonek",              0x0104 },
	{ "Aring",                0x00C5 },
	{ "Aringsmall",           0xF8E5 },
	{ "Asmall",               0xF861 },
	{ "Atilde",               0x00C3 },
	{ "Atildesmall",          0xF8E3 },
	{ "B",                    0x0042 },
	{ "Beta",                 0x0392 },
	{ "Brevesmall",           0xF7F4 },
	{ "Bsmall",               0xF862 },
	{ "C",                    0x0043 },
	{ "Cacute",               0x0106 },
	{ "Caron",                0xF7B1 },
	{ "Caronsmall",           0xF7F5 },
	{ "Ccaron",               0x010C },
	{ "Ccedilla",             0x00C7 },
	{ "Ccedillasmall",        0xF8E7 },
	{ "Ccircumflex",          0x0108 },
	{ "Cdotaccent",           0x010A },
	{ "Cedillasmall",         0xF8B8 },
	{ "Chi",                  0x03A7 },
	{ "Circumflexsmall",      0xF7F6 },
	{ "Csmall",               0xF863 },
	{ "D",                    0x0044 },
	{ "Dcaron",               0x010E },
	{ "Dcroat",               0x0110 },
	{ "Delta",                0x2206 },
	{ "Delta_",               0x0394 },		//*
	{ "Dieresis",             0xF7B2 },
	{ "DieresisAcute",        0xF7B3 },
	{ "DieresisGrave",        0xF7B4 },
	{ "Dieresissmall",        0xF8A8 },
	{ "Dotaccentsmall",       0xF7F7 },
	{ "Dsmall",               0xF864 },
	{ "E",                    0x0045 },
	{ "Eacute",               0x00C9 },
	{ "Eacutesmall",          0xF8E9 },
	{ "Ecaron",               0x011A },
	{ "Ecircumflex",          0x00CA },
	{ "Ecircumflexsmall",     0xF8EA },
	{ "Edieresis",            0x00CB },
	{ "Edieresissmall",       0xF8EB },
	{ "Edotaccent",           0x0116 },
	{ "Egrave",               0x00C8 },
	{ "Egravesmall",          0xF8E8 },
	{ "Emacron",              0x0112 },
	{ "Eng",                  0x014A },
	{ "Eogonek",              0x0118 },
	{ "Epsilon",              0x0395 },
	{ "Epsilontonos",         0x0388 },
	{ "Esmall",               0xF865 },
	{ "Eta",                  0x0397 },
	{ "Etatonos",             0x0389 },
	{ "Eth",                  0x00D0 },
	{ "Ethsmall",             0xF8F0 },
	{ "F",                    0x0046 },
	{ "Fsmall",               0xF866 },
	{ "G",                    0x0047 },
	{ "Gamma",                0x0393 },
	{ "Gbreve",               0x011E },
	{ "Gcaron",               0x01E6 },
	{ "Gcircumflex",          0x011C },
	{ "Gcommaaccent",         0x0122 },
	{ "Gdotaccent",           0x0120 },
	{ "Grave",                0xF7B5 },
	{ "Gravesmall",           0xF860 },
	{ "Gsmall",               0xF867 },
	{ "H",                    0x0048 },
	{ "Hbar",                 0x0126 },
	{ "Hcircumflex",          0x0124 },
	{ "Hsmall",               0xF868 },
	{ "Hungarumlaut",         0xF7B6 },
	{ "Hungarumlautsmall",    0xF7F8 },
	{ "I",                    0x0049 },
	{ "IJ",                   0x0132 },
	{ "Iacute",               0x00CD },
	{ "Iacutesmall",          0xF8ED },
	{ "Icircumflex",          0x00CE },
	{ "Icircumflexsmall",     0xF8EE },
	{ "Idieresis",            0x00CF },
	{ "Idieresissmall",       0xF8EF },
	{ "Idotaccent",           0x0130 },
	{ "Ifraktur",             0x2111 },
	{ "Igrave",               0x00CC },
	{ "Igravesmall",          0xF8EC },
	{ "Imacron",              0x012A },
	{ "Iogonek",              0x012E },
	{ "Iota",                 0x0399 },
	{ "Iotadieresis",         0x03AA },
	{ "Iotatonos",            0x038A },
	{ "Ismall",               0xF869 },
	{ "Itilde",               0x0128 },
	{ "J",                    0x004A },
	{ "Jcircumflex",          0x0134 },
	{ "Jsmall",               0xF86A },
	{ "K",                    0x004B },
	{ "Kappa",                0x039A },
	{ "Kcommaaccent",         0x0136 },
	{ "Ksmall",               0xF86B },
	{ "L",                    0x004C },
	{ "LL",                   0xF7A6 },
	{ "Lacute",               0x0139 },
	{ "Lambda",               0x039B },
	{ "Lcaron",               0x013D },
	{ "Lcommaaccent",         0x013B },
	{ "Ldot",                 0x013F },
	{ "Lslash",               0x0141 },
	{ "Lslashsmall",          0xF7F9 },
	{ "Lsmall",               0xF86C },
	{ "M",                    0x004D },
	{ "Macron",               0xF7B7 },
	{ "Macronsmall",          0xF8AF },
	{ "Msmall",               0xF86D },
	{ "Mu",                   0x039C },
	{ "N",                    0x004E },
	{ "Nacute",               0x0143 },
	{ "Ncaron",               0x0147 },
	{ "Ncommaaccent",         0x0145 },
	{ "Nsmall",               0xF86E },
	{ "Ntilde",               0x00D1 },
	{ "Ntildesmall",          0xF8F1 },
	{ "Nu",                   0x039D },
	{ "O",                    0x004F },
	{ "OE",                   0x0152 },
	{ "OEsmall",              0xF7FA },
	{ "Oacute",               0x00D3 },
	{ "Oacutesmall",          0xF8F3 },
	{ "Ocircumflex",          0x00D4 },
	{ "Ocircumflexsmall",     0xF8F4 },
	{ "Odieresis",            0x00D6 },
	{ "Odieresissmall",       0xF8F6 },
	{ "Ogoneksmall",          0xF7FB },
	{ "Ograve",               0x00D2 },
	{ "Ogravesmall",          0xF8F2 },
	{ "Ohorn",                0x01A0 },
	{ "Ohungarumlaut",        0x0150 },
	{ "Omacron",              0x014C },
	{ "Omega",                0x2126 },
	{ "Omega_",               0x03a9 },		//*
	{ "Omegatonos",           0x038F },
	{ "Omicron",              0x039F },
	{ "Omicrontonos",         0x038C },
	{ "Oslash",               0x00D8 },
	{ "Oslashsmall",          0xF8F8 },
	{ "Osmall",               0xF86F },
	{ "Otilde",               0x00D5 },
	{ "Otildesmall",          0xF8F5 },
	{ "P",                    0x0050 },
	{ "Phi",                  0x03A6 },
	{ "Pi",                   0x03A0 },
	{ "Psi",                  0x03A8 },
	{ "Psmall",               0xF870 },
	{ "Pts",                  0x20A7 },
	{ "Q",                    0x0051 },
	{ "Qsmall",               0xF871 },
	{ "R",                    0x0052 },
	{ "Racute",               0x0154 },
	{ "Rcaron",               0x0158 },
	{ "Rcommaaccent",         0x0156 },
	{ "Rfraktur",             0x211C },
	{ "Rho",                  0x03A1 },
	{ "Ringsmall",            0xF7FC },
	{ "Rsmall",               0xF872 },
	{ "S",                    0x0053 },
	{ "Sacute",               0x015A },
	{ "Scaron",               0x0160 },
	{ "Scaronsmall",          0xF7FD },
	{ "Scedilla",             0xF7A8 },
	{ "Scircumflex",          0x015C },
	{ "Scommaaccent",         0x015E },
	{ "Sigma",                0x03A3 },
	{ "Ssmall",               0xF873 },
	{ "T",                    0x0054 },
	{ "Tau",                  0x03A4 },
	{ "Tbar",                 0x0166 },
	{ "Tcaron",               0x0164 },
	{ "Tcommaaccent",         0x0162 },
	{ "Theta",                0x0398 },
	{ "Thorn",                0x00DE },
	{ "Thornsmall",           0xF8FE },
	{ "Tildesmall",           0xF7FE },
	{ "Tsmall",               0xF874 },
	{ "U",                    0x0055 },
	{ "Uacute",               0x00DA },
	{ "Uacutesmall",          0xF8FA },
	{ "Ubreve",               0x016C },
	{ "Ucircumflex",          0x00DB },
	{ "Ucircumflexsmall",     0xF8FB },
	{ "Udieresis",            0x00DC },
	{ "Udieresissmall",       0xF8FC },
	{ "Ugrave",               0x00D9 },
	{ "Ugravesmall",          0xF8F9 },
	{ "Uhorn",                0x01AF },
	{ "Uhungarumlaut",        0x0170 },
	{ "Umacron",              0x016A },
	{ "Uogonek",              0x0172 },
	{ "Upsilon",              0x03A5 },
	{ "Upsilon1",             0x03D2 },
	{ "Upsilondieresis",      0x03AB },
	{ "Upsilontonos",         0x038E },
	{ "Uring",                0x016E },
	{ "Usmall",               0xF875 },
	{ "Utilde",               0x0168 },
	{ "V",                    0x0056 },
	{ "Vsmall",               0xF876 },
	{ "W",                    0x0057 },
	{ "Wsmall",               0xF877 },
	{ "X",                    0x0058 },
	{ "Xi",                   0x039E },
	{ "Xsmall",               0xF878 },
	{ "Y",                    0x0059 },
	{ "Yacute",               0x00DD },
	{ "Yacutesmall",          0xF8FD },
	{ "Ydieresis",            0x0178 },
	{ "Ydieresissmall",       0xF8FF },
	{ "Ysmall",               0xF879 },
	{ "Z",                    0x005A },
	{ "Zacute",               0x0179 },
	{ "Zcaron",               0x017D },
	{ "Zcaronsmall",          0xF7FF },
	{ "Zdotaccent",           0x017B },
	{ "Zeta",                 0x0396 },
	{ "Zsmall",               0xF87A },
	{ "a",                    0x0061 },
	{ "aacute",               0x00E1 },
	{ "abreve",               0x0103 },
	{ "acircumflex",          0x00E2 },
	{ "acute",                0x00B4 },
	{ "acutecomb",            0x0301 },
	{ "adieresis",            0x00E4 },
	{ "ae",                   0x00E6 },
	{ "afii00208",            0x2015 },
	{ "afii10017",            0x0410 },
	{ "afii10018",            0x0411 },
	{ "afii10019",            0x0412 },
	{ "afii10020",            0x0413 },
	{ "afii10021",            0x0414 },
	{ "afii10022",            0x0415 },
	{ "afii10023",            0x0401 },
	{ "afii10024",            0x0416 },
	{ "afii10025",            0x0417 },
	{ "afii10026",            0x0418 },
	{ "afii10027",            0x0419 },
	{ "afii10028",            0x041A },
	{ "afii10029",            0x041B },
	{ "afii10030",            0x041C },
	{ "afii10031",            0x041D },
	{ "afii10032",            0x041E },
	{ "afii10033",            0x041F },
	{ "afii10034",            0x0420 },
	{ "afii10035",            0x0421 },
	{ "afii10036",            0x0422 },
	{ "afii10037",            0x0423 },
	{ "afii10038",            0x0424 },
	{ "afii10039",            0x0425 },
	{ "afii10040",            0x0426 },
	{ "afii10041",            0x0427 },
	{ "afii10042",            0x0428 },
	{ "afii10043",            0x0429 },
	{ "afii10044",            0x042A },
	{ "afii10045",            0x042B },
	{ "afii10046",            0x042C },
	{ "afii10047",            0x042D },
	{ "afii10048",            0x042E },
	{ "afii10049",            0x042F },
	{ "afii10050",            0x0490 },
	{ "afii10051",            0x0402 },
	{ "afii10052",            0x0403 },
	{ "afii10053",            0x0404 },
	{ "afii10054",            0x0405 },
	{ "afii10055",            0x0406 },
	{ "afii10056",            0x0407 },
	{ "afii10057",            0x0408 },
	{ "afii10058",            0x0409 },
	{ "afii10059",            0x040A },
	{ "afii10060",            0x040B },
	{ "afii10061",            0x040C },
	{ "afii10062",            0x040E },
	{ "afii10063",            0xF7AB },
	{ "afii10064",            0xF7AC },
	{ "afii10065",            0x0430 },
	{ "afii10066",            0x0431 },
	{ "afii10067",            0x0432 },
	{ "afii10068",            0x0433 },
	{ "afii10069",            0x0434 },
	{ "afii10070",            0x0435 },
	{ "afii10071",            0x0451 },
	{ "afii10072",            0x0436 },
	{ "afii10073",            0x0437 },
	{ "afii10074",            0x0438 },
	{ "afii10075",            0x0439 },
	{ "afii10076",            0x043A },
	{ "afii10077",            0x043B },
	{ "afii10078",            0x043C },
	{ "afii10079",            0x043D },
	{ "afii10080",            0x043E },
	{ "afii10081",            0x043F },
	{ "afii10082",            0x0440 },
	{ "afii10083",            0x0441 },
	{ "afii10084",            0x0442 },
	{ "afii10085",            0x0443 },
	{ "afii10086",            0x0444 },
	{ "afii10087",            0x0445 },
	{ "afii10088",            0x0446 },
	{ "afii10089",            0x0447 },
	{ "afii10090",            0x0448 },
	{ "afii10091",            0x0449 },
	{ "afii10092",            0x044A },
	{ "afii10093",            0x044B },
	{ "afii10094",            0x044C },
	{ "afii10095",            0x044D },
	{ "afii10096",            0x044E },
	{ "afii10097",            0x044F },
	{ "afii10098",            0x0491 },
	{ "afii10099",            0x0452 },
	{ "afii10100",            0x0453 },
	{ "afii10101",            0x0454 },
	{ "afii10102",            0x0455 },
	{ "afii10103",            0x0456 },
	{ "afii10104",            0x0457 },
	{ "afii10105",            0x0458 },
	{ "afii10106",            0x0459 },
	{ "afii10107",            0x045A },
	{ "afii10108",            0x045B },
	{ "afii10109",            0x045C },
	{ "afii10110",            0x045E },
	{ "afii10145",            0x040F },
	{ "afii10146",            0x0462 },
	{ "afii10147",            0x0472 },
	{ "afii10148",            0x0474 },
	{ "afii10192",            0xF7AD },
	{ "afii10193",            0x045F },
	{ "afii10194",            0x0463 },
	{ "afii10195",            0x0473 },
	{ "afii10196",            0x0475 },
	{ "afii10831",            0xF7AE },
	{ "afii10832",            0xF7AF },
	{ "afii10846",            0x04D9 },
	{ "afii57388",            0x060C },
	{ "afii57392",            0x0660 },
	{ "afii57393",            0x0661 },
	{ "afii57394",            0x0662 },
	{ "afii57395",            0x0663 },
	{ "afii57396",            0x0664 },
	{ "afii57397",            0x0665 },
	{ "afii57398",            0x0666 },
	{ "afii57399",            0x0667 },
	{ "afii57400",            0x0668 },
	{ "afii57401",            0x0669 },
	{ "afii57403",            0x061B },
	{ "afii57407",            0x061F },
	{ "afii57409",            0x0621 },
	{ "afii57410",            0x0622 },
	{ "afii57411",            0x0623 },
	{ "afii57412",            0x0624 },
	{ "afii57413",            0x0625 },
	{ "afii57414",            0x0626 },
	{ "afii57415",            0x0627 },
	{ "afii57416",            0x0628 },
	{ "afii57417",            0x0629 },
	{ "afii57418",            0x062A },
	{ "afii57419",            0x062B },
	{ "afii57420",            0x062C },
	{ "afii57421",            0x062D },
	{ "afii57422",            0x062E },
	{ "afii57423",            0x062F },
	{ "afii57424",            0x0630 },
	{ "afii57425",            0x0631 },
	{ "afii57426",            0x0632 },
	{ "afii57427",            0x0633 },
	{ "afii57428",            0x0634 },
	{ "afii57429",            0x0635 },
	{ "afii57430",            0x0636 },
	{ "afii57431",            0x0637 },
	{ "afii57432",            0x0638 },
	{ "afii57433",            0x0639 },
	{ "afii57434",            0x063A },
	{ "afii57440",            0x0640 },
	{ "afii57441",            0x0641 },
	{ "afii57442",            0x0642 },
	{ "afii57443",            0x0643 },
	{ "afii57444",            0x0644 },
	{ "afii57445",            0x0645 },
	{ "afii57446",            0x0646 },
	{ "afii57448",            0x0648 },
	{ "afii57449",            0x0649 },
	{ "afii57450",            0x064A },
	{ "afii57451",            0x064B },
	{ "afii57452",            0x064C },
	{ "afii57453",            0x064D },
	{ "afii57454",            0x064E },
	{ "afii57455",            0x064F },
	{ "afii57456",            0x0650 },
	{ "afii57457",            0x0651 },
	{ "afii57458",            0x0652 },
	{ "afii57470",            0x0647 },
	{ "afii57506",            0x067E },
	{ "afii57507",            0x0686 },
	{ "afii57508",            0x0698 },
	{ "afii57509",            0x06AF },
	{ "afii57596",            0x200E },
	{ "afii57597",            0x200F },
	{ "afii57598",            0x200D },
	{ "afii57636",            0x20AA },
	{ "afii57645",            0x05BE },
	{ "afii57658",            0x05C3 },
	{ "afii57664",            0x05D0 },
	{ "afii57665",            0x05D1 },
	{ "afii57666",            0x05D2 },
	{ "afii57667",            0x05D3 },
	{ "afii57668",            0x05D4 },
	{ "afii57669",            0x05D5 },
	{ "afii57670",            0x05D6 },
	{ "afii57671",            0x05D7 },
	{ "afii57672",            0x05D8 },
	{ "afii57673",            0x05D9 },
	{ "afii57674",            0x05DA },
	{ "afii57675",            0x05DB },
	{ "afii57676",            0x05DC },
	{ "afii57677",            0x05DD },
	{ "afii57678",            0x05DE },
	{ "afii57679",            0x05DF },
	{ "afii57680",            0x05E0 },
	{ "afii57681",            0x05E1 },
	{ "afii57682",            0x05E2 },
	{ "afii57683",            0x05E3 },
	{ "afii57684",            0x05E4 },
	{ "afii57685",            0x05E5 },
	{ "afii57686",            0x05E6 },
	{ "afii57687",            0x05E7 },
	{ "afii57688",            0x05E8 },
	{ "afii57689",            0x05E9 },
	{ "afii57690",            0x05EA },
	{ "afii57716",            0x05F0 },
	{ "afii57717",            0x05F1 },
	{ "afii57718",            0x05F2 },
	{ "afii57793",            0x05B4 },
	{ "afii57794",            0x05B5 },
	{ "afii57795",            0x05B6 },
	{ "afii57796",            0x05BB },
	{ "afii57797",            0x05B8 },
	{ "afii57798",            0x05B7 },
	{ "afii57799",            0x05B0 },
	{ "afii57800",            0x05B2 },
	{ "afii57801",            0x05B1 },
	{ "afii57802",            0x05B3 },
	{ "afii57803",            0x05C2 },
	{ "afii57804",            0x05C1 },
	{ "afii57806",            0x05B9 },
	{ "afii57807",            0x05BC },
	{ "afii57839",            0x05BD },
	{ "afii57841",            0x05BF },
	{ "afii57842",            0x05C0 },
	{ "afii57929",            0x02BC },
	{ "afii61352",            0x2116 },
	{ "afii61664",            0x200C },
	{ "afii64937",            0x02BD },
	{ "agrave",               0x00E0 },
	{ "aleph",                0x2135 },
	{ "alpha",                0x03B1 },
	{ "alphatonos",           0x03AC },
	{ "amacron",              0x0101 },
	{ "ampersand",            0x0026 },
	{ "ampersandsmall",       0xF826 },
	{ "angle",                0x2220 },
	{ "angleleft",            0x2329 },
	{ "angleright",           0x232A },
	{ "aogonek",              0x0105 },
	{ "approxequal",          0x2248 },
	{ "aring",                0x00E5 },
	{ "arrowboth",            0x2194 },
	{ "arrowdblboth",         0x21D4 },
	{ "arrowdbldown",         0x21D3 },
	{ "arrowdblleft",         0x21D0 },
	{ "arrowdblright",        0x21D2 },
	{ "arrowdblup",           0x21D1 },
	{ "arrowdown",            0x2193 },
	{ "arrowhorizex",         0xF7C0 },
	{ "arrowleft",            0x2190 },
	{ "arrowright",           0x2192 },
	{ "arrowup",              0x2191 },
	{ "arrowvertex",          0xF7C1 },
	{ "asciicircum",          0x005E },
	{ "asciitilde",           0x007E },
	{ "asterisk",             0x002A },
	{ "asteriskmath",         0x2217 },
	{ "asuperior",            0xF7E9 },
	{ "at",                   0x0040 },
	{ "atilde",               0x00E3 },
	{ "b",                    0x0062 },
	{ "backslash",            0x005C },
	{ "bar",                  0x007C },
	{ "beta",                 0x03B2 },
	{ "braceex",              0xF7C2 },
	{ "braceleft",            0x007B },
	{ "braceleftbt",          0xF7C3 },
	{ "braceleftmid",         0xF7C4 },
	{ "bracelefttp",          0xF7C5 },
	{ "braceright",           0x007D },
	{ "bracerightbt",         0xF7C6 },
	{ "bracerightmid",        0xF7C7 },
	{ "bracerighttp",         0xF7C8 },
	{ "bracketleft",          0x005B },
	{ "bracketleftbt",        0xF7C9 },
	{ "bracketleftex",        0xF7CA },
	{ "bracketlefttp",        0xF7CB },
	{ "bracketright",         0x005D },
	{ "bracketrightbt",       0xF7CC },
	{ "bracketrightex",       0xF7CD },
	{ "bracketrighttp",       0xF7CE },
	{ "breve",                0x02D8 },
	{ "brokenbar",            0x00A6 },
	{ "bsuperior",            0xF7EA },
	{ "bullet",               0x2022 },
	{ "c",                    0x0063 },
	{ "cacute",               0x0107 },
	{ "caron",                0x02C7 },
	{ "carriagereturn",       0x21B5 },
	{ "ccaron",               0x010D },
	{ "ccedilla",             0x00E7 },
	{ "ccircumflex",          0x0109 },
	{ "cdotaccent",           0x010B },
	{ "cedilla",              0x00B8 },
	{ "cent",                 0x00A2 },
	{ "centinferior",         0xF7DF },
	{ "centoldstyle",         0xF8A2 },
	{ "centsuperior",         0xF7E0 },
	{ "chi",                  0x03C7 },
	{ "circlemultiply",       0x2297 },
	{ "circleplus",           0x2295 },
	{ "circumflex",           0x02C6 },
	{ "club",                 0x2663 },
	{ "colon",                0x003A },
	{ "colonmonetary",        0x20A1 },
	{ "comma",                0x002C },
	{ "commaaccent",          0xF7AA },
	{ "commainferior",        0xF7E1 },
	{ "commasuperior",        0xF7E2 },
	{ "congruent",            0x2245 },
	{ "copyright",            0x00A9 },
	{ "copyrightsans",        0xF7CF },
	{ "copyrightserif",       0xF7D0 },
	{ "currency",             0x00A4 },
	{ "cyrBreve",             0xF7B8 },
	{ "cyrFlex",              0xF7B9 },
	{ "cyrbreve",             0xF7BB },
	{ "cyrflex",              0xF7BC },
	{ "d",                    0x0064 },
	{ "dagger",               0x2020 },
	{ "daggerdbl",            0x2021 },
	{ "dblGrave",             0xF7BA },
	{ "dblgrave",             0xF7BD },
	{ "dcaron",               0x010F },
	{ "dcroat",               0x0111 },
	{ "degree",               0x00B0 },
	{ "delta",                0x03B4 },
	{ "diamond",              0x2666 },
	{ "dieresis",             0x00A8 },
	{ "dieresisacute",        0xF7BE },
	{ "dieresisgrave",        0xF7BF },
	{ "dieresistonos",        0x0385 },
	{ "divide",               0x00F7 },
	{ "dollar",               0x0024 },
	{ "dollarinferior",       0xF7E3 },
	{ "dollaroldstyle",       0xF824 },
	{ "dollarsuperior",       0xF7E4 },
	{ "dong",                 0x20AB },
	{ "dotaccent",            0x02D9 },
	{ "dotbelowcomb",         0x0323 },
	{ "dotlessi",             0x0131 },
	{ "dotmath",              0x22C5 },
	{ "dsuperior",            0xF7EB },
	{ "e",                    0x0065 },
	{ "eacute",               0x00E9 },
	{ "ecaron",               0x011B },
	{ "ecircumflex",          0x00EA },
	{ "edieresis",            0x00EB },
	{ "edotaccent",           0x0117 },
	{ "egrave",               0x00E8 },
	{ "eight",                0x0038 },
	{ "eightinferior",        0x2088 },
	{ "eightoldstyle",        0xF838 },
	{ "eightsuperior",        0x2078 },
	{ "element",              0x2208 },
	{ "ellipsis",             0x2026 },
	{ "emacron",              0x0113 },
	{ "emdash",               0x2014 },
	{ "emptyset",             0x2205 },
	{ "endash",               0x2013 },
	{ "eng",                  0x014B },
	{ "eogonek",              0x0119 },
	{ "epsilon",              0x03B5 },
	{ "epsilontonos",         0x03AD },
	{ "equal",                0x003D },
	{ "equivalence",          0x2261 },
	{ "esuperior",            0xF7EC },
	{ "eta",                  0x03B7 },
	{ "etatonos",             0x03AE },
	{ "eth",                  0x00F0 },
	{ "exclam",               0x0021 },
	{ "exclamdbl",            0x203C },
	{ "exclamdown",           0x00A1 },
	{ "exclamdownsmall",      0xF8A1 },
	{ "exclamsmall",          0xF821 },
	{ "existential",          0x2203 },
	{ "f",                    0x0066 },
	{ "ff",                   0xFB00 },
	{ "ffi",                  0xFB03 },
	{ "ffl",                  0xFB04 },
	{ "fi",                   0xFB01 },
	{ "figuredash",           0x2012 },
	{ "five",                 0x0035 },
	{ "fiveeighths",          0x215D },
	{ "fiveinferior",         0x2085 },
	{ "fiveoldstyle",         0xF835 },
	{ "fivesuperior",         0x2075 },
	{ "fl",                   0xFB02 },
	{ "florin",               0x0192 },
	{ "four",                 0x0034 },
	{ "fourinferior",         0x2084 },
	{ "fouroldstyle",         0xF834 },
	{ "foursuperior",         0x2074 },
	{ "fraction",             0x2044 },
	{ "franc",                0x20A3 },
	{ "g",                    0x0067 },
	{ "gamma",                0x03B3 },
	{ "gbreve",               0x011F },
	{ "gcaron",               0x01E7 },
	{ "gcircumflex",          0x011D },
	{ "gcommaaccent",         0x0123 },
	{ "gdotaccent",           0x0121 },
	{ "germandbls",           0x00DF },
	{ "gradient",             0x2207 },
	{ "grave",                0x0060 },
	{ "gravecomb",            0x0300 },
	{ "greater",              0x003E },
	{ "greaterequal",         0x2265 },
	{ "guillemotleft",        0x00AB },
	{ "guillemotright",       0x00BB },
	{ "guilsinglleft",        0x2039 },
	{ "guilsinglright",       0x203A },
	{ "h",                    0x0068 },
	{ "hbar",                 0x0127 },
	{ "hcircumflex",          0x0125 },
	{ "heart",                0x2665 },
	{ "hookabovecomb",        0x0309 },
	{ "hungarumlaut",         0x02DD },
	{ "hyphen",               0x002D },
	{ "hyphen_",              0x00ad },		//*
	{ "hyphen__",             0x2010 },		//*
	{ "hypheninferior",       0xF7E5 },
	{ "hyphensuperior",       0xF7E6 },
	{ "i",                    0x0069 },
	{ "iacute",               0x00ED },
	{ "icircumflex",          0x00EE },
	{ "idieresis",            0x00EF },
	{ "igrave",               0x00EC },
	{ "ij",                   0x0133 },
	{ "imacron",              0x012B },
	{ "infinity",             0x221E },
	{ "integral",             0x222B },
	{ "integralbt",           0x2321 },
	{ "integralex",           0xF7D1 },
	{ "integraltp",           0x2320 },
	{ "intersection",         0x2229 },
	{ "iogonek",              0x012F },
	{ "iota",                 0x03B9 },
	{ "iotadieresis",         0x03CA },
	{ "iotadieresistonos",    0x0390 },
	{ "iotatonos",            0x03AF },
	{ "isuperior",            0xF7ED },
	{ "itilde",               0x0129 },
	{ "j",                    0x006A },
	{ "jcircumflex",          0x0135 },
	{ "k",                    0x006B },
	{ "kappa",                0x03BA },
	{ "kcommaaccent",         0x0137 },
	{ "kgreenlandic",         0x0138 },
	{ "l",                    0x006C },
	{ "lacute",               0x013A },
	{ "lambda",               0x03BB },
	{ "lcaron",               0x013E },
	{ "lcommaaccent",         0x013C },
	{ "ldot",                 0x0140 },
	{ "less",                 0x003C },
	{ "lessequal",            0x2264 },
	{ "lira",                 0x20A4 },
	{ "ll",                   0xF7A7 },
	{ "logicaland",           0x2227 },
	{ "logicalnot",           0x00AC },
	{ "logicalor",            0x2228 },
	{ "lozenge",              0x25CA },
	{ "lslash",               0x0142 },
	{ "lsuperior",            0xF7EE },
	{ "m",                    0x006D },
	{ "macron",               0x00AF },
	{ "minus",                0x2212 },
	{ "minute",               0x2032 },
	{ "msuperior",            0xF7EF },
	{ "mu",                   0x00B5 },
	{ "mu_",                  0x03bc },		//*
	{ "multiply",             0x00D7 },
	{ "n",                    0x006E },
	{ "nacute",               0x0144 },
	{ "napostrophe",          0x0149 },
	{ "ncaron",               0x0148 },
	{ "ncommaaccent",         0x0146 },
	{ "nine",                 0x0039 },
	{ "nineinferior",         0x2089 },
	{ "nineoldstyle",         0xF839 },
	{ "ninesuperior",         0x2079 },
	{ "notelement",           0x2209 },
	{ "notequal",             0x2260 },
	{ "notsubset",            0x2284 },
	{ "nsuperior",            0x207F },
	{ "ntilde",               0x00F1 },
	{ "nu",                   0x03BD },
	{ "numbersign",           0x0023 },
	{ "o",                    0x006F },
	{ "oacute",               0x00F3 },
	{ "ocircumflex",          0x00F4 },
	{ "odieresis",            0x00F6 },
	{ "oe",                   0x0153 },
	{ "ogonek",               0x02DB },
	{ "ograve",               0x00F2 },
	{ "ohorn",                0x01A1 },
	{ "ohungarumlaut",        0x0151 },
	{ "omacron",              0x014D },
	{ "omega",                0x03C9 },
	{ "omega1",               0x03D6 },
	{ "omegatonos",           0x03CE },
	{ "omicron",              0x03BF },
	{ "omicrontonos",         0x03CC },
	{ "one",                  0x0031 },
	{ "onedotenleader",       0x2024 },
	{ "oneeighth",            0x215B },
	{ "onefitted",            0xF7DC },
	{ "onehalf",              0x00BD },
	{ "oneinferior",          0x2081 },
	{ "oneoldstyle",          0xF831 },
	{ "onequarter",           0x00BC },
	{ "onesuperior",          0x00B9 },
	{ "onethird",             0x2153 },
	{ "ordfeminine",          0x00AA },
	{ "ordmasculine",         0x00BA },
	{ "oslash",               0x00F8 },
	{ "osuperior",            0xF7F0 },
	{ "otilde",               0x00F5 },
	{ "p",                    0x0070 },
	{ "paragraph",            0x00B6 },
	{ "parenleft",            0x0028 },
	{ "parenleftbt",          0xF7D2 },
	{ "parenleftex",          0xF7D3 },
	{ "parenleftinferior",    0x208D },
	{ "parenleftsuperior",    0x207D },
	{ "parenlefttp",          0xF7D4 },
	{ "parenright",           0x0029 },
	{ "parenrightbt",         0xF7D5 },
	{ "parenrightex",         0xF7D6 },
	{ "parenrightinferior",   0x208E },
	{ "parenrightsuperior",   0x207E },
	{ "parenrighttp",         0xF7D7 },
	{ "partialdiff",          0x2202 },
	{ "percent",              0x0025 },
	{ "period",               0x002E },
	{ "periodcentered",       0x00B7 },
	{ "periodinferior",       0xF7E7 },
	{ "periodsuperior",       0xF7E8 },
	{ "perpendicular",        0x22A5 },
	{ "perthousand",          0x2030 },
	{ "phi",                  0x03C6 },
	{ "phi1",                 0x03D5 },
	{ "pi",                   0x03C0 },
	{ "plus",                 0x002B },
	{ "plusminus",            0x00B1 },
	{ "prescription",         0x211E },
	{ "product",              0x220F },
	{ "propersubset",         0x2282 },
	{ "propersuperset",       0x2283 },
	{ "proportional",         0x221D },
	{ "psi",                  0x03C8 },
	{ "q",                    0x0071 },
	{ "question",             0x003F },
	{ "questiondown",         0x00BF },
	{ "questiondownsmall",    0xF8BF },
	{ "questionsmall",        0xF83F },
	{ "quotedbl",             0x0022 },
	{ "quotedblbase",         0x201E },
	{ "quotedblleft",         0x201C },
	{ "quotedblright",        0x201D },
	{ "quoteleft",            0x2018 },
	{ "quoteright",           0x2019 },
	{ "quotesinglbase",       0x201A },
	{ "quotesingle",          0x0027 },
	{ "r",                    0x0072 },
	{ "racute",               0x0155 },
	{ "radical",              0x221A },
	{ "radicalex",            0x203E },
	{ "rcaron",               0x0159 },
	{ "rcommaaccent",         0x0157 },
	{ "reflexsubset",         0x2286 },
	{ "reflexsuperset",       0x2287 },
	{ "registered",           0x00AE },
	{ "registersans",         0xF7D8 },
	{ "registerserif",        0xF7D9 },
	{ "rho",                  0x03C1 },
	{ "ring",                 0x02DA },
	{ "rsuperior",            0xF7F1 },
	{ "rupiah",               0xF7DD },
	{ "s",                    0x0073 },
	{ "sacute",               0x015B },
	{ "scaron",               0x0161 },
	{ "scedilla",             0xF7A9 },
	{ "scircumflex",          0x015D },
	{ "scommaaccent",         0x015F },
	{ "second",               0x2033 },
	{ "section",              0x00A7 },
	{ "semicolon",            0x003B },
	{ "seven",                0x0037 },
	{ "seveneighths",         0x215E },
	{ "seveninferior",        0x2087 },
	{ "sevenoldstyle",        0xF837 },
	{ "sevensuperior",        0x2077 },
	{ "sigma",                0x03C3 },
	{ "sigma1",               0x03C2 },
	{ "similar",              0x223C },
	{ "six",                  0x0036 },
	{ "sixinferior",          0x2086 },
	{ "sixoldstyle",          0xF836 },
	{ "sixsuperior",          0x2076 },
	{ "slash",                0x002F },
	{ "space",                0x0020 },
	{ "space_",               0x00a0 },		//*
	{ "spade",                0x2660 },
	{ "ssuperior",            0xF7F2 },
	{ "sterling",             0x00A3 },
	{ "suchthat",             0x220B },
	{ "summation",            0x2211 },
	{ "t",                    0x0074 },
	{ "tau",                  0x03C4 },
	{ "tbar",                 0x0167 },
	{ "tcaron",               0x0165 },
	{ "tcommaaccent",         0x0163 },
	{ "therefore",            0x2234 },
	{ "theta",                0x03B8 },
	{ "theta1",               0x03D1 },
	{ "thorn",                0x00FE },
	{ "three",                0x0033 },
	{ "threeeighths",         0x215C },
	{ "threeinferior",        0x2083 },
	{ "threeoldstyle",        0xF833 },
	{ "threequarters",        0x00BE },
	{ "threequartersemdash",  0xF7DE },
	{ "threesuperior",        0x00B3 },
	{ "tilde",                0x02DC },
	{ "tildecomb",            0x0303 },
	{ "tonos",                0x0384 },
	{ "trademark",            0x2122 },
	{ "trademarksans",        0xF7DA },
	{ "trademarkserif",       0xF7DB },
	{ "tsuperior",            0xF7F3 },
	{ "two",                  0x0032 },
	{ "twodotenleader",       0x2025 },
	{ "twoinferior",          0x2082 },
	{ "twooldstyle",          0xF832 },
	{ "twosuperior",          0x00B2 },
	{ "twothirds",            0x2154 },
	{ "u",                    0x0075 },
	{ "uacute",               0x00FA },
	{ "ubreve",               0x016D },
	{ "ucircumflex",          0x00FB },
	{ "udieresis",            0x00FC },
	{ "ugrave",               0x00F9 },
	{ "uhorn",                0x01B0 },
	{ "uhungarumlaut",        0x0171 },
	{ "umacron",              0x016B },
	{ "underscore",           0x005F },
	{ "underscoredbl",        0x2017 },
	{ "union",                0x222A },
	{ "universal",            0x2200 },
	{ "uogonek",              0x0173 },
	{ "upsilon",              0x03C5 },
	{ "upsilondieresis",      0x03CB },
	{ "upsilondieresistonos", 0x03B0 },
	{ "upsilontonos",         0x03CD },
	{ "uring",                0x016F },
	{ "utilde",               0x0169 },
	{ "v",                    0x0076 },
	{ "w",                    0x0077 },
	{ "weierstrass",          0x2118 },
	{ "x",                    0x0078 },
	{ "xi",                   0x03BE },
	{ "y",                    0x0079 },
	{ "yacute",               0x00FD },
	{ "ydieresis",            0x00FF },
	{ "yen",                  0x00A5 },
	{ "z",                    0x007A },
	{ "zacute",               0x017A },
	{ "zcaron",               0x017E },
	{ "zdotaccent",           0x017C },
	{ "zero",                 0x0030 },
	{ "zeroinferior",         0x2080 },
	{ "zerooldstyle",         0xF830 },
	{ "zerosuperior",         0x2070 },
	{ "zeta",                 0x03B6 },
};

/* following is new one from 97/Mar/8 */

/* 1044 entries of which 170 are in corporate use subareas */

STATIC const UNICODE_MAP unicodeMap[] =
{
	{ "A",					0x0041 },
	{ "AE",					0x00C6 },
	{ "AEacute",			0x01FC },	/* NEW */
#ifdef CORPORATEUSE
	{ "AEsmall",			0xF7E6 },	/* Adobe corporate use subarea */
#endif
	{ "Aacute",				0x00C1 },
#ifdef CORPORATEUSE
	{ "Aacutesmall",		0xF7E1 },	/* Adobe corporate use subarea */
#endif
	{ "Abreve",				0x0102 },
	{ "Acircumflex",		0x00C2 },
#ifdef CORPORATEUSE
	{ "Acircumflexsmall",	0xF7E2 },	/* Adobe corporate use subarea */
	{ "Acute",				0xF6C9 },	/* Adobe corporate use subarea */
	{ "Acutesmall",			0xF7B4 },	/* Adobe corporate use subarea */
#endif
	{ "Adieresis",			0x00C4 },
#ifdef CORPORATEUSE
	{ "Adieresissmall",		0xF7E4 },	/* Adobe corporate use subarea */
#endif
	{ "Agrave",				0x00C0 },
#ifdef CORPORATEUSE
	{ "Agravesmall",		0xF7E0 },	/* Adobe corporate use subarea */
#endif
	{ "Alpha",				0x0391 },
	{ "Alphatonos",			0x0386 },
	{ "Amacron",			0x0100 },
	{ "Aogonek",			0x0104 },
	{ "Aring",				0x00C5 },
	{ "Aringacute",			0x01FA },
#ifdef CORPORATEUSE
	{ "Aringsmall",			0xF7E5 },	/* Adobe corporate use subarea */
	{ "Asmall",				0xF761 },	/* Adobe corporate use subarea */
#endif
	{ "Atilde",				0x00C3 },
#ifdef CORPORATEUSE
	{ "Atildesmall",		0xF7E3 },	/* Adobe corporate use subarea */
#endif
	{ "B",					0x0042 },
	{ "Beta",				0x0392 },
#ifdef CORPORATEUSE
	{ "Brevesmall",			0xF6F4 },	/* Adobe corporate use subarea */
	{ "Bsmall",				0xF762 },	/* Adobe corporate use subarea */
#endif
	{ "C",					0x0043 },
	{ "Cacute",				0x0106 },
#ifdef CORPORATEUSE
	{ "Caron",				0xF6CA },	/* Adobe corporate use subarea */
	{ "Caronsmall",			0xF6F5 },	/* Adobe corporate use subarea */
#endif
	{ "Ccaron",				0x010C },
	{ "Ccedilla",			0x00C7 },
#ifdef CORPORATEUSE
	{ "Ccedillasmall",		0xF7E7 },	/* Adobe corporate use subarea */
#endif
	{ "Ccircumflex",		0x0108 },
	{ "Cdotaccent",			0x010A },
#ifdef CORPORATEUSE
	{ "Cedillasmall",		0xF7B8 },	/* Adobe corporate use subarea */
#endif
	{ "Chi",				0x03A7 },
#ifdef CORPORATEUSE
	{ "Circumflexsmall",	0xF6F6 },	/* Adobe corporate use subarea */
	{ "Csmall",				0xF763 },	/* Adobe corporate use subarea */
#endif
	{ "D",					0x0044 },
	{ "Dcaron",				0x010E },
	{ "Dcroat",				0x0110 },
	{ "Delta",				0x0394 },	/* 			Duplicate */
	{ "Delta_",				0x2206 },	/* actually `increment' */
#ifdef CORPORATEUSE
	{ "Dieresis",			0xF6CB },	/* Adobe corporate use subarea */
	{ "DieresisAcute",		0xF6CC },	/* Adobe corporate use subarea */
	{ "DieresisGrave",		0xF6CD },	/* Adobe corporate use subarea */
	{ "Dieresissmall",		0xF7A8 },	/* Adobe corporate use subarea */
	{ "Dotaccentsmall",		0xF6F7 },	/* Adobe corporate use subarea */
	{ "Dsmall",				0xF764 },	/* Adobe corporate use subarea */
#endif
	{ "E",					0x0045 },
	{ "Eacute",				0x00C9 },
#ifdef CORPORATEUSE
	{ "Eacutesmall",		0xF7E9 },	/* Adobe corporate use subarea */
#endif
	{ "Ebreve",				0x0114 },
	{ "Ecaron",				0x011A },
	{ "Ecircumflex",		0x00CA },
#ifdef CORPORATEUSE
	{ "Ecircumflexsmall",	0xF7EA },	/* Adobe corporate use subarea */
#endif
	{ "Edieresis",			0x00CB },
#ifdef CORPORATEUSE
	{ "Edieresissmall",		0xF7EB },	/* Adobe corporate use subarea */
#endif
	{ "Edotaccent",			0x0116 },
	{ "Egrave",				0x00C8 },
#ifdef CORPORATEUSE
	{ "Egravesmall",		0xF7E8 },	/* Adobe corporate use subarea */
#endif
	{ "Emacron",			0x0112 },
	{ "Eng",				0x014A },
	{ "Eogonek",			0x0118 },
	{ "Epsilon",			0x0395 },
	{ "Epsilontonos",		0x0388 },
#ifdef CORPORATEUSE
	{ "Esmall",				0xF765 },	/* Adobe corporate use subarea */
#endif
	{ "Eta",				0x0397 },
	{ "Etatonos",			0x0389 },
	{ "Eth",				0x00D0 },
#ifdef CORPORATEUSE
	{ "Ethsmall",			0xF7F0 },	/* Adobe corporate use subarea */
#endif
	{ "F",					0x0046 },
#ifdef CORPORATEUSE
	{ "Fsmall",				0xF766 },	/* Adobe corporate use subarea */
#endif
	{ "G",					0x0047 },
	{ "Gamma",				0x0393 },
	{ "Gbreve",				0x011E },
	{ "Gcaron",				0x01E6 },
	{ "Gcircumflex",		0x011C },
	{ "Gcommaaccent",		0x0122 },
	{ "Gdotaccent",			0x0120 },
#ifdef CORPORATEUSE
	{ "Grave",				0xF6CE },	/* Adobe corporate use subarea */
	{ "Gravesmall",			0xF760 },	/* Adobe corporate use subarea */
	{ "Gsmall",				0xF767 },	/* Adobe corporate use subarea */
#endif
	{ "H",					0x0048 },
	{ "H18533",				0x25CF },
	{ "H18543",				0x25AA },
	{ "H18551",				0x25AB },
	{ "H22073",				0x25A1 },
	{ "Hbar",				0x0126 },
	{ "Hcircumflex",		0x0124 },
#ifdef CORPORATEUSE
	{ "Hsmall",				0xF768 },	/* Adobe corporate use subarea */
	{ "Hungarumlaut",		0xF6CF },	/* Adobe corporate use subarea */
	{ "Hungarumlautsmall",	0xF6F8 },	/* Adobe corporate use subarea */
#endif
	{ "I",					0x0049 },
	{ "IJ",					0x0132 },
	{ "Iacute",				0x00CD },
#ifdef CORPORATEUSE
	{ "Iacutesmall",		0xF7ED },	/* Adobe corporate use subarea */
#endif
	{ "Ibreve",				0x012C },
	{ "Icircumflex",		0x00CE },
#ifdef CORPORATEUSE
	{ "Icircumflexsmall",	0xF7EE },	/* Adobe corporate use subarea */
#endif
	{ "Idieresis",			0x00CF },
#ifdef CORPORATEUSE
	{ "Idieresissmall",		0xF7EF },	/* Adobe corporate use subarea */
#endif
	{ "Idotaccent",			0x0130 },
	{ "Ifraktur",			0x2111 },
	{ "Igrave",				0x00CC },
#ifdef CORPORATEUSE
	{ "Igravesmall",		0xF7EC },	/* Adobe corporate use subarea */
#endif
	{ "Imacron",			0x012A },
	{ "Iogonek",			0x012E },
	{ "Iota",				0x0399 },
	{ "Iotadieresis",		0x03AA },
	{ "Iotatonos",			0x038A },
#ifdef CORPORATEUSE
	{ "Ismall",				0xF769 },	/* Adobe corporate use subarea */
#endif
	{ "Itilde",				0x0128 },
	{ "J",					0x004A },
	{ "Jcircumflex",		0x0134 },
#ifdef CORPORATEUSE
	{ "Jsmall",				0xF76A },	/* Adobe corporate use subarea */
#endif
	{ "K",					0x004B },
	{ "Kappa",				0x039A },
	{ "Kcommaaccent",		0x0136 },
#ifdef CORPORATEUSE
	{ "Ksmall",				0xF76B },	/* Adobe corporate use subarea */
#endif
	{ "L",					0x004C },
#ifdef CORPORATEUSE
	{ "LL",					0xF6BF },	/* Adobe corporate use subarea */
#endif
	{ "Lacute",				0x0139 },
	{ "Lambda",				0x039B },
	{ "Lcaron",				0x013D },
	{ "Lcommaaccent",		0x013B },
	{ "Ldot",				0x013F },
	{ "Lslash",				0x0141 },
#ifdef CORPORATEUSE
	{ "Lslashsmall",		0xF6F9 },	/* Adobe corporate use subarea */
	{ "Lsmall",				0xF76C },	/* Adobe corporate use subarea */
#endif
	{ "M",					0x004D },
#ifdef CORPORATEUSE
	{ "Macron",				0xF6D0 },	/* Adobe corporate use subarea */
	{ "Macronsmall",		0xF7AF },	/* Adobe corporate use subarea */
	{ "Msmall",				0xF76D },	/* Adobe corporate use subarea */
#endif
	{ "Mu",					0x039C },
	{ "N",					0x004E },
	{ "Nacute",				0x0143 },
	{ "Ncaron",				0x0147 },
	{ "Ncommaaccent",		0x0145 },
#ifdef CORPORATEUSE
	{ "Nsmall",				0xF76E },	/* Adobe corporate use subarea */
#endif
	{ "Ntilde",				0x00D1 },
#ifdef CORPORATEUSE
	{ "Ntildesmall",		0xF7F1 },	/* Adobe corporate use subarea */
#endif
	{ "Nu",					0x039D },
	{ "O",					0x004F },
	{ "OE",					0x0152 },
#ifdef CORPORATEUSE
	{ "OEsmall",			0xF6FA },	/* Adobe corporate use subarea */
#endif
	{ "Oacute",				0x00D3 },
#ifdef CORPORATEUSE
	{ "Oacutesmall",		0xF7F3 },	/* Adobe corporate use subarea */
#endif
	{ "Obreve",				0x014E },
	{ "Ocircumflex",		0x00D4 },
#ifdef CORPORATEUSE
	{ "Ocircumflexsmall",	0xF7F4 },	/* Adobe corporate use subarea */
#endif
	{ "Odieresis",			0x00D6 },
#ifdef CORPORATEUSE
	{ "Odieresissmall",		0xF7F6 },	/* Adobe corporate use subarea */
	{ "Ogoneksmall",		0xF6FB },	/* Adobe corporate use subarea */
#endif
	{ "Ograve",				0x00D2 },
#ifdef CORPORATEUSE
	{ "Ogravesmall",		0xF7F2 },	/* Adobe corporate use subarea */
#endif
	{ "Ohorn",				0x01A0 },
	{ "Ohungarumlaut",		0x0150 },
	{ "Omacron",			0x014C },
	{ "Omega",				0x03A9 },	/* 			Duplicate */
	{ "Omega_",				0x2126 },	/* actually Ohm */
	{ "Omegatonos",			0x038F },
	{ "Omicron",			0x039F },
	{ "Omicrontonos",		0x038C },
	{ "Oslash",				0x00D8 },
	{ "Oslashacute",		0x01FE },
#ifdef CORPORATEUSE
	{ "Oslashsmall",		0xF7F8 },	/* Adobe corporate use subarea */
	{ "Osmall",				0xF76F },	/* Adobe corporate use subarea */
#endif
	{ "Otilde",				0x00D5 },
#ifdef CORPORATEUSE
	{ "Otildesmall",		0xF7F5 },	/* Adobe corporate use subarea */
#endif
	{ "P",					0x0050 },
	{ "Phi",				0x03A6 },
	{ "Pi",					0x03A0 },
	{ "Psi",				0x03A8 },
#ifdef CORPORATEUSE
	{ "Psmall",				0xF770 },	/* Adobe corporate use subarea */
#endif
	{ "Q",					0x0051 },
#ifdef CORPORATEUSE
	{ "Qsmall",				0xF771 },	/* Adobe corporate use subarea */
#endif
	{ "R",					0x0052 },
	{ "Racute",				0x0154 },
	{ "Rcaron",				0x0158 },
	{ "Rcommaaccent",		0x0156 },
	{ "Rfraktur",			0x211C },
	{ "Rho",				0x03A1 },
#ifdef CORPORATEUSE
	{ "Ringsmall",			0xF6FC },	/* Adobe corporate use subarea */
	{ "Rsmall",				0xF772 },	/* Adobe corporate use subarea */
#endif
	{ "S",					0x0053 },
	{ "SF010000",			0x250C },
	{ "SF020000",			0x2514 },
	{ "SF030000",			0x2510 },
	{ "SF040000",			0x2518 },
	{ "SF050000",			0x253C },
	{ "SF060000",			0x252C },
	{ "SF070000",			0x2534 },
	{ "SF080000",			0x251C },
	{ "SF090000",			0x2524 },
	{ "SF100000",			0x2500 },
	{ "SF110000",			0x2502 },
	{ "SF190000",			0x2561 },
	{ "SF200000",			0x2562 },
	{ "SF210000",			0x2556 },
	{ "SF220000",			0x2555 },
	{ "SF230000",			0x2563 },
	{ "SF240000",			0x2551 },
	{ "SF250000",			0x2557 },
	{ "SF260000",			0x255D },
	{ "SF270000",			0x255C },
	{ "SF280000",			0x255B },
	{ "SF360000",			0x255E },
	{ "SF370000",			0x255F },
	{ "SF380000",			0x255A },
	{ "SF390000",			0x2554 },
	{ "SF400000",			0x2569 },
	{ "SF410000",			0x2566 },
	{ "SF420000",			0x2560 },
	{ "SF430000",			0x2550 },
	{ "SF440000",			0x256C },
	{ "SF450000",			0x2567 },
	{ "SF460000",			0x2568 },
	{ "SF470000",			0x2564 },
	{ "SF480000",			0x2565 },
	{ "SF490000",			0x2559 },
	{ "SF500000",			0x2558 },
	{ "SF510000",			0x2552 },
	{ "SF520000",			0x2553 },
	{ "SF530000",			0x256B },
	{ "SF540000",			0x256A },
	{ "Sacute",				0x015A },
	{ "Scaron",				0x0160 },
#ifdef CORPORATEUSE
	{ "Scaronsmall",		0xF6FD },	/* Adobe corporate use subarea */
	{ "Scedilla",			0xF6C1 },	/* Adobe corporate use subarea */
#endif
	{ "Scircumflex",		0x015C },
	{ "Scommaaccent",		0x015E },
	{ "Sigma",				0x03A3 },
#ifdef CORPORATEUSE
	{ "Ssmall",				0xF773 },	/* Adobe corporate use subarea */
#endif
	{ "T",					0x0054 },
	{ "Tau",				0x03A4 },
	{ "Tbar",				0x0166 },
	{ "Tcaron",				0x0164 },
	{ "Tcommaaccent",		0x0162 },
	{ "Theta",				0x0398 },
	{ "Thorn",				0x00DE },
#ifdef CORPORATEUSE
	{ "Thornsmall",			0xF7FE },	/* Adobe corporate use subarea */
	{ "Tildesmall",			0xF6FE },	/* Adobe corporate use subarea */
	{ "Tsmall",				0xF774 },	/* Adobe corporate use subarea */
#endif
	{ "U",					0x0055 },
	{ "Uacute",				0x00DA },
#ifdef CORPORATEUSE
	{ "Uacutesmall",		0xF7FA },	/* Adobe corporate use subarea */
#endif
	{ "Ubreve",				0x016C },
	{ "Ucircumflex",		0x00DB },
#ifdef CORPORATEUSE
	{ "Ucircumflexsmall",	0xF7FB },	/* Adobe corporate use subarea */
#endif
	{ "Udieresis",			0x00DC },
#ifdef CORPORATEUSE
	{ "Udieresissmall",		0xF7FC },	/* Adobe corporate use subarea */
#endif
	{ "Ugrave",				0x00D9 },
#ifdef CORPORATEUSE
	{ "Ugravesmall",		0xF7F9 },	/* Adobe corporate use subarea */
#endif
	{ "Uhorn",				0x01AF },
	{ "Uhungarumlaut",		0x0170 },
	{ "Umacron",			0x016A },
	{ "Uogonek",			0x0172 },
	{ "Upsilon",			0x03A5 },
	{ "Upsilon1",			0x03D2 },
	{ "Upsilondieresis",	0x03AB },
	{ "Upsilontonos",		0x038E },
	{ "Uring",				0x016E },
#ifdef CORPORATEUSE
	{ "Usmall",				0xF775 },	/* Adobe corporate use subarea */
#endif
	{ "Utilde",				0x0168 },
	{ "V",					0x0056 },
#ifdef CORPORATEUSE
	{ "Vsmall",				0xF776 },	/* Adobe corporate use subarea */
#endif
	{ "W",					0x0057 },
	{ "Wacute",				0x1E82 },
	{ "Wcircumflex",		0x0174 },
	{ "Wdieresis",			0x1E84 },
	{ "Wgrave",				0x1E80 },
#ifdef CORPORATEUSE
	{ "Wsmall",				0xF777 },	/* Adobe corporate use subarea */
#endif
	{ "X",					0x0058 },
	{ "Xi",					0x039E },
#ifdef CORPORATEUSE
	{ "Xsmall",				0xF778 },	/* Adobe corporate use subarea */
#endif
	{ "Y",					0x0059 },
	{ "Yacute",				0x00DD },
#ifdef CORPORATEUSE
	{ "Yacutesmall",		0xF7FD },	/* Adobe corporate use subarea */
#endif
	{ "Ycircumflex",		0x0176 },
	{ "Ydieresis",			0x0178 },
#ifdef CORPORATEUSE
	{ "Ydieresissmall",		0xF7FF },	/* Adobe corporate use subarea */
#endif
	{ "Ygrave",				0x1EF2 },
#ifdef CORPORATEUSE
	{ "Ysmall",				0xF779 },	/* Adobe corporate use subarea */
#endif
	{ "Z",					0x005A },
	{ "Zacute",				0x0179 },
	{ "Zcaron",				0x017D },
#ifdef CORPORATEUSE
	{ "Zcaronsmall",		0xF6FF },	/* Adobe corporate use subarea */
#endif
	{ "Zdotaccent",			0x017B },
	{ "Zeta",				0x0396 },
#ifdef CORPORATEUSE
	{ "Zsmall",				0xF77A },	/* Adobe corporate use subarea */
#endif
	{ "a",					0x0061 },
	{ "aacute",				0x00E1 },
	{ "abreve",				0x0103 },
	{ "acircumflex",		0x00E2 },
	{ "acute",				0x00B4 },
	{ "acutecomb",			0x0301 },
	{ "adieresis",			0x00E4 },
	{ "ae",					0x00E6 },
	{ "aeacute",			0x01FD },
	{ "afii00208",			0x2015 },
	{ "afii10017",			0x0410 },
	{ "afii10018",			0x0411 },
	{ "afii10019",			0x0412 },
	{ "afii10020",			0x0413 },
	{ "afii10021",			0x0414 },
	{ "afii10022",			0x0415 },
	{ "afii10023",			0x0401 },
	{ "afii10024",			0x0416 },
	{ "afii10025",			0x0417 },
	{ "afii10026",			0x0418 },
	{ "afii10027",			0x0419 },
	{ "afii10028",			0x041A },
	{ "afii10029",			0x041B },
	{ "afii10030",			0x041C },
	{ "afii10031",			0x041D },
	{ "afii10032",			0x041E },
	{ "afii10033",			0x041F },
	{ "afii10034",			0x0420 },
	{ "afii10035",			0x0421 },
	{ "afii10036",			0x0422 },
	{ "afii10037",			0x0423 },
	{ "afii10038",			0x0424 },
	{ "afii10039",			0x0425 },
	{ "afii10040",			0x0426 },
	{ "afii10041",			0x0427 },
	{ "afii10042",			0x0428 },
	{ "afii10043",			0x0429 },
	{ "afii10044",			0x042A },
	{ "afii10045",			0x042B },
	{ "afii10046",			0x042C },
	{ "afii10047",			0x042D },
	{ "afii10048",			0x042E },
	{ "afii10049",			0x042F },
	{ "afii10050",			0x0490 },
	{ "afii10051",			0x0402 },
	{ "afii10052",			0x0403 },
	{ "afii10053",			0x0404 },
	{ "afii10054",			0x0405 },
	{ "afii10055",			0x0406 },
	{ "afii10056",			0x0407 },
	{ "afii10057",			0x0408 },
	{ "afii10058",			0x0409 },
	{ "afii10059",			0x040A },
	{ "afii10060",			0x040B },
	{ "afii10061",			0x040C },
	{ "afii10062",			0x040E },
#ifdef CORPORATEUSE
	{ "afii10063",			0xF6C4 },	/* Adobe corporate use subarea */
	{ "afii10064",			0xF6C5 },	/* Adobe corporate use subarea */
#endif
	{ "afii10065",			0x0430 },
	{ "afii10066",			0x0431 },
	{ "afii10067",			0x0432 },
	{ "afii10068",			0x0433 },
	{ "afii10069",			0x0434 },
	{ "afii10070",			0x0435 },
	{ "afii10071",			0x0451 },
	{ "afii10072",			0x0436 },
	{ "afii10073",			0x0437 },
	{ "afii10074",			0x0438 },
	{ "afii10075",			0x0439 },
	{ "afii10076",			0x043A },
	{ "afii10077",			0x043B },
	{ "afii10078",			0x043C },
	{ "afii10079",			0x043D },
	{ "afii10080",			0x043E },
	{ "afii10081",			0x043F },
	{ "afii10082",			0x0440 },
	{ "afii10083",			0x0441 },
	{ "afii10084",			0x0442 },
	{ "afii10085",			0x0443 },
	{ "afii10086",			0x0444 },
	{ "afii10087",			0x0445 },
	{ "afii10088",			0x0446 },
	{ "afii10089",			0x0447 },
	{ "afii10090",			0x0448 },
	{ "afii10091",			0x0449 },
	{ "afii10092",			0x044A },
	{ "afii10093",			0x044B },
	{ "afii10094",			0x044C },
	{ "afii10095",			0x044D },
	{ "afii10096",			0x044E },
	{ "afii10097",			0x044F },
	{ "afii10098",			0x0491 },
	{ "afii10099",			0x0452 },
	{ "afii10100",			0x0453 },
	{ "afii10101",			0x0454 },
	{ "afii10102",			0x0455 },
	{ "afii10103",			0x0456 },
	{ "afii10104",			0x0457 },
	{ "afii10105",			0x0458 },
	{ "afii10106",			0x0459 },
	{ "afii10107",			0x045A },
	{ "afii10108",			0x045B },
	{ "afii10109",			0x045C },
	{ "afii10110",			0x045E },
	{ "afii10145",			0x040F },
	{ "afii10146",			0x0462 },
	{ "afii10147",			0x0472 },
	{ "afii10148",			0x0474 },
#ifdef CORPORATEUSE
	{ "afii10192",			0xF6C6 },	/* Adobe corporate use subarea */
#endif
	{ "afii10193",			0x045F },
	{ "afii10194",			0x0463 },
	{ "afii10195",			0x0473 },
	{ "afii10196",			0x0475 },
#ifdef CORPORATEUSE
	{ "afii10831",			0xF6C7 },	/* Adobe corporate use subarea */
	{ "afii10832",			0xF6C8 },	/* Adobe corporate use subarea */
#endif
	{ "afii10846",			0x04D9 },
	{ "afii299",			0x200E },
	{ "afii300",			0x200F },
	{ "afii301",			0x200D },
	{ "afii57381",			0x066A },
	{ "afii57388",			0x060C },
	{ "afii57392",			0x0660 },
	{ "afii57393",			0x0661 },
	{ "afii57394",			0x0662 },
	{ "afii57395",			0x0663 },
	{ "afii57396",			0x0664 },
	{ "afii57397",			0x0665 },
	{ "afii57398",			0x0666 },
	{ "afii57399",			0x0667 },
	{ "afii57400",			0x0668 },
	{ "afii57401",			0x0669 },
	{ "afii57403",			0x061B },
	{ "afii57407",			0x061F },
	{ "afii57409",			0x0621 },
	{ "afii57410",			0x0622 },
	{ "afii57411",			0x0623 },
	{ "afii57412",			0x0624 },
	{ "afii57413",			0x0625 },
	{ "afii57414",			0x0626 },
	{ "afii57415",			0x0627 },
	{ "afii57416",			0x0628 },
	{ "afii57417",			0x0629 },
	{ "afii57418",			0x062A },
	{ "afii57419",			0x062B },
	{ "afii57420",			0x062C },
	{ "afii57421",			0x062D },
	{ "afii57422",			0x062E },
	{ "afii57423",			0x062F },
	{ "afii57424",			0x0630 },
	{ "afii57425",			0x0631 },
	{ "afii57426",			0x0632 },
	{ "afii57427",			0x0633 },
	{ "afii57428",			0x0634 },
	{ "afii57429",			0x0635 },
	{ "afii57430",			0x0636 },
	{ "afii57431",			0x0637 },
	{ "afii57432",			0x0638 },
	{ "afii57433",			0x0639 },
	{ "afii57434",			0x063A },
	{ "afii57440",			0x0640 },
	{ "afii57441",			0x0641 },
	{ "afii57442",			0x0642 },
	{ "afii57443",			0x0643 },
	{ "afii57444",			0x0644 },
	{ "afii57445",			0x0645 },
	{ "afii57446",			0x0646 },
	{ "afii57448",			0x0648 },
	{ "afii57449",			0x0649 },
	{ "afii57450",			0x064A },
	{ "afii57451",			0x064B },
	{ "afii57452",			0x064C },
	{ "afii57453",			0x064D },
	{ "afii57454",			0x064E },
	{ "afii57455",			0x064F },
	{ "afii57456",			0x0650 },
	{ "afii57457",			0x0651 },
	{ "afii57458",			0x0652 },
	{ "afii57470",			0x0647 },
	{ "afii57505",			0x06A4 },
	{ "afii57506",			0x067E },
	{ "afii57507",			0x0686 },
	{ "afii57508",			0x0698 },
	{ "afii57509",			0x06AF },
	{ "afii57511",			0x0679 },
	{ "afii57512",			0x0688 },
	{ "afii57513",			0x0691 },
	{ "afii57514",			0x06BA },
	{ "afii57519",			0x06D2 },
	{ "afii57534",			0x06D5 },
	{ "afii57636",			0x20AA },
	{ "afii57645",			0x05BE },
	{ "afii57658",			0x05C3 },
	{ "afii57664",			0x05D0 },
	{ "afii57665",			0x05D1 },
	{ "afii57666",			0x05D2 },
	{ "afii57667",			0x05D3 },
	{ "afii57668",			0x05D4 },
	{ "afii57669",			0x05D5 },
	{ "afii57670",			0x05D6 },
	{ "afii57671",			0x05D7 },
	{ "afii57672",			0x05D8 },
	{ "afii57673",			0x05D9 },
	{ "afii57674",			0x05DA },
	{ "afii57675",			0x05DB },
	{ "afii57676",			0x05DC },
	{ "afii57677",			0x05DD },
	{ "afii57678",			0x05DE },
	{ "afii57679",			0x05DF },
	{ "afii57680",			0x05E0 },
	{ "afii57681",			0x05E1 },
	{ "afii57682",			0x05E2 },
	{ "afii57683",			0x05E3 },
	{ "afii57684",			0x05E4 },
	{ "afii57685",			0x05E5 },
	{ "afii57686",			0x05E6 },
	{ "afii57687",			0x05E7 },
	{ "afii57688",			0x05E8 },
	{ "afii57689",			0x05E9 },
	{ "afii57690",			0x05EA },
	{ "afii57694",			0xFB2A },	/* alphabetic presentation form */
	{ "afii57695",			0xFB2B },	/* alphabetic presentation form */
	{ "afii57700",			0xFB4B },	/* alphabetic presentation form */
	{ "afii57705",			0xFB1F },	/* alphabetic presentation form */
	{ "afii57716",			0x05F0 },
	{ "afii57717",			0x05F1 },
	{ "afii57718",			0x05F2 },
	{ "afii57723",			0xFB35 },	/* alphabetic presentation form */
	{ "afii57793",			0x05B4 },
	{ "afii57794",			0x05B5 },
	{ "afii57795",			0x05B6 },
	{ "afii57796",			0x05BB },
	{ "afii57797",			0x05B8 },
	{ "afii57798",			0x05B7 },
	{ "afii57799",			0x05B0 },
	{ "afii57800",			0x05B2 },
	{ "afii57801",			0x05B1 },
	{ "afii57802",			0x05B3 },
	{ "afii57803",			0x05C2 },
	{ "afii57804",			0x05C1 },
	{ "afii57806",			0x05B9 },
	{ "afii57807",			0x05BC },
	{ "afii57839",			0x05BD },
	{ "afii57841",			0x05BF },
	{ "afii57842",			0x05C0 },
	{ "afii57929",			0x02BC },
	{ "afii61248",			0x2105 },
	{ "afii61289",			0x2113 },
	{ "afii61352",			0x2116 },
	{ "afii61573",			0x202C },
	{ "afii61574",			0x202D },
	{ "afii61575",			0x202E },
	{ "afii61664",			0x200C },
	{ "afii63167",			0x066D },
	{ "afii64937",			0x02BD },
	{ "agrave",				0x00E0 },
	{ "aleph",				0x2135 },
	{ "alpha",				0x03B1 },
	{ "alphatonos",			0x03AC },
	{ "amacron",			0x0101 },
	{ "ampersand",			0x0026 },
#ifdef CORPORATEUSE
	{ "ampersandsmall",		0xF726 },	/* Adobe corporate use subarea */
#endif
	{ "angle",				0x2220 },
	{ "angleleft",			0x2329 },
	{ "angleright",			0x232A },
	{ "anoteleia",			0x0387 },
	{ "aogonek",			0x0105 },
	{ "approxequal",		0x2248 },
	{ "aring",				0x00E5 },
	{ "aringacute",			0x01FB },
	{ "arrowboth",			0x2194 },
	{ "arrowdblboth",		0x21D4 },
	{ "arrowdbldown",		0x21D3 },
	{ "arrowdblleft",		0x21D0 },
	{ "arrowdblright",		0x21D2 },
	{ "arrowdblup",			0x21D1 },
	{ "arrowdown",			0x2193 },
#ifdef CORPORATEUSE
	{ "arrowhorizex",		0xF8E7 },	/* Apple corporate use subarea */
#endif
	{ "arrowleft",			0x2190 },
	{ "arrowright",			0x2192 },
	{ "arrowup",			0x2191 },
	{ "arrowupdn",			0x2195 },
	{ "arrowupdnbse",		0x21A8 },
#ifdef CORPORATEUSE
	{ "arrowvertex",		0xF8E6 },	/* Apple corporate use subarea */
#endif
	{ "asciicircum",		0x005E },
	{ "asciitilde",			0x007E },
	{ "asterisk",			0x002A },
	{ "asteriskmath",		0x2217 },
#ifdef CORPORATEUSE
	{ "asuperior",			0xF6E9 },	/* Adobe corporate use subarea */
#endif
	{ "at",					0x0040 },
	{ "atilde",				0x00E3 },
	{ "b",					0x0062 },
	{ "backslash",			0x005C },
	{ "bar",				0x007C },
	{ "beta",				0x03B2 },
	{ "block",				0x2588 },
#ifdef CORPORATEUSE
	{ "braceex",			0xF8F4 },	/* Apple corporate use subarea */
#endif
	{ "braceleft",			0x007B },
#ifdef CORPORATEUSE
	{ "braceleftbt",		0xF8F3 },	/* Apple corporate use subarea */
	{ "braceleftmid",		0xF8F2 },	/* Apple corporate use subarea */
	{ "bracelefttp",		0xF8F1 },	/* Apple corporate use subarea */
#endif
	{ "braceright",			0x007D },
#ifdef CORPORATEUSE
	{ "bracerightbt",		0xF8FE },	/* Apple corporate use subarea */
	{ "bracerightmid",		0xF8FD },	/* Apple corporate use subarea */
	{ "bracerighttp",		0xF8FC },	/* Apple corporate use subarea */
#endif
	{ "bracketleft",		0x005B },
#ifdef CORPORATEUSE
	{ "bracketleftbt",		0xF8F0 },	/* Apple corporate use subarea */
	{ "bracketleftex",		0xF8EF },	/* Apple corporate use subarea */
	{ "bracketlefttp",		0xF8EE },	/* Apple corporate use subarea */
#endif
	{ "bracketright",		0x005D },
#ifdef CORPORATEUSE
	{ "bracketrightbt",		0xF8FB },	/* Apple corporate use subarea */
	{ "bracketrightex",		0xF8FA },	/* Apple corporate use subarea */
	{ "bracketrighttp",		0xF8F9 },	/* Apple corporate use subarea */
#endif
	{ "breve",				0x02D8 },
	{ "brokenbar",			0x00A6 },
#ifdef CORPORATEUSE
	{ "bsuperior",			0xF6EA },	/* Adobe corporate use subarea */
#endif
	{ "bullet",				0x2022 },
	{ "c",					0x0063 },
	{ "cacute",				0x0107 },
	{ "caron",				0x02C7 },
	{ "carriagereturn",		0x21B5 },
	{ "ccaron",				0x010D },
	{ "ccedilla",			0x00E7 },
	{ "ccircumflex",		0x0109 },
	{ "cdotaccent",			0x010B },
	{ "cedilla",			0x00B8 },
	{ "cent",				0x00A2 },
#ifdef CORPORATEUSE
	{ "centinferior",		0xF6DF },	/* Adobe corporate use subarea */
	{ "centoldstyle",		0xF7A2 },	/* Adobe corporate use subarea */
	{ "centsuperior",		0xF6E0 },	/* Adobe corporate use subarea */
#endif
	{ "chi",				0x03C7 },
	{ "circle",				0x25CB },
	{ "circlemultiply",		0x2297 },
	{ "circleplus",			0x2295 },
	{ "circumflex",			0x02C6 },
	{ "club",				0x2663 },
	{ "colon",				0x003A },
	{ "colonmonetary",		0x20A1 },
	{ "comma",				0x002C },
#ifdef CORPORATEUSE
	{ "commaaccent",		0xF6C3 },	/* Adobe corporate use subarea */
#endif
#ifdef CORPORATEUSE
	{ "commainferior",		0xF6E1 },	/* Adobe corporate use subarea */
	{ "commasuperior",		0xF6E2 },	/* Adobe corporate use subarea */
#endif
	{ "congruent",			0x2245 },
	{ "copyright",			0x00A9 },
#ifdef CORPORATEUSE
	{ "copyrightsans",		0xF8E9 },	/* Apple corporate use subarea */
	{ "copyrightserif",		0xF6D9 },	/* Adobe corporate use subarea */
#endif
	{ "currency",			0x00A4 },
#ifdef CORPORATEUSE
	{ "cyrBreve",			0xF6D1 },	/* Adobe corporate use subarea */
	{ "cyrFlex",			0xF6D2 },	/* Adobe corporate use subarea */
	{ "cyrbreve",			0xF6D4 },	/* Adobe corporate use subarea */
	{ "cyrflex",			0xF6D5 },	/* Adobe corporate use subarea */
#endif
	{ "d",					0x0064 },
	{ "dagger",				0x2020 },
	{ "daggerdbl",			0x2021 },
#ifdef CORPORATEUSE
	{ "dblGrave",			0xF6D3 },	/* Adobe corporate use subarea */
	{ "dblgrave",			0xF6D6 },	/* Adobe corporate use subarea */
#endif
#ifdef CORPORATEUSE
	{ "dblgrave",			0xF6D6 },	/* Adobe corporate use subarea */
#endif
	{ "dcaron",				0x010F },
	{ "dcroat",				0x0111 },
	{ "degree",				0x00B0 },
	{ "delta",				0x03B4 },
	{ "diamond",			0x2666 },
	{ "dieresis",			0x00A8 },
#ifdef CORPORATEUSE
	{ "dieresisacute",		0xF6D7 },	/* Adobe corporate use subarea */
	{ "dieresisgrave",		0xF6D8 },	/* Adobe corporate use subarea */
#endif
	{ "dieresistonos",		0x0385 },
	{ "divide",				0x00F7 },
	{ "dkshade",			0x2593 },
	{ "dnblock",			0x2584 },
	{ "dollar",				0x0024 },
#ifdef CORPORATEUSE
	{ "dollarinferior",		0xF6E3 },	/* Adobe corporate use subarea */
	{ "dollaroldstyle",		0xF724 },	/* Adobe corporate use subarea */
	{ "dollarsuperior",		0xF6E4 },	/* Adobe corporate use subarea */
#endif
	{ "dong",				0x20AB },
	{ "dotaccent",			0x02D9 },
	{ "dotbelowcomb",		0x0323 },
	{ "dotlessi",			0x0131 },
	{ "dotmath",			0x22C5 },
#ifdef CORPORATEUSE
	{ "dsuperior",			0xF6EB },	/* Adobe corporate use subarea */
#endif
	{ "e",					0x0065 },
	{ "eacute",				0x00E9 },
	{ "ebreve",				0x0115 },
	{ "ecaron",				0x011B },
	{ "ecircumflex",		0x00EA },
	{ "edieresis",			0x00EB },
	{ "edotaccent",			0x0117 },
	{ "egrave",				0x00E8 },
	{ "eight",				0x0038 },
	{ "eightinferior",		0x2088 },
#ifdef CORPORATEUSE
	{ "eightoldstyle",		0xF738 },	/* Adobe corporate use subarea */
#endif
	{ "eightsuperior",		0x2078 },
	{ "element",			0x2208 },
	{ "ellipsis",			0x2026 },
	{ "emacron",			0x0113 },
	{ "emdash",				0x2014 },
	{ "emptyset",			0x2205 },
	{ "endash",				0x2013 },
	{ "eng",				0x014B },
	{ "eogonek",			0x0119 },
	{ "epsilon",			0x03B5 },
	{ "epsilontonos",		0x03AD },
	{ "equal",				0x003D },
	{ "equivalence",		0x2261 },
	{ "estimated",			0x212E },
#ifdef CORPORATEUSE
	{ "esuperior",			0xF6EC },	/* Adobe corporate use subarea */
#endif
	{ "eta",				0x03B7 },
	{ "etatonos",			0x03AE },
	{ "eth",				0x00F0 },
	{ "exclam",				0x0021 },
	{ "exclamdbl",			0x203C },
	{ "exclamdown",			0x00A1 },
#ifdef CORPORATEUSE
	{ "exclamdownsmall",	0xF7A1 },	/* Adobe corporate use subarea */
	{ "exclamsmall",		0xF721 },	/* Adobe corporate use subarea */
#endif
	{ "existential",		0x2203 },
	{ "f",					0x0066 },
	{ "female",				0x2640 },
	{ "ff",					0xFB00 },	/* alphabetic presentation form */
	{ "ffi",				0xFB03 },	/* alphabetic presentation form */
	{ "ffl",				0xFB04 },	/* alphabetic presentation form */
	{ "fi",					0xFB01 },	/* alphabetic presentation form */
	{ "figuredash",			0x2012 },
	{ "filledbox",			0x25A0 },
	{ "filledrect",			0x25AC },
	{ "five",				0x0035 },
	{ "fiveeighths",		0x215D },
	{ "fiveinferior",		0x2085 },
#ifdef CORPORATEUSE
	{ "fiveoldstyle",		0xF735 },	/* Adobe corporate use subarea */
#endif
	{ "fivesuperior",		0x2075 },
	{ "fl",					0xFB02 },	/* alphabetic presentation form */
	{ "florin",				0x0192 },
	{ "four",				0x0034 },
	{ "fourinferior",		0x2084 },
#ifdef CORPORATEUSE
	{ "fouroldstyle",		0xF734 },	/* Adobe corporate use subarea */
#endif
	{ "foursuperior",		0x2074 },
	{ "fraction",			0x2044 },
	{ "franc",				0x20A3 },
	{ "g",					0x0067 },
	{ "gamma",				0x03B3 },
	{ "gbreve",				0x011F },
	{ "gcaron",				0x01E7 },
	{ "gcircumflex",		0x011D },
	{ "gcommaaccent",		0x0123 },
	{ "gdotaccent",			0x0121 },
	{ "germandbls",			0x00DF },
	{ "gradient",			0x2207 },
	{ "grave",				0x0060 },
	{ "gravecomb",			0x0300 },
	{ "greater",			0x003E },
	{ "greaterequal",		0x2265 },
	{ "guillemotleft",		0x00AB },
	{ "guillemotright",		0x00BB },
	{ "guilsinglleft",		0x2039 },
	{ "guilsinglright",		0x203A },
	{ "h",					0x0068 },
	{ "hbar",				0x0127 },
	{ "hcircumflex",		0x0125 },
	{ "heart",				0x2665 },
	{ "hookabovecomb",		0x0309 },
	{ "house",				0x2302 },
	{ "hungarumlaut",		0x02DD },
	{ "hyphen",				0x002D },
	{ "hyphen_",			0x00AD },	/* 			Duplicate sfthyphen */
#ifdef CORPORATEUSE
	{ "hypheninferior",		0xF6E5 },	/* Adobe corporate use subarea */
	{ "hyphensuperior",		0xF6E6 },	/* Adobe corporate use subarea */
#endif
	{ "i",					0x0069 },
	{ "iacute",				0x00ED },
	{ "ibreve",				0x012D },
	{ "icircumflex",		0x00EE },
	{ "idieresis",			0x00EF },
	{ "igrave",				0x00EC },
	{ "ij",					0x0133 },
	{ "imacron",			0x012B },
	{ "infinity",			0x221E },
	{ "integral",			0x222B },
	{ "integralbt",			0x2321 },
#ifdef CORPORATEUSE
	{ "integralex",			0xF8F5 },	/* Apple corporate use subarea */
#endif
	{ "integraltp",			0x2320 },
	{ "intersection",		0x2229 },
	{ "invbullet",			0x25D8 },
	{ "invcircle",			0x25D9 },
	{ "invsmileface",		0x263B },
	{ "iogonek",			0x012F },
	{ "iota",				0x03B9 },
	{ "iotadieresis",		0x03CA },
	{ "iotadieresistonos",	0x0390 },
	{ "iotatonos",			0x03AF },
#ifdef CORPORATEUSE
	{ "isuperior",			0xF6ED },	/* Adobe corporate use subarea */
#endif
	{ "itilde",				0x0129 },
	{ "j",					0x006A },
	{ "jcircumflex",		0x0135 },
	{ "k",					0x006B },
	{ "kappa",				0x03BA },
	{ "kcommaaccent",		0x0137 },
	{ "kgreenlandic",		0x0138 },
	{ "l",					0x006C },
	{ "lacute",				0x013A },
	{ "lambda",				0x03BB },
	{ "lcaron",				0x013E },
	{ "lcommaaccent",		0x013C },
	{ "ldot",				0x0140 },
	{ "less",				0x003C },
	{ "lessequal",			0x2264 },
	{ "lfblock",			0x258C },
	{ "lira",				0x20A4 },
#ifdef CORPORATEUSE
	{ "ll",					0xF6C0 },	/* Adobe corporate use subarea */
#endif
	{ "logicaland",			0x2227 },
	{ "logicalnot",			0x00AC },
	{ "logicalor",			0x2228 },
	{ "longs",				0x017F },
	{ "lozenge",			0x25CA },
	{ "lslash",				0x0142 },
#ifdef CORPORATEUSE
	{ "lsuperior",			0xF6EE },	/* Adobe corporate use subarea */
#endif
	{ "ltshade",			0x2591 },
	{ "m",					0x006D },
	{ "macron",				0x00AF },	/* ISO Latin 1 position */
	{ "macron_",			0x02C9 },	/* 			Duplicate */
	{ "male",				0x2642 },
	{ "minus",				0x2212 },
	{ "minute",				0x2032 },
#ifdef CORPORATEUSE
	{ "msuperior",			0xF6EF },	/* Adobe corporate use subarea */
#endif
	{ "mu",					0x00B5 },	/* ISO Latin 1 position */
	{ "mu_",				0x03BC },	/* 			Duplicate */
	{ "multiply",			0x00D7 },
	{ "musicalnote",		0x266A },
	{ "musicalnotedbl",		0x266B },
	{ "n",					0x006E },
	{ "nacute",				0x0144 },
	{ "napostrophe",		0x0149 },
	{ "ncaron",				0x0148 },
	{ "ncommaaccent",		0x0146 },
	{ "nine",				0x0039 },
	{ "nineinferior",		0x2089 },
#ifdef CORPORATEUSE
	{ "nineoldstyle",		0xF739 },	/* Adobe corporate use subarea */
#endif
	{ "ninesuperior",		0x2079 },
	{ "notelement",			0x2209 },
	{ "notequal",			0x2260 },
	{ "notsubset",			0x2284 },
	{ "nsuperior",			0x207F },
	{ "ntilde",				0x00F1 },
	{ "nu",					0x03BD },
	{ "numbersign",			0x0023 },
	{ "o",					0x006F },
	{ "oacute",				0x00F3 },
	{ "obreve",				0x014F },
	{ "ocircumflex",		0x00F4 },
	{ "odieresis",			0x00F6 },
	{ "oe",					0x0153 },
	{ "ogonek",				0x02DB },
	{ "ograve",				0x00F2 },
	{ "ohorn",				0x01A1 },
	{ "ohungarumlaut",		0x0151 },
	{ "omacron",			0x014D },
	{ "omega",				0x03C9 },
	{ "omega1",				0x03D6 },
	{ "omegatonos",			0x03CE },
	{ "omicron",			0x03BF },
	{ "omicrontonos",		0x03CC },
	{ "one",				0x0031 },
	{ "onedotenleader",		0x2024 },
	{ "oneeighth",			0x215B },
#ifdef CORPORATEUSE
	{ "onefitted",			0xF6DC },	/* Adobe corporate use subarea */
#endif
	{ "onehalf",			0x00BD },
	{ "oneinferior",		0x2081 },
#ifdef CORPORATEUSE
	{ "oneoldstyle",		0xF731 },	/* Adobe corporate use subarea */
#endif
	{ "onequarter",			0x00BC },
	{ "onesuperior",		0x00B9 },
	{ "onethird",			0x2153 },
	{ "openbullet",			0x25E6 },
	{ "ordfeminine",		0x00AA },
	{ "ordmasculine",		0x00BA },
	{ "orthogonal",			0x221F },
	{ "oslash",				0x00F8 },
	{ "oslashacute",		0x01FF },
#ifdef CORPORATEUSE
	{ "osuperior",			0xF6F0 },	/* Adobe corporate use subarea */
#endif
	{ "otilde",				0x00F5 },
	{ "p",					0x0070 },
	{ "paragraph",			0x00B6 },
	{ "parenleft",			0x0028 },
#ifdef CORPORATEUSE
	{ "parenleftbt",		0xF8ED },	/* Apple corporate use subarea */
	{ "parenleftex",		0xF8EC },	/* Apple corporate use subarea */
#endif
	{ "parenleftinferior",	0x208D },
	{ "parenleftsuperior",	0x207D },
#ifdef CORPORATEUSE
	{ "parenlefttp",		0xF8EB },	/* Apple corporate use subarea */
#endif
	{ "parenright",			0x0029 },
#ifdef CORPORATEUSE
	{ "parenrightbt",		0xF8F8 },	/* Apple corporate use subarea */
	{ "parenrightex",		0xF8F7 },	/* Apple corporate use subarea */
#endif
	{ "parenrightinferior",	0x208E },
	{ "parenrightsuperior",	0x207E },
#ifdef CORPORATEUSE
	{ "parenrighttp",		0xF8F6 },	/* Apple corporate use subarea */
#endif
	{ "partialdiff",		0x2202 },
	{ "percent",			0x0025 },
	{ "period",				0x002E },
	{ "periodcentered",		0x00B7 },	/* ISO Latin 1 position */
	{ "periodcentered_",	0x2219 },	/* 			Duplicate */
#ifdef CORPORATEUSE
	{ "periodinferior",		0xF6E7 },	/* Adobe corporate use subarea */
	{ "periodsuperior",		0xF6E8 },	/* Adobe corporate use subarea */
#endif
	{ "perpendicular",		0x22A5 },
	{ "perthousand",		0x2030 },
	{ "peseta",				0x20A7 },
	{ "phi",				0x03C6 },
	{ "phi1",				0x03D5 },
	{ "pi",					0x03C0 },
	{ "plus",				0x002B },
	{ "plusminus",			0x00B1 },
	{ "prescription",		0x211E },
	{ "product",			0x220F },
	{ "propersubset",		0x2282 },
	{ "propersuperset",		0x2283 },
	{ "proportional",		0x221D },
	{ "psi",				0x03C8 },
	{ "q",					0x0071 },
	{ "question",			0x003F },
	{ "questiondown",		0x00BF },
#ifdef CORPORATEUSE
	{ "questiondownsmall",	0xF7BF },	/* Adobe corporate use subarea */
	{ "questionsmall",		0xF73F },	/* Adobe corporate use subarea */
#endif
	{ "quotedbl",			0x0022 },
	{ "quotedblbase",		0x201E },
	{ "quotedblleft",		0x201C },
	{ "quotedblright",		0x201D },
	{ "quoteleft",			0x2018 },
	{ "quotereversed",		0x201B },
	{ "quoteright",			0x2019 },
	{ "quotesinglbase",		0x201A },
	{ "quotesingle",		0x0027 },
	{ "r",					0x0072 },
	{ "racute",				0x0155 },
	{ "radical",			0x221A },
#ifdef CORPORATEUSE
	{ "radicalex",			0xF8E5 },	/* Apple corporate use subarea */
#endif
	{ "rcaron",				0x0159 },
	{ "rcommaaccent",		0x0157 },
	{ "reflexsubset",		0x2286 },
	{ "reflexsuperset",		0x2287 },
	{ "registered",			0x00AE },
#ifdef CORPORATEUSE
	{ "registersans",		0xF8E8 },	/* Apple corporate use subarea */
	{ "registerserif",		0xF6DA },	/* Adobe corporate use subarea */
#endif
	{ "revlogicalnot",		0x2310 },
	{ "rho",				0x03C1 },
	{ "ring",				0x02DA },
#ifdef CORPORATEUSE
	{ "rsuperior",			0xF6F1 },	/* Adobe corporate use subarea */
#endif
	{ "rtblock",			0x2590 },
#ifdef CORPORATEUSE
	{ "rupiah",				0xF6DD },	/* Adobe corporate use subarea */
#endif
	{ "s",					0x0073 },
	{ "sacute",				0x015B },
	{ "scaron",				0x0161 },
#ifdef CORPORATEUSE
	{ "scedilla",			0xF6C2 },	/* Adobe corporate use subarea */
#endif
	{ "scircumflex",		0x015D },
	{ "scommaaccent",		0x015F },
	{ "second",				0x2033 },
	{ "section",			0x00A7 },
	{ "semicolon",			0x003B },
	{ "seven",				0x0037 },
	{ "seveneighths",		0x215E },
	{ "seveninferior",		0x2087 },
#ifdef CORPORATEUSE
	{ "sevenoldstyle",		0xF737 },	/* Adobe corporate use subarea */
#endif
	{ "sevensuperior",		0x2077 },
	{ "shade",				0x2592 },
	{ "sigma",				0x03C3 },
	{ "sigma1",				0x03C2 },
	{ "similar",			0x223C },
	{ "six",				0x0036 },
	{ "sixinferior",		0x2086 },
#ifdef CORPORATEUSE
	{ "sixoldstyle",		0xF736 },	/* Adobe corporate use subarea */
#endif
	{ "sixsuperior",		0x2076 },
	{ "slash",				0x002F },
	{ "smileface",			0x263A },
	{ "space",				0x0020 },
	{ "space_",				0x00A0 },	/* 			Duplicate nbspace */
	{ "spade",				0x2660 },
#ifdef CORPORATEUSE
	{ "ssuperior",			0xF6F2 },	/* Adobe corporate use subarea */
#endif
	{ "sterling",			0x00A3 },
	{ "suchthat",			0x220B },
	{ "summation",			0x2211 },
	{ "sun",				0x263C },
	{ "t",					0x0074 },
	{ "tau",				0x03C4 },
	{ "tbar",				0x0167 },
	{ "tcaron",				0x0165 },
	{ "tcommaaccent",		0x0163 },
	{ "therefore",			0x2234 },
	{ "theta",				0x03B8 },
	{ "theta1",				0x03D1 },
	{ "thorn",				0x00FE },
	{ "three",				0x0033 },
	{ "threeeighths",		0x215C },
	{ "threeinferior",		0x2083 },
#ifdef CORPORATEUSE
	{ "threeoldstyle",		0xF733 },	/* Adobe corporate use subarea */
#endif
	{ "threequarters",		0x00BE },
#ifdef CORPORATEUSE
	{ "threequartersemdash",	0xF6DE },	/* Adobe corporate use subarea */
#endif
	{ "threesuperior",		0x00B3 },
	{ "tilde",				0x02DC },
	{ "tildecomb",			0x0303 },
	{ "tonos",				0x0384 },
	{ "trademark",			0x2122 },
#ifdef CORPORATEUSE
	{ "trademarksans",		0xF8EA },	/* Apple corporate use subarea */
	{ "trademarkserif",		0xF6DB },	/* Adobe corporate use subarea */
#endif
	{ "triagdn",			0x25BC },
	{ "triaglf",			0x25C4 },
	{ "triagrt",			0x25BA },
	{ "triagup",			0x25B2 },
#ifdef CORPORATEUSE
	{ "tsuperior",			0xF6F3 },	/* Adobe corporate use subarea */
#endif
	{ "two",				0x0032 },
	{ "twodotenleader",		0x2025 },
	{ "twoinferior",		0x2082 },
#ifdef CORPORATEUSE
	{ "twooldstyle",		0xF732 },	/* Adobe corporate use subarea */
#endif
	{ "twosuperior",		0x00B2 },
	{ "twothirds",			0x2154 },
	{ "u",					0x0075 },
	{ "uacute",				0x00FA },
	{ "ubreve",				0x016D },
	{ "ucircumflex",		0x00FB },
	{ "udieresis",			0x00FC },
	{ "ugrave",				0x00F9 },
	{ "uhorn",				0x01B0 },
	{ "uhungarumlaut",		0x0171 },
	{ "umacron",			0x016B },
	{ "underscore",			0x005F },
	{ "underscoredbl",		0x2017 },
	{ "union",				0x222A },
	{ "universal",			0x2200 },
	{ "uogonek",			0x0173 },
	{ "upblock",			0x2580 },
	{ "upsilon",			0x03C5 },
	{ "upsilondieresis",	0x03CB },
	{ "upsilondieresistonos",	0x03B0 },
	{ "upsilontonos",		0x03CD },
	{ "uring",				0x016F },
	{ "utilde",				0x0169 },
	{ "v",					0x0076 },
	{ "w",					0x0077 },
	{ "wacute",				0x1E83 },
	{ "wcircumflex",		0x0175 },
	{ "wdieresis",			0x1E85 },
	{ "weierstrass",		0x2118 },
	{ "wgrave",				0x1E81 },
	{ "x",					0x0078 },
	{ "xi",					0x03BE },
	{ "y",					0x0079 },
	{ "yacute",				0x00FD },
	{ "ycircumflex",		0x0177 },
	{ "ydieresis",			0x00FF },
	{ "yen",				0x00A5 },
	{ "ygrave",				0x1EF3 },
	{ "z",					0x007A },
	{ "zacute",				0x017A },
	{ "zcaron",				0x017E },
	{ "zdotaccent",			0x017C },
	{ "zero",				0x0030 },
	{ "zeroinferior",		0x2080 },
#ifdef CORPORATEUSE
	{ "zerooldstyle",		0xF730 },	/* Adobe corporate use subarea */
#endif
	{ "zerosuperior",		0x2070 },
	{ "zeta",				0x03B6 },
};
