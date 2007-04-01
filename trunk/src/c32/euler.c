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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>


void solve_RPY_transform (double **m)
{
	double thetax, thetay, thetaz;
    double rg, ra, sg, cg;

//    cc.Rz = atan2 (cc.r4, cc.r1);
	thetaz = atan2 (m[1][0], m[0][0]);

//    SINCOS (cc.Rz, sg, cg);
//	sg = sin (thetaz);
//	cg = cos (thetaz);

	rg = sqrt(m[0][0] * m[0][0] + m[1][0] * m[1][0]);
	if (rg == 0.0) {
		printf("Bad \"rotation\" matrix\n");
		exit(-1);
	}
	sg = m[1][0] / rg;
	cg = m[0][0] / rg;

//    cc.Ry = atan2 (-cc.r7, cc.r1 * cg + cc.r4 * sg);
//	thetay = atan2 (-m[2][0], m[0][0] * cg + m[1][0] * sg);
	thetay = atan2 (-m[2][0], rg);

	ra = sqrt(m[2][1] * m[2][1] + m[2][2] * m[2][2]);
	if (ra == 0.0) {
		printf("Bad \"rotation\" matrix\n");
		exit(-1);
	}

	sa = m[2][1] / ra;
	ca = m[2][2] / ra;
	thetax = atan2 (m[2][1], m[2][2]);
	
//    cc.Rx = atan2 (cc.r3 * sg - cc.r6 * cg, cc.r5 * cg - cc.r2 * sg);
//	thetax = atan2 (m[0][2] * sg - m[1][2] * cg, m[1][1] * cg - m[0][1] * sg);
}

//	in the above rg should equal ra
//	in the above rg and ra should equal |cb|
//	in the above rg and ra should equal sqrt(1 - m[2][0] * m[2][0])

/***********************************************************************\
* This routine simply takes the Euler angles roll, pitch and yaw and fills in *
* the rotation matrix elements r1-r9.                                   *
\***********************************************************************/
void      apply_RPY_transform (double thetax, double thetay, double thetaz) {
    double    sa,              ca,              sb,              cb,              sg,              cg;

//    SINCOS (cc.Rx, sa, ca);
	sa = sin(thetax);
	ca = cos(thetax);
//    SINCOS (cc.Ry, sb, cb);
	sb = sin(thetay);
	cb = cos(thetay);
//    SINCOS (cc.Rz, sg, cg);
	sg = sin(thetaz);
	cg = cos(thetaz);

//    cc.r1 = cb * cg;
	m[0][0] = cb * cg;
//    cc.r2 = cg * sa * sb - ca * sg;
	m[0][1] = cg * sa * sb - ca * sg;
//    cc.r3 = sa * sg + ca * cg * sb;
	m[0][2] = sa * sg + ca * cg * sb;
//    cc.r4 = cb * sg;
	m[1][0] = cb * sg;
//    cc.r5 = sa * sb * sg + ca * cg;
	m[1][1] = sa * sb * sg + ca * cg;
//    cc.r6 = ca * sb * sg - cg * sa;
	m[1][2] = ca * sb * sg - cg * sa;
//    cc.r7 = -sb;
	m[2][0] = -sb;
//    cc.r8 = cb * sa;
	m[2][1] = cb * sa;
//    cc.r9 = ca * cb;
	m[2][2] = ca * cb;
}
