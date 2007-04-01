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
#include <string.h>

#define MAXLINE 1024

char *fontlist="fontlist.atm";

char *defaultpath="\\..\\..\\psfonts";

char line[MAXLINE];

int forcedefault=1;		/* always use above defaultpath */

char *getline(FILE *input, char *line, int nlen) {
	int c;
	char *s= line;
	while ((c = getc(input)) != EOF) {
		*s++ = (char) c;
		if (c == '\r') {
			c = getc(input);
			if (c != '\n') ungetc(c, input);
			else *s++ = (char) c;
			break;
		}
		if (c == '\n') break;
		if (s - line >= nlen-3) break;
	}
	*s='\0';
	if (c == EOF && s == line) return NULL;
	else return line;
}

void extension(char *fname, char *str) { /* supply extension if none */
	char *s, *t;
	if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
		strcat(fname, "."); strcat(fname, str);
	}
}

void forceexten(char *fname, char *str) { /* change extension if present */
	char *s, *t;
	if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
		strcat(fname, "."); strcat(fname, str);	
	}
	else strcpy(s+1, str);		/* give it default extension */
}

/* return pointer to file name - minus path - returns pointer to filename */

char *extractfilename(char *pathname) {
	char *s;

	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

/* strips file name, leaves only path */

void stripname(char *file) {
	char *s;
	if ((s = strrchr(file, '\\')) != NULL) *s = '\0';
	else if ((s = strrchr(file, '/')) != NULL) *s = '\0';
	else if ((s = strrchr(file, ':')) != NULL) *(s+1) = '\0';	
	else *file = '\0';
}

int processafile (FILE *output, FILE *input, char *prefix) {
	char *s, *t;
	while(getline(input, line, sizeof(line)) != NULL) {
		if (*line == '%' || *line < ' ') continue;
		if ((s = strstr(line, "/FontName")) != NULL) {
			s += 9;
			while (*s != '\0' && *s <= ' ') s++;
			s++;			/* flush / */
			t = s;
			while (*t != '\0' && *t > ' ') t++;
			*t = '\0';
			while ((t = strchr(s, '-')) != NULL) *t = ' ';
			fprintf(output, "UM,%s,%s\n", prefix, s);
			return 1;	/* succeeded */
		}
	}
	return 0;	/* failed */
}

void uppercase (char *s) {
	int c;
	while ((c = *s) != '\0') {
		if (c >= 'a' && c <= 'z') *s = (char) (c + 'A' - 'a');
		s++;
	}
}

/*	make default argument *.* ? then need fintfirst and findnext ??? */

int main (int argc, char *argv[]) {
	FILE *input, *output;
	char filename[FILENAME_MAX];
	char prefix[FILENAME_MAX];
	long nlen, diskspace, pfbspace, pfmspace;
	int m, pfbs, pfms, nfontnames;
	char *s;
	
	diskspace = pfbspace = pfmspace = 0;
	pfbs = pfms = 0;
	if (argc <= 1) {
		printf("ERROR: specify PFB and PFM files on command line\n");
		exit(1);
	}
	for (m = 1; m < argc; m++) {
		strcpy(filename, argv[m]);
		if (strstr(filename, ".pf") != NULL ||
			strstr(filename, ".PF") != NULL) { 
			if ((input = fopen(filename, "rb")) == NULL) {
				perror(filename);
				continue;
			}
			fseek(input, 0, SEEK_END);
			nlen = ftell(input);
			diskspace += nlen;
			fclose(input);
			if (strstr(filename, ".pfb") != NULL ||
				strstr(filename, ".PFB") != NULL) {
				pfbspace += nlen;
				pfbs++;
			}
			if (strstr(filename, ".pfm") != NULL ||
				strstr(filename, ".PFM") != NULL) {
				pfmspace += nlen;
				pfms++;
			}
		}
	}
	if (pfms != pfbs) {
		printf("ERROR: %d PFBs and %d PFMs (should match)\n", pfbs, pfms);
		exit(1);
	}
	else {
		printf("Seen %d font files (PFB and PFM) total %ld bytes\n",
				pfbs, diskspace);
		if (pfbs > 0 || pfms > 0) {
		printf("\n");
		printf("[SpaceReq]\n");
		printf("System=510,2\n");
		printf("Pfb=%d,%d\n", (int) ((pfbspace+500) / 1000), pfbs);
		printf("Pfm=%d,%d\n", (int) ((pfmspace+500) / 1000), pfms);
		printf("Db=0,1\n");
		printf("ACP=1624,28\n");
		printf("\n");
		}
	}
	if (pfbs == 0 || pfms == 0) {
		printf("ERROR: No PFBs or PFMs\n");
		exit(1);
	}
	strcpy(prefix, argv[1]);
	stripname(prefix);
	if (strcmp(prefix, "") == 0) strcpy(prefix, defaultpath);
	if (forcedefault) strcpy(prefix, defaultpath);
	printf("NOTE: Using %s as path\n", prefix);
	if ((output = fopen(fontlist, "w")) == NULL) {
		perror(fontlist);
		exit(1);
	}
	fprintf(output, "Font List\n");
	fprintf(output, "Version=1.0\n");
	fprintf(output, "Path=%s\n", prefix);
	fprintf(output, "Size=%ld\n", diskspace);	
	fprintf(output, "\n", diskspace);
	fprintf(output,
		"// %d UniMaster fonts, file prefix, FontName with hyphen replaced by space.\n",
			pfbs);
	fprintf(output, "\n", diskspace);
	nfontnames=0;
	for (m = 1; m < argc; m++) {
		strcpy(filename, argv[m]);
		strcpy(prefix, extractfilename(filename));
		if (strstr(prefix, ".pfb") != NULL ||
			strstr(prefix, ".PFB") != NULL) {
			if ((input = fopen(filename, "rb")) == NULL) {
				perror(filename);
				continue;
			}
			if ((s = strchr(prefix, '.')) != NULL) *s = '\0';
			uppercase (prefix);
			if (processafile(output, input, prefix)) nfontnames++;
			fclose(input);
		}
	}
	fclose(output);
	if (nfontnames != pfbs) {
		printf("ERROR: %d FontNames and %d PFBs\n", nfontnames, pfbs);
	}
	return 0;
}
