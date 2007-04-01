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

/* softrest.c simulate resistive grid --- annular geometry */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define LAYERS 23
#define SENSORS 72

/* Number of sensors approximately pi * number of layers */
/* tangential step width matches radial step with at half full radius */

double radius[LAYERS+2];	/* radius of each layer */
double area[LAYERS+2];		/* per cell area for each depth */
double c_radial[LAYERS+2];	/* radial conductance */
double c_tangential[LAYERS+2];	/* tangential conductance */

double leakage[SENSORS+2][LAYERS+2];	/* leakage conductance unit area */

double potential[SENSORS+2][LAYERS+2];	/* potential on grid */

double response[SENSORS+2][SENSORS+2];	/* accumulated response */

double layer[SENSORS+2][LAYERS+2];	/* result for leakage at each layer */

double covar[LAYERS+2][LAYERS+2];	/* layer^T * layer */

/* double base_curve[SENSORS+2]; */ /* response to uniform leakage */

double base_curve[SENSORS+2]={ 
102.833, 102.833, 71.0308, 55.2415, 45.2921, 38.2419, 32.9141, 28.724,
25.3379, 22.5474, 20.2129, 18.2371, 16.5489, 15.0954, 13.8358, 12.7385,
11.7782, 10.9348, 10.1918, 9.53575, 8.95539, 8.44143, 7.98607, 7.58271,
7.2258, 6.91062, 6.63315, 6.38998, 6.17825, 5.99548, 5.83964, 5.70901,
5.60218, 5.51802, 5.45564, 5.41441, 5.3939, 5.3939, 5.41441, 5.45564,
5.51802, 5.60218, 5.70901, 5.83964, 5.99548, 6.17825, 6.38998, 6.63315,
6.91062, 7.2258, 7.58271, 7.98607, 8.44143, 8.95539, 9.53575, 10.1918,
10.9348, 11.7782, 12.7385, 13.8358, 15.0954, 16.5489, 18.2371, 20.2129,
22.5474, 25.3379, 28.724, 32.9141, 38.2419, 45.2921, 55.2415, 71.0308,
102.833 };

#define PI 3.141592653

int verboseflag=1;
int wrapflag = 1;
int debugflag=0;
int usebaseflag=1;
int showflag=1;
/* int singleflag=1; */
int singleflag=0;
int layerflag=1;

int usemaxflag=1;
/* int rotateflag=2;*/
int rotateflag=1;
int splitflag=1;
int perturbflag=1; 
/* int perturbflag=0; */
int sinkflag=1;
int baseflag=1;
int relativeflag=1;

int nlayers=LAYERS;
int nsensors=SENSORS;
int times=1000; /* int times=100; */
int interval=100; /* int interval=10; */

/* double leak_aver=0.5; */ 	/* average leakage conductance to ground  */
/* double leak_aver=1.0; */  	/* average leakage conductance to ground  */
double leak_aver=2.0;   	/* *** average leakage conductance to ground  */
/* double leak_aver=4.0;  	/* average leakage conductance to ground  */
/* double leak_aver=8.0; */  	/* average leakage conductance to ground  */

double inject=100.0;	/* injected by source */

double rmax=1.0;	/* outer radius */

double sigma=1.0;	/* resistivity */

double ini_relax=1.90; 	/* 2.0 is optimum for Gauss-Seidel */

double over_relax=1.90; 	/* 2.0 is optimum for Gauss-Seidel */

int ini_method=2;	/* how to initialize - first guess */

double min_change=0.00000001; /* change less than this insignificant */

/* double min_change=0.0000001; */ 	/* speed up for debugging */

/* double min_change=0.000001; */	/* speed up for debugging */

double potential_ini=0.0;	/* intial value for potential - computed */

int i_source=SENSORS/2;		/* position of source on peripheri */

int i_sink=SENSORS/2;		/* position of sink under sensor */
/* int j_sink=20; */			/* position of sink in layer */
/* int j_sink=23; */			/* position of sink in layer */
/* int j_sink=15; */ 			/* position of sink in layer */
/* int j_sink=10; */  			/* position of sink in layer */
/* int j_sink=5; */  			/* position of sink in layer */
int j_sink=1;  			/* position of sink in layer */

