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

typedef struct tagGLYPHMETRICS
{
    UINT    gmBlackBoxX;
    UINT    gmBlackBoxY;
    POINT   gmptGlyphOrigin;
    int     gmCellIncX;
    int     gmCellIncY;
} GLYPHMETRICS, FAR* LPGLYPHMETRICS;

typedef struct tagFIXED
{
    UINT    fract;
    int     value;
} FIXED, FAR* LPFIXED;

typedef struct tagMAT2
{
    FIXED  eM11;
    FIXED  eM12;
    FIXED  eM21;
    FIXED  eM22;
} MAT2, FAR* LPMAT2;

DWORD   WINAPI GetGlyphOutline(HDC, UINT, UINT, GLYPHMETRICS FAR*, DWORD, void FAR*, const MAT2 FAR*);

/* GetGlyphOutline constants */
#define GGO_METRICS        0
#define GGO_BITMAP         1
#define GGO_NATIVE         2

#define TT_POLYGON_TYPE   24

#define TT_PRIM_LINE       1
#define TT_PRIM_QSPLINE    2

typedef struct tagPOINTFX
{
    FIXED x;
    FIXED y;
} POINTFX, FAR* LPPOINTFX;

typedef struct tagTTPOLYCURVE
{
    UINT    wType;
    UINT    cpfx;
    POINTFX apfx[1];
} TTPOLYCURVE, FAR* LPTTPOLYCURVE;

typedef struct tagTTPOLYGONHEADER
{
    DWORD   cb;
    DWORD   dwType;
    POINTFX pfxStart;
} TTPOLYGONHEADER, FAR* LPTTPOLYGONHEADER;



DWORD nLen;						/* space needed for dat a*/
GLYPHMETRICS GM;				/* Glyph Metrics structure */
MAT2 Mat;						/* Transformation matrix */
HLOCAL hBuf=NULL;				/* Memory Handle */
char *lpBuffer;					/* Pointer to Memory Buffer */
LPTTPOLYGONHEADER lpPolyHead=NULL;	/* Pointer to Polygon Header */
LPTTPOLYCURVE lpPolyCurve=NULL;		/* Pointer to Polygon Data */
POINTFX Start;					/* Starting Point and End point */
LPPOINTFX lpPoint;				/* Pointer to outline data knot */
FIXED x, y;

int first=0;

FIXED xll, yll, xur, yur;

void CheckBBox (LPPOINTFX lpPoint) {
	FIXED x, y;
	x = lpPoint->x;	y = lpPoint->y;
	if (first != 0) {
		xll = xur = x;
		yll = yur = y;
		first = 0;
	}
	if (x < xll) xll = x;
	if (x > xur) xur = x;
	if (y < yll) yll = y;
	if (y > yur) yur = y;	
}

/* hDc has font selected */
/* chr is char code of glyph we ant */
Mat.eM11.value=1;	Mat.eM12.value=0;
Mat.eM21.value=0;	Mat.eM22.value=1;
Mat.eM11.fract=0;	Mat.eM12.fract=0;
Mat.eM21.fract=0;	Mat.eM22.fract=0;
nLen = GetGlyphOutline(hDC, chr, GGO_NATIVE, &GM, 0, NULL, &Mat);
hBuf = LocalAlloc(nLen);
lpBuffer = LocalLock(hBuf);
flag = GetGlyphOutline(hDC, chr, GGO_NATIVE, &GM, nLen, lpBuffer, &Mat);
if (flag < 0) winerror("UGH!");

lpPolyHead = (LPTTPOLYGONHEADER) lpBuffer;	/* initialize to first header */
first = 1;
while (lpPolyHead < lpBuffer + nLen) {		/* until all used up */
	cbContour = lPolyHead->cb;	/* length of header structure *and* records */
	if (lpPolyHead->dwType != TT_POLYGON_TYPE) winerror("UGH!");
/*	Start = lpPolyHead->pfxStart; */
	checkbbox(lpPolyHead.pfxStart);
	/* watch out increment is in bytes ... */
	lpPolyCurve = (LPTTPOLYCURVE) (lpPolyHead + sizeof(TTPOLYGONHEADER));
	while (lpPolyCurve < lpPolyHead + cbContour) {
		Type = lpPolyCurve->wType;		/* TT_PRIM_LINE or TT_PRIM_QSPLINE */
		nPoly = lpPolyCurve->cpfx;		/* how many points in contour */
		lpPoint = lpPolyCurve.apfx;
		for (k = 0; k < nPoly; k++) {
/*			x = lpPoint->x; y = lpPoint->y; */
			checkbbox(lpPoint);
			lpPoint++;
		}
		/* watch out increment is in bytes ... */
		lpPolyCurve += sizeof(TTPOLYCURVE) + (nPoly-1) * sizeof (POINTFX);		
	}
	/* watch out increment is in bytes ... */
	lpPolyHead += cbContour;
}

LocalUnlock(hBuf);
LocalFree(hBuf);
