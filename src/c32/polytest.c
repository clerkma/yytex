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

double x0=0.0, y0=0.0, z0=10.0;			/* centroid of polyhedron */

double xa=5.0, ya=0.0, za=0.0;			/* direction of one half edge */
double xb=0.0, yb=5.0, zb=0.0;			/* direction of one half edge */
double xc=0.0, yc=0.0, zc=5.0;			/* direction of one half edge */

double xv[2][2][2];			/* corners of polyhedron in world */
double yv[2][2][2];			/* corners of polyhedron in world */
double zv[2][2][2];			/* corners of polyhedron in world */

double xi[2][2][2];			/* corners of polyhedron in image */
double yi[2][2][2];			/* corners of polyhedron in image */

double f=10.0;				/* principle distance */

void setcorners(void) {
	xv[0][0][0] = x0 - xa - xb - xc;
	yv[0][0][0] = y0 - ya - yb - yc;
	zv[0][0][0] = z0 - za - zb - zc;
	xv[0][0][1] = x0 - xa - xb + xc;
	yv[0][0][1] = y0 - ya - yb + yc;
	zv[0][0][1] = z0 - za - zb + zc;	
	xv[0][1][0] = x0 - xa + xb - xc;
	yv[0][1][0] = y0 - ya + yb - yc;
	zv[0][1][0] = z0 - za + zb - zc;	
	xv[1][0][0] = x0 + xa - xb - xc;
	yv[1][0][0] = y0 + ya - yb - yc;
	zv[1][0][0] = z0 + za - zb - zc;	
	xv[0][1][1] = x0 - xa + xb + xc;
	yv[0][1][1] = y0 - ya + yb + yc;
	zv[0][1][1] = z0 - za + zb + zc;	
	xv[1][1][0] = x0 + xa + xb - xc;
	yv[1][1][0] = y0 + ya + yb - yc;
	zv[1][1][0] = z0 + za + zb - zc;	
	xv[1][0][1] = x0 + xa - xb + xc;
	yv[1][0][1] = y0 + ya - yb + yc;
	zv[1][0][1] = z0 + za - zb + zc;	
	xv[1][1][1] = x0 + xa + xb + xc;
	yv[1][1][1] = y0 + ya + yb + yc;
	zv[1][1][1] = z0 + za + zb + zc;
}

void project (void) {
	xi[0][0][0]
}