/* double sink_leak = 100.0; */	/* conductance at sink */
double sink_leak = 100.0;	/* conductance at sink */

double magnitude = 0.1;		 /* perturbation amplitude */

double ring_fract = 1.0;	/* leakage on outer ring */
				/* NOTE: gets scaled by 1/r */

int p_layer=0;

/* double average_grey=128.0; */	/* when in ratio mode */
double average_grey=200.0;	/* when in ratio mode */
/* double contrast=2.0; */ 	/* expand grey scale of output */
/* double contrast=4.0; */ 	/* expand grey scale of output */
/* double contrast=10.0; */  		/* expand grey scale of output */
double contrast=20.0;  		/* expand grey scale of output */

char *basefile="base.ps";
char *profilefile="profile.ps";
char *responsefile="response.ps";

/* int i_start, i_inc, i_final; */
/* int j_start, j_inc, j_final; */

double change, average;

void boundary_tangential (int, int);

/* The following leaves a hole at the origin */
/* The inner most and outer most are buffer zones */

void setup_radii (int nsensors, int nlayers, int rmax) {
  int k;
/* This version steps linearly in radius */
  for (k=0; k < nlayers+2; k++) {
    radius[k] = rmax * k / (double) nlayers;
/*    if (debugflag) printf("%d\tradius %lg\n", k, radius[k]); */
    area[k] = (2.0 * PI * radius[k]) / (double) nsensors * 
      (rmax / (double) nlayers) ;
  }
  for (k=1; k < nlayers+2; k++) {
    c_tangential[k] = sigma * (rmax / (2.0 * PI * radius[k])) *
      ((double) nsensors / (double) nlayers);
/*    if (debugflag) printf("%d\ttangential %lg\n", k, c_tangential[k]); */
  }
/* c_radial[k] is the conductance of the resistor from layer k to k+1 */
  for (k=0; k < nlayers+1; k++) {
    c_radial[k] = sigma * ((2.0 * PI * radius[k]) / rmax) *
      ((double) nlayers / (double) nsensors);
/*    if (debugflag) printf("%d\tradial %lg\n", k, c_radial[k]); */
  }
  for (k=1; k < nlayers+1; k++) {
    if (debugflag) 
    printf("%d\tr %lg\ta %lg\t1/Rt %lg\t1/Rr %lg\n", 
	   k, radius[k], area[k], c_tangential[k], c_radial[k]);
  }  
}

#ifdef IGNORE
double perturbation (double r, double theta) { /* constant */
  return magnitude * leak_aver;
}
#endif

#ifdef IGNORE
double perturbation (double r, double theta) { /* x */
  return magnitude * leak_aver * r * cos(theta);
}
#endif

#ifdef IGNORE
double perturbation (double r, double theta) { /* y */
  return magnitude * leak_aver * r * sin(theta);
}
#endif

#ifdef IGNORE
double perturbation (double r, double theta) { /* x * y */
  return magnitude * leak_aver * r * r * cos(theta) * sin (theta);
}
#endif

#ifdef IGNORE
double perturbation (double r, double theta) { /* x * x - 1/4 */
  double x = r * cos(theta);
  return magnitude * leak_aver * (x * x - 0.25);
}

#endif

#ifdef IGNORE
double perturbation (double r, double theta) { /* y * y - 1/4 */
  double y = r * sin(theta);
  return magnitude * leak_aver * (y * y - 0.25);
}
#endif


#ifdef IGNORE
double perturbation (double r, double theta) { /* r * r - 1/2 */
  return magnitude * leak_aver * (r * r - 0.5);
}
#endif

double perturbation (double r, double theta) { /* r * r - 1/2 */
  return magnitude * leak_aver * (0.5 - r * r);
}

#ifdef IGNORE
double perturbation (double r, double theta) { /* r^3 cos 3 theta */
  return magnitude * leak_aver * (r * r * r * cos (3.0 * theta));
}
#endif

