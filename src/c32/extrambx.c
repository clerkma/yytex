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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 1024

char line[MAXLINE];
char oldline[MAXLINE];
char emailadr[MAXLINE];
char username[MAXLINE];

int wantfrom = 1;
int wantto = 1;
int flushsingle = 0;

int verboseflag=0;
int traceflag=0;

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

/* strips file name, leaves only path */

void stripname(char *file) {
	char *s;
	if ((s = strrchr(file, '\\')) != NULL) *s = '\0';
	else if ((s = strrchr(file, '/')) != NULL) *s = '\0';
	else if ((s = strrchr(file, ':')) != NULL) *(s+1) = '\0';	
	else *file = '\0';
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

/* Possible input forms: */
/* From: big91@hotmail.com */
/* From: ari@ivritype.com (Ari Davidow) */
/* From: Teeth Bleaching<chkijw@iampe.com> */
/* From: ESI Tech Support <dknt_support@executive.com> */
/* From: "Leif E. Peterson" <peterson@bcm.tmc.edu> */
/* From: "Leighton Wildrick"<leightonw@locateinkent.com> */

/* To: jmilgram@mit.edu */
/* To: ESI Tech Support Mailing List <esi-tech-support@datadepot.com> */
/* To: "Henry A. Thedick" <hthedick@sprynet.com> */

/* OUTPUT desired form:  */
/* "Leif E. Peterson" <peterson@bcm.tmc.edu> */

void cleanup (char *name) {
	char *s;
	if (*name == '\0') return;
	if (traceflag) printf("CLEANIN:  %s\n", name);
	s = name;
	while (*s == ' ') s++;
	strcpy(name, s);
	s = name + strlen(name) - 1;
	while (s > name && *s == ' ') *s-- = '\0';
	if (*name == '\"') {
		strcpy(name, name+1);
		if ((s = strrchr(name, '\"')) != NULL) *s = '\0';
	}
	if (*name == '\'') {
		strcpy(name, name+1);
		if ((s = strrchr(name, '\'')) != NULL) *s = '\0';
	}
	if (*name == '(') {
		strcpy(name, name+1);
		if ((s = strrchr(name, ')')) != NULL) *s = '\0';
	}
	if (*name == '<') {
		strcpy(name, name+1);
		if ((s = strrchr(name, '>')) != NULL) *s = '\0';
	}
	if (traceflag) printf("CLEANOUT: %s\n", name);
}

int reprocess (FILE *output, char *line, int flag) {
	char *s;
	if (strchr(line, '@') == NULL) return -1;
	if (strstr(line, "YandY") != NULL ||
		strstr(line, "yandy") != NULL ||
		strstr(line, "Y&Y") != NULL ||
		strstr(line, "Y & Y") != NULL ||
		strstr(line, "tiac") != NULL ||
		strstr(line, "bkph") != NULL ||
		strstr(line, "Mail Delivery") != NULL
	   ) return -1;
	if ((s = strrchr(line, ':')) == NULL) return -1;
	s++;
	while (*s == ' ') s++;
	strcpy(line, s);							/* flush leading spaces */
	s = line + strlen(line) - 1;
	while (s > line && *s <= ' ') *s-- = '\0';		/* flush trailing spaces */
	if (traceflag) printf("REPROCESS: %s\n", line);
/* decompose input */
	if ((s = strrchr(line, '<')) != NULL) {
		strcpy(emailadr, s);
		*s = '\0';
		strcpy(username, line);
	}
	else if ((s = strrchr(line, '\"')) != NULL) {
		strcpy(username, s);
		s++;
		*s = '\0';
		strcpy(emailadr, line);
	}
	else if ((s = strrchr(line, '(')) != NULL) {
		strcpy(username, s);
		*s = '\0';
		strcpy(emailadr, line);
	}
	else if ((s = strrchr(line, ' ')) != NULL) {
		strcpy(username, s+1);
		*s = '\0';
		strcpy(emailadr, line);
	}
	else {
		strcpy(emailadr, line);
		strcpy(username, "");
	}
	cleanup(username);
	cleanup(emailadr);

/* compose result */
	strcpy(line, "\t");
	if (*username != '\0') {
		strcpy(line, "\"");
		strcat(line, username);
		strcat(line, "\"");
		strcat(line, " ");
		if (flushsingle) return -1;
	}
	if (*emailadr != '\0') {
		strcat(line, "<");
		strcat(line, emailadr);
		strcat(line, ">");
	}
	strcat(line, "\n");
	if (strcmp(line, oldline) == 0) return -1;
	if (verboseflag) fputs(line, stdout);
	else fputs(line, output);
	strcpy(oldline, line);
	return 0;
}

int processfile(FILE *output, FILE *input) {
	int nlines=0;
	strcpy(oldline, "");
	while (fgets(line, sizeof(line), input) != NULL) {
		if (wantfrom && strncmp(line, "From: ", 6) == 0)
			if (reprocess(output, line, 0) == 0) nlines++;
		if (wantto && strncmp(line, "To: ", 4) == 0)
			if (reprocess(output, line, 1) == 0) nlines++;
	}
	return nlines;
}

int main(int argc, char *argv[]) {
	int m, nlines;
	char infile[FILENAME_MAX], outfile[FILENAME_MAX];
	FILE *input, *output;
	for (m = 1; m < argc; m++) {
		strcpy(infile, argv[m]);
		extension(infile, "mbx");
		printf("Opening %s\n", infile);
		input = fopen(infile, "rb");
		if (input == NULL) {
			perror(infile);
			continue;
		}
		strcpy(outfile, extractfilename(argv[m]));
		forceexten(outfile, "adr");
		output = fopen(outfile, "wb");
		if (output == NULL) {
			perror(outfile);
			exit(1);
		}
		nlines = processfile(output, input);
		fclose (output);
		fclose (input);
		printf("%d items in %s\n", nlines, infile);
		fflush (stdout);
	}
	return 0;
}