#ifdef IGNORE
double perturbation (double r, double theta) { /* r^4 cos 4 theta */
  return magnitude * leak_aver * (r * r * r * r * cos (4.0 * theta));
}
#endif

/* The actual resistive grid runs from i = 1 to i = nlayers */
/* The actual resistive grid runs from j = 1 to j = nsensors */
/* A column is appended on each end for wrap around */
/* A row is appended on each end for boundary conditions */

/* do we want leakage on the boundary to be zero ? */

void setup_leakage (int nsensors, int nlayers, double conduct) {
  int i, j;
  double r, theta;
  for (i = 0; i < nsensors+2; i++) {
    for (j = 0; j < nlayers+2; j++) {
      leakage[i][j] = conduct;
    }
  }
  if (layerflag && p_layer != 0) {
    printf("Adding perturbing leakage conductance to layer %d\n", p_layer);
    j = p_layer;
    for (i = 1; i < nsensors+2; i++) {
      leakage[i][j] =  leakage[i][j] + conduct * ring_fract * rmax / radius[j];
    }
  }
  else if (perturbflag) {
    printf("Adding perturbing leakage conductance pattern\n");
    for (i = 0; i < nsensors+2; i++) {
      theta = 2 * PI * (double) i / (double) nsensors;
      for (j = 0; j < nlayers+2; j++) {
	r = rmax * (double) j / (double) nlayers;
	leakage[i][j] =  leakage[i][j] + perturbation(r, theta);
      }
    }
  }
  else if (sinkflag) {
    printf("Adding leakage conductance %lg at %d %d\n",
	   sink_leak,  i_sink, j_sink);
    leakage[i_sink][j_sink] = sink_leak + conduct;
  }
}

void  initial_base(int nsensors, int nlayers) {
  int i, j;
  for (i = 0; i < nsensors+2; i++) {
    potential[i][nlayers] = base_curve[i];
  }
}

void setup_initial0 (int nsensors, int nlayers, double average) {
  int i, j;
  if (verboseflag) printf("Initializing to zero\n");
  for (i = 0; i < nsensors+2; i++) {
    for (j = 0; j < nlayers+2; j++) {
      potential[i][j] = 0.0;
    }
  }
}

void setup_initial1 (int nsensors, int nlayers, double average) {
  int i, j;
  if (verboseflag) printf("Initializing to average value\n");
  for (i = 0; i < nsensors+2; i++) {
    for (j = 0; j < nlayers+2; j++) {
      potential[i][j] = average;
    }
  }
}

void setup_initial2 (int nsensors, int nlayers, double average) {
  int i, j;
  double theta_sourc;
  double x_sourc, y_sourc;
  double r, theta, cos_t, sin_t;
  double x, y, rho;

  if (verboseflag) printf("Initializing to conical slope\n");
  if (splitflag)
/*    theta_sourc = 2.0 * PI * ((double) i_source + 0.5) / (double) nsensors; */
    theta_sourc = 2.0 * PI * ((double) i_source - 0.5) / (double) nsensors;
  else 
    theta_sourc = 2.0 * PI * (double) i_source / (double) nsensors;
  x_sourc = rmax * cos (theta_sourc);
  y_sourc = rmax * sin (theta_sourc);
  for (i = 0; i < nsensors+2; i++) {
    theta = 2.0 * PI * (double) i / (double) nsensors;
    cos_t = cos(theta);
    sin_t = sin(theta);
    for (j = 0; j < nlayers+2; j++) {
      r = radius[j];
      x = r * cos_t;
      y = r * sin_t;
      rho = sqrt ((x - x_sourc) * (x - x_sourc) + (y - y_sourc) * (y - y_sourc));
      potential[i][j] = average * (2.0 * rmax - rho) / rmax ;
    }
  }
}

/* rotate the solution one sensor position to the right */

void rotate (int nsensors, int nlayers) {
  int i, j;
  if (verboseflag) printf("Rotating solution\n");
/*  for (i = 0; i < nsensors+2; i++) { */
  for (i = nsensors+1; i > 0; i--) {
    for (j = 0; j < nlayers+2; j++) {
      potential[i][j] = potential[i-1][j];
    }
  }
  boundary_tangential (nsensors, nlayers);
}

void rotate_smear (int nsensors, int nlayers) {
  int i, j;
  double alpha, beta;
  alpha = 0.5;
  beta = 1.0 - alpha;
  if (verboseflag) printf("Rotating and smearing solution\n");
/*  for (i = 0; i < nsensors+2; i++) { */
  for (i = nsensors+1; i > 0; i--) {
    for (j = 0; j < nlayers+2; j++) {
      potential[i][j] = alpha * potential[i-1][j] + beta * potential[i][j];
    }
  }
  boundary_tangential (nsensors, nlayers);
}

/* for accuracy in final stages, may want to use potential relative to old */

#ifdef IGNORE
double poisson_local (int i, int j, double inject) {
  double total_conduct, total_current;
/*  double old, new; */

/*  old = potential[i,j]; */
/*  if (debugflag && (inject != 0.0)) printf("(%d %d) ", i, j); */
/*  total_conduct =  leakage[i,j] * radius[j] +*/
/*  total_conduct =  leakage[i][j] * radius[j] + */
  total_conduct =  leakage[i][j] * area[j] +
    + c_radial[j-1] + c_radial[j] +
      + c_tangential[j] * 2.0;
  total_current = inject + 
/*    potential[i,j-1] * c_radial[j-1] +  potential[i,j+1] * c_radial[j] + */
    potential[i][j-1] * c_radial[j-1] +  potential[i][j+1] * c_radial[j] +
/*      (potential[i-1,j] + potential[i+1,j]) * c_tangential[j];*/
      (potential[i-1][j] + potential[i+1][j]) * c_tangential[j];

  return total_current / total_conduct;
}
#endif

double poisson_local (int i, int j, double inject) {
  double total_conduct, total_current;
  double old, new; 
  double leak;

  leak = leakage[i][j] * area[j];
  old = potential[i][j];
  total_conduct =  leak + c_radial[j-1] + c_radial[j] + c_tangential[j] * 2.0;
  total_current = inject -  old * leak +
    (potential[i][j-1] - old) * c_radial[j-1] +  
      (potential[i][j+1] - old) * c_radial[j] +
	((potential[i-1][j] - old) + (potential[i+1][j] - old)) 
	  * c_tangential[j];
  new = (total_current / total_conduct) + old;
  return new;
}

/* This returns total change in values as iteration test */
/* Assumes `guard' rows and columns have been set up */
/* make four copies for different i,j order and different directions */

double update_sweep (int nsensors, int nlayers, int i_source, double inject) {
  int i, j;
  double old_poten, new_poten;
  double total_inc=0.0;
  double total_pot=0.0;

/*  i_start = 1;
  i_final = nsensors;
  i_inc = 1; */

/*  j_start = nlayers;
  j_final = 1;
  j_inc = -1; */


  for (i = 1; i < nsensors+1; i++) { 
/*  for (i = i_start; i != i_final + i_inc; i = i + i_inc) { */
/*    for (j = 1; j < nlayers+1; j++) { */
      if (wrapflag && i == nsensors / 2) {
	boundary_tangential (nsensors, nlayers);
      }
    for (j = nlayers; j > 0; j--) {
/*    for (j = j_start; j != j_final + j_inc; j = j + j_inc) { */
      old_poten = potential[i][j];
      new_poten = poisson_local(i, j, 0.0); /* general case */
/*  recompute if happens to be current injection point */
      if (j == nlayers) {
	if (splitflag) {
/*	  if (i == i_source || i == (i_source + 1) ||
	      (i == 1 && i_source == nsensors)) {  */
	  if (i == i_source || i == (i_source - 1) ||
	      (i == nsensors && i_source == 1)) { 
	    if (debugflag) printf("Injecting %lg at %d %d\n", 
				  inject / 2.0, i, j); 
	    new_poten = poisson_local(i, j, inject / 2.0);
	  } 
	}
	else {
	  if (i == i_source) { 
	    if (debugflag) printf("Injecting %lg at %d %d\n", 
				  inject, i, j); 
	    new_poten = poisson_local(i, j, inject);
	  }
	}
      }

/*      total_inc = total_inc +  fabs(new_poten - potential[i,j]); */
      if (new_poten > old_poten)
	total_inc = total_inc +  (new_poten - old_poten);
      else total_inc = total_inc +  (old_poten - new_poten);
/*      potential[i,j] = new_poten; */

      if (over_relax == 1.0) potential[i][j] = new_poten;
      else potential[i][j] = old_poten + (new_poten - old_poten) * over_relax;
/* weight potential by area */
      total_pot = total_pot + new_poten * area[j];
    }
  }
  average = total_pot / (PI * rmax * rmax);
  change = total_inc / (double) (nlayers * nsensors); 
  return change;
}

void boundary_tangential (int nsensors, int nlayers) {
  int i,j;
/* now replicate - wrap around at ends of array */  
  for (j = 0; j < nlayers+2; j++) {
/*    potential[0,j] =  potential[nsensors,j];*/
    potential[0][j] =  potential[nsensors][j];
  }
  for (j = 0; j < nlayers+2; j++) {
/*    potential[nsensors+1,j] =  potential[1,j];*/
    potential[nsensors+1][j] =  potential[1][j];
  }
}

void boundary_radial (int nsensors, int nlayers) {
  int i,j;
  double aver=0.0;

/* now replicate - boundary inside and out */ 
  for (i = 1; i < nsensors+1; i++) {
/*    aver = aver + potential[i,1];*/
    aver = aver + potential[i][1];
  }
  aver = aver / (double) nsensors;
  for (i = 0; i < nsensors+2; i++) {
/*    potential[i,0] = potential[i,1]; */
/*    potential[i,0] = aver; */
    potential[i][0] = aver;
  }
  for (i = 0; i < nsensors+2; i++) {
/*    potential[i,nlayers+1] = potential[i,nlayers];*/
    potential[i][nlayers+1] = potential[i][nlayers];
  }
}

/* each output line is a *radial* line in the resistive grid */

void show_grid (int nsensors, int nlayers) {
  int i, j;
  double slim;
  for (i = 1; i < nsensors+1; i++) { 
/*  for (i = 0; i < nsensors+2; i++) { */
/*    if (i != 1 && i != i_source) continue; */
    if (i != 1 && i != i_source/2 && i != i_source) continue;
    printf("Radial scan at %d\n", i);
    for (j = 1; j < nlayers+1; j++) { 
/*    for (j = 0; j < nlayers+2; j++) { */
/*      slim = potential[i,j];*/
      slim = potential[i][j];
      slim = (double) ((int) (slim * 100000.0)) / 100000.0;
/*      printf("(%d %d) %lg\t", i, j, slim); */
      printf("%lg\t", slim); 
    }
/*    putc('\n', stdout); */
    printf("\n*********************************************\n");
  }
}

/* note: source is at 1 and 2 */

/* prints out something we can use in PS to draw a graph */

void show_sensor (FILE *output, int nsensors, int nlayers) {
  int i;
  double slim;

  printf("SENSOR OUTPUTS:\n");
  putc('[', output);
  for (i = 1; i < nsensors + 1; i++) {
/*  for (i = 2; i < nsensors + 2; i++) { */
      if (relativeflag) slim = potential[i][nlayers] / base_curve[i];
      else slim = potential[i][nlayers];

/*      slim = (double) ((int) (slim * 10000.0)) / 10000.0;  */
      fprintf(output, "%lg ", slim);
    }
  putc(']', output);
  putc('\n', output);
}

double max_response (int nsensors) {
  double max_value = response[1][1];
  int i, j;
  for (i = 1; i < nsensors + 1; i++) {
    for (j = 1; j < nsensors + 1; j++) {
      if (response[i][j] > max_value) max_value = response[i][j];
    }
  }
  return max_value;
}

double min_response (int nsensors) {
  double min_value = response[1][1];
  int i, j;
  for (i = 1; i < nsensors + 1; i++) {
    for (j = 1; j < nsensors + 1; j++) {
      if (response[i][j] < min_value) min_value = response[i][j];
    }
  }
  return min_value;
}

/* print out something we can use as an image in PS */

void writehex (int num, FILE *output) {
  int c, d;
  c = (num >> 4) & 15;
  d = num & 15;
  if (c > 9) c = c + 'A' - 10;
  else c = c + '0';
  putc(c, output);
  if (d > 9) d = d + 'A' - 10;
  else d = d + '0';
  putc(d, output);
}

writeimagehead(FILE *output, int nsensors) {
  fprintf(output, "%%!PS-Adobe-2.0\n");
  fprintf(output, "/inch{72 mul}def\n");
  fprintf(output, "/freq 53 def /angle 45 def /eta 0.08 def\n");
  fprintf(output, "freq angle {1 add 180 mul cos 1 eta add mul exch 2 add 180 mul cos 1 eta sub\n");
  fprintf(output, "mul add 2 div} bind setscreen %% (c) bkph 1989\n");
  fprintf(output, "/bufstr %d string def\n", nsensors);
  fprintf(output, "1 inch 1 inch translate\n");
  fprintf(output, "4 inch 4 inch scale\n");
  fprintf(output, "%d %d 8\n", nsensors, nsensors);
  fprintf(output, "[%d 0 0 %d neg 0 %d]\n", nsensors, nsensors, nsensors);
  fprintf(output, "{currentfile bufstr readhexstring pop}\n");
  fprintf(output, "image\n");
}

writeimagetail(FILE *output) {
  fprintf(output, "showpage\n");
}

void show_response (FILE *output, int nsensors) {
  double scalefactor;
  double value;
  int i_offset;
  int i, j;
  int grey;

  writeimagehead(output, nsensors);
  if (baseflag) scalefactor = average_grey;
  else if (usemaxflag) scalefactor = 256.0 / max_response (nsensors);
  else scalefactor = 10.0 / min_response (nsensors);
  for (i = 1; i < nsensors + 1; i++) {
    for (j = 1; j < nsensors + 1; j++) {
      if (baseflag) {
	if (i >= j) i_offset = i - j + 1 + 1;
	else i_offset = i + nsensors - j + 1 + 1;
	value = response[i][j] / base_curve[i_offset];
	if (contrast != 1.0) value = contrast * (value - 1.0) + 1.0;
/*	printf("%d %d (offset %d) response %lg base %lg ratio %lg\n",
       i, j, i_offset, response[i][j], base_curve[i_offset], value); */
      }
      else value = response[i][j];
/*      grey = (int) (response[i][j] * scalefactor + 0.5); */
      grey = (int) (value * scalefactor + 0.5);
      if (grey < 0) grey = 0;
      else if (grey > 255) grey = 255;
      writehex(grey, output);
    }
    putc('\n', output);
  }
  writeimagetail(output);
}

void show_base(FILE *output, int nsensors, int nlayers) {
  int i;
  putc('[', output);
  for (i = 1; i < nsensors+2; i++)
    fprintf(output, "%lg ", base_curve[i]);
  putc(']', output);
  putc('\n', output);
}


void poisson_iter (int nsensors, int nlayers, int times, int interval) {
  int k, temp;
  double old_change = 100000000.0;

  boundary_tangential(nsensors, nlayers);
  boundary_radial(nsensors, nlayers);

/*  i_start = 1;
  i_final = nsensors;
  i_inc = 1; */

/*  j_start = nlayers;
  j_final = 1;
  j_inc = -1;*/

  for (k = 0; k < times; k++) {
    update_sweep(nsensors, nlayers, i_source, inject);
/*    boundary(nsensors, nlayers);  */
    boundary_tangential(nsensors, nlayers);
    boundary_radial(nsensors, nlayers); 
/*    if (change > old_change && change > 0.01) { */
    if (change > old_change && (change > 0.01 || change > old_change * 1.2)) {
      if (over_relax > 1.8) {
	over_relax = over_relax - 0.01;
	printf("Reducing over-relaxation to %lg (step %d change %lg)\n", 
	     over_relax, k, change);
      }
    }
    old_change = change;
    if (change < average * min_change) {
      printf("Skipping out since change is now less than %lg\n", 
	     average * min_change);
      break;
    }
    if (k % interval == 0) 
      printf("Iteration %d average %lg change %lg\n", k, average, change);
  }
  printf("Iteration %d average %lg change %lg\n", k, average, change);
}


#ifdef IGNORE
/* we don't want to change sweep direction if we use over relaxation ... */
/*    if (k % 2 == 0) {
      i_inc = - i_inc; temp = i_final; i_final = i_start; i_start = temp;
    } */
/*    if (k % 2 == 0) {
      j_inc = - j_inc; temp = j_final; j_final = j_start; j_start = temp;
    } */
#endif

void multiply_transpose (void) {
  int i, j, k;
  double sum;
  printf("Multiplying calibration data matrix by transpose\n");
  for (i = 1; i < nlayers + 1; i++) {
    for (j = 1; j < nlayers + 1; j++) {
      sum = 0.0;
      for (m = 1; m < nsensors + 1; m++) {
	sum = sum + layer[m][i] *  layer[m][j];
      }
      covar[i][j] = sum;
    }
  }
}

int commandline (int argc, char *argv[], int firstarg) {
  char *s;
  int c;
  while (firstarg < argc && *argv[firstarg] == '-') {
    s = argv[firstarg];
    s++;
    c = *s++;
    s++; /* step over the = */
    if (c == 'N') {
      times = atoi(s);
      if (verboseflag) printf("%s N = %d\n", argv[firstarg], times);
    }
    else if (c == 'M') {
      interval = atoi(s);
      if (verboseflag) printf("%s M = %d\n", argv[firstarg], interval);
    }
    else if (c == 'L') {
      nlayers = atoi(s);
      if (nlayers > LAYERS) nlayers = LAYERS;
      if (verboseflag) printf("%s L = %d\n", argv[firstarg], nlayers);
    }
    else if (c == 'S') {
      nsensors = atoi(s);
      if (nsensors > SENSORS) nsensors = SENSORS;
      if (verboseflag) printf("%s S = %d\n", argv[firstarg], nlayers);
    }
    else if (c == 'I') {
      ini_method = atoi(s);
      if (verboseflag) printf("%s I = %d\n", argv[firstarg], ini_method);
    }
    else if (c == 'r') {
      rotateflag = atoi(s);
      if (verboseflag) printf("%s r = %d\n", argv[firstarg], rotateflag);
    }
    else if (c == 'A') {
      sscanf (s, "%lg", &over_relax);
      if (verboseflag) printf("%s A = %lg\n", argv[firstarg], over_relax);
    }
    else if (c == 'C') {
      sscanf (s, "%lg", &leak_aver);
      if (verboseflag) printf("%s C = %lg\n", argv[firstarg], leak_aver);
    }
/*    else if (c == 'R') singleflag = 0; */
    else if (c == 'R') singleflag = 1;
    else if (c == 'b') debugflag = 1;
    else if (c == 'v') verboseflag = 0;
    else if (c == 's') showflag = 0;
    else if (c == 'h') splitflag = 0;
    else printf("Don't understand %s\n", argv[firstarg]);

    firstarg++;
  }
return firstarg;
}

int main (int argc, char *argv[]) {
  int firstarg=1;
  char *s;
  int m, n;
  int c, i;
  FILE *output;
  int old_sinkflag, old_perturbflag;
  double old_min_change;

/*  if (argc > 1) times = atoi(argv[1]); */
/*  if (argc > 2) interval = atoi(argv[2]); */
  
  firstarg = commandline (argc, argv, firstarg);

  printf("We will iterate %d times\n", times);

  printf("The grid has %d layers of %d sensors\n",
	 nlayers, nsensors);

  setup_radii(nsensors, nlayers, rmax);
  printf("We have set up the resistive network\n");

  setup_leakage (nsensors, nlayers, leak_aver);

  potential_ini = inject / (PI * rmax * rmax * leak_aver);
  printf("The guess is that the average potential will be %lg\n", 
	 potential_ini);

  if (ini_method==1)
      setup_initial1 (nsensors, nlayers, potential_ini);
  else if (ini_method==2)
      setup_initial2 (nsensors, nlayers, potential_ini);
  else {
    setup_initial0 (nsensors, nlayers, potential_ini);
    printf("Invalid intitial method %d\n", ini_method);
  }
  if (usebaseflag) initial_base(nsensors, nlayers);
    
  printf("We have set up the initial values\n");

  if (singleflag) {
    printf("Doing single interation\n");
    poisson_iter (nsensors, nlayers, times, interval);

    printf("Completed the iterations\n");
    show_grid(nsensors, nlayers);
    show_sensor (stdout, nsensors, nlayers);

    output = fopen(profilefile, "w");
    if (output == NULL) {
      perror(profilefile);
      exit(0);
    }
    show_sensor (output, nsensors, nlayers);
    fclose (output);

    exit(0);
  }

  if (baseflag) {
    old_sinkflag = sinkflag;
    old_perturbflag = perturbflag;
    sinkflag=0;
    perturbflag = 0;
    setup_leakage (nsensors, nlayers, leak_aver);
    i_source = 1;
    printf("Going to set base reference\n");
/* solve it once to get base_response */
    old_min_change = min_change;
    min_change=0.0;   /* speed up for debugging */
    poisson_iter (nsensors, nlayers, times, interval);
/*    for (i = 1; i < nsensors+1; i++) { */
    for (i = 0; i < nsensors+2; i++) {
      base_curve[i] = potential[i][nlayers];
/*      printf("%d base %lg\n", i, base_curve[i]); */
    } 
    min_change = old_min_change;
    sinkflag=old_sinkflag;
    perturbflag = old_perturbflag;
    printf("Finished setting up base reference\n");
    output = fopen(basefile, "w");
    if (output == NULL) {
      perror(basefile);
      exit(0);
    }
    show_base(stdout, nsensors, nlayers);
    show_base(output, nsensors, nlayers);
    fclose (output);
  }

  if (layerflag) {
    output = fopen(profilefile, "w");
    if (output == NULL) {
      perror(profilefile);
      exit(0);
    }
    for (m = 1; m < nlayers+1; m++) {
      p_layer = m;
      setup_leakage(nsensors, nlayers, leak_aver);
      printf("Doing single interation\n");
      poisson_iter (nsensors, nlayers, times, interval);
      
      printf("Completed the iterations for layer %d\n", m);
/*    show_grid(nsensors, nlayers); */
      show_sensor (stdout, nsensors, nlayers);
      show_sensor (output, nsensors, nlayers);
/*	possibly subtract 1.0 ??? */
      for (i = 0; i < nsensors+2; i++) {
	layer[i][m]=potential[i][nlayers] / base_curve[i];
      }
    }
    fclose (output);
    multiple_transpose();
    exit(0);
  }

/*  sinkflag = 0; */	/* debugging only */
  setup_leakage (nsensors, nlayers, leak_aver);

  for (m = 1; m < nsensors+1; m++) {

    over_relax = ini_relax;
    i_source = m;
    poisson_iter (nsensors, nlayers, times, interval);
  
    if (verboseflag)
    printf("Completed the iterations for source position %d\n", i_source);
/*    show_grid(nsensors, nlayers); */
/*    show_sensor (nsensors, nlayers); */
    for (n = 1; n < nsensors+1; n++) response[m][n] = potential[n][nlayers];
    if (rotateflag == 2) rotate_smear (nsensors, nlayers);
    else if (rotateflag == 1) rotate (nsensors, nlayers);
  }

  output = fopen(responsefile, "w");
  if (output == NULL) {
    perror(responsefile);
    exit(0);
  }
  show_response (output, nsensors);
  fclose (output);

  exit(0);

}

