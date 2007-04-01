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

/* Make PS file of changed pages only - compare new PS file to old PS file */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXFILENAME 128

#define MAXLINE 8192

char line[MAXLINE], oldline[MAXLINE], newline[MAXLINE];

#define MAXPAGES 1024

long pagepos[MAXPAGES];

int pageones[MAXPAGES], pagetwos[MAXPAGES];

int numpage;			/* index into the above */

/* long trailerpos; */

int verboseflag=0;
int detailflag=0;
int traceflag=0;
int dotsflag=1;			/* show progress using dots */
int skipcheck=1;		/* don't compare files first to see if different */
int tryfgets=1;			/* try and use fgets (for speed?) first */
int adjustsequence=1;	/* use new sequential page number in output */
int usepagenone=1;		/* non-zero => use first of the two page numbers */
						/* which is the `logical' page number */
						/* otherwise use the second number which `physical'*/

int followorder=1;		/* pages in order specified */

int outputflag=0;		/* next arg is output file name */
int pagesflag=0;		/* next arg is page list */

int nesting;			/* nesting level of inserted `Documents' */
int resource;			/* font resources seen inside pages */
int npages;				/* number of `difference' pages output */

/* int pageindex; */	/* number of PS pages in input file */

int pageone, pagetwo;

int oldpageone, oldpagetwo;
int newpageone, newpagetwo;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char *programversion =
	"CHANGED: DSC compliant PS file extraction utility, version 1.1";

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

char *copyright = "\
Copyright (C) 1993, 1994  Y&Y, Inc.  (978) 371-3286  http://www.YandY.com\
";

/* Copyright (C) 1993, 1994  Y&Y, Inc. All rights reserved. (508) 371-3286\ */

/* #define COPYHASH 16545456 */
#define COPYHASH 5036743

/* WARNING:  CHANGE COPYHASH if COPYRIGHT message is changed !!! */

int wantcpyrght=1;		/* want copyright message in there */

char *outdefault="changed.ps";			/* default output file */

char *pagelist="";		/* list of pages in form 2,3-5,10-12 */

int selectpages=0;		/* selected pages 1994/July/1 */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* This is an fgets that deals with \n, \r, \r + \n */

char *fgetline(char *line, int len, FILE *input) {
	int k, c;
	char *s=line;

	if (tryfgets) {			/*	try fgets first to see whether it works */
		s = fgets(line, len, input);
		if (s == NULL) 	return NULL;				/* hit EOF */
		if (strlen(line) < (unsigned int) (len-1))
			return line;	/* fgets worked */
		if (traceflag) printf("fgets failed\n");
	}

	for (k = 0; k < len-1; k++) {
		c = getc(input);
		if (c == EOF) {
			*s = '\0';
			if (k == 0) return NULL;		/* nothing left */
			else return line;				/* still something chars left */
		}		
		*s++ = (char) c;
		if (c == '\n') {
			*s = '\0';
			return line;
		}
		else if (c == '\r') {
			c = getc(input);
			if (c == '\n') *s++ = (char) c;
			else ungetc(c, input);
			*s = '\0';
			return line;
		}
	}
	*s = '\0';
	if (traceflag) printf("Input line too long (> %d)\n", MAXLINE);
	return line;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void extension(char *fname, char *ext) { /* supply extension if none */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); 
			strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) { /* change extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); 
			strcat(fname, ext);
	}
	else strcpy(s+1, ext);		/* give it default extension */
}

char *stripname(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#define MAXPAGERANGE 128

int pageindex;				/* number of page ranges */

int beginpage[MAXPAGERANGE], endpage[MAXPAGERANGE];

/* list of pages in form 2,3-5,10-12 */

char *delimiters=",.;:+_|~^/#!*";

int analpages (char *pages) {
	char *s;
	int page, bpage, epage;

	pageindex = 0;
	s = strtok (pages, delimiters);
	while (s != NULL) {
		if (pageindex >= MAXPAGERANGE) {
			fprintf(stderr, "ERROR: Too many page ranges: %s\n", s);
			break;
		}
		if (sscanf(s, "%d-%d", &bpage, &epage) == 2) {
			beginpage[pageindex] = bpage;
			endpage[pageindex] = epage;
			pageindex++;
		}
		else if (sscanf(s, "%d", &page) == 1) {
			beginpage[pageindex] = page;
			endpage[pageindex] = page;
			pageindex++;
		}
		else fprintf(stderr, "Error in page range: %s", s);
		s = strtok (NULL, delimiters);
	}
	if (traceflag) {
		int k;
		printf("There %s %d page range%s\n",
			(pageindex == 1) ? "is" : "are", pageindex,
				(pageindex == 1) ? "" : "s");
		for (k = 0; k < pageindex; k++)
			printf("%d - %d\n", beginpage[k], endpage[k]);
	}
	selectpages = 1;
	return pageindex;
}

int desired (int pageno) {
	int k;
	for (k = 0; k < pageindex; k++) {
		if (beginpage[k] <= pageno && endpage[k] >= pageno)
			 return 1;			/* we do want this page */
	}
	return 0;		/* we don't want it, its in no range */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* skip to point just before next %%Page: (ignoring first line) */
/* returns EOF if hits end of file before meeting next %%Page: */
/* otherwise returns second page number in %%Page: <num-1> <num-2> */

int skippage (FILE *input, int first) {
	long present;
	char *s;

	nesting = 0;
	if (fgetline(line, MAXLINE, input) == NULL) return EOF;
	present = ftell(input);
	if (traceflag) printf("skippage start: %s", line);
	while ((s = fgetline(line, MAXLINE, input)) != NULL) {
		if (strncmp(line, "%%", 2) == 0) {
			if (strncmp(line, "%%BeginDocument", 15) == 0) nesting++; 
			if (strncmp(line, "%%EndDocument", 13) == 0) nesting--;
			if (nesting == 0) {
				if (strncmp(line, "%%Page:", 7) == 0) break;
				if (strncmp(line, "%%Trailer", 9) == 0) break;
			}
			if (first == 0 && nesting == 0) {
				if (strncmp(line, "%%BeginFont", 11) == 0 ||
					strncmp(line, "%%BeginResource: font", 21) == 0) {
					if (verboseflag) puts(line);
					resource++;
				}
			}
		}
		present = ftell (input);		/* remember line start */
	}
	if (s == NULL) return EOF;
	if (traceflag) printf ("skippage to:    %s", line);
	fseek (input, present, SEEK_SET);	/* step back to start of line */
	if (nesting != 0)					/* impossible ! */
		fprintf(stderr, "ERROR: invalid nesting level at end of page\n");
	if (strncmp(line, "%%Page:", 7) == 0) {
		pageone = 0; pagetwo = 0;
		if (sscanf (line+7, "%d %d", &pageone, &pagetwo) < 1) {
			printf("Sorry, don't understand: %s", line);
		}
		return pagetwo;
	}
	else return 0;
}

/* copy up to point just before next %%Page: (always copy first line) */
/* returns EOF if hits end of file before meeting next %%Page: */
/* otherwise returns second page number in %%Page: <num-1> <num-2> */

int copypage (FILE *output, FILE *input, int first) {
	long present;
	char *s;
	int pageno, n;

	nesting = 0;
	if (first == 0) npages++;			/* count pages */
	if (fgetline(line, MAXLINE, input) == NULL) return EOF;
	if (adjustsequence) {				/* use new seq page number */
		if (strncmp(line, "%%Page:", 7) == 0) {
			if (sscanf(line+7, "%d %n", &pageno, &n) > 0) {
				if (strstr(line, "\r\n") != NULL)
					sprintf(line+7+n, "%d\r\n", npages);
				else sprintf(line+7+n, "%d\n", npages);
			}
		}
	}
	fputs(line, output);
	if (traceflag) printf("copypage start: %s", line);
	present = ftell(input);
	while ((s = fgetline(line, MAXLINE, input)) != NULL) {
		if (strncmp(line, "%%", 2) == 0) {
			if (strncmp(line, "%%BeginDocument", 15) == 0) nesting++; 
			if (strncmp(line, "%%EndDocument", 13) == 0) nesting--;
			if (nesting == 0) {
				if (strncmp(line, "%%Page:", 7) == 0) break;
				if (strncmp(line, "%%Trailer", 9) == 0) break;
			}
			if (first == 0 && nesting == 0) {
				if (strncmp(line, "%%BeginFont", 11) == 0 ||
					strncmp(line, "%%BeginResource: font", 21) == 0) {
					if (verboseflag) puts(line);
					resource++;
				}
			}
		}
		fputs(line, output);
		present = ftell(input);		/* remember page start */
	}
	if (s == NULL) return EOF;
	if (traceflag) printf ("copypage to:    %s", line);
	fseek (input, present, SEEK_SET);	/* step back to start of line */
	if (nesting != 0)					/* impossible ! */
		fprintf(stderr, "ERROR: invalid nesting level at end of page\n");
	if (strncmp(line, "%%Page:", 7) == 0) {
		pageone = 0; pagetwo = 0;
		if (sscanf (line+7, "%d %d", &pageone, &pagetwo) < 1) {
			printf("Sorry, don't understand: %s", line);
		}
		return pagetwo;
	}
	else return 0;
}

/* compares up to point just before next %%Page: (ignore first line) */
/* returns EOF if hits end of file while files still match */
/* returns EOF if new file shorter than old */
/* returns 0 if exact match */
/* returns 1 of not match - or if new file is longer than old */

int comparepage (FILE *oldfile, FILE *newfile) {
	long oldpresent, newpresent;
	char *olds, *news;

	nesting = 0;
	olds = fgetline(oldline, MAXLINE, oldfile);
	news = fgetline(newline, MAXLINE, newfile);
	if (olds == NULL && news == NULL) return EOF;	/* match at EOF */
	if (olds == NULL) return 1;			/* new longer than old */
	if (news == NULL) return EOF;		/* new shorter than old */
	oldpresent = ftell(oldfile);
	newpresent = ftell(newfile);
	if (traceflag) printf("comparepage from old: %s", oldline);
	if (traceflag) printf("comparepage from new: %s", newline);
	for (;;) {
		olds = fgetline(oldline, MAXLINE, oldfile);
		news = fgetline(newline, MAXLINE, newfile);
		if (olds == NULL || news == NULL) break;
/* This needs work ! or and and */
		if (strncmp(oldline, "%%", 2) == 0) {
			if (strncmp(oldline, "%%BeginDocument", 15) == 0) nesting++; 
			if (strncmp(oldline, "%%EndDocument", 13) == 0) nesting--;
			if (nesting == 0) {
				if (strncmp(oldline, "%%Page:", 7) == 0) break;
				if (strncmp(oldline, "%%Trailer", 9) == 0) break;
			}
			if (nesting == 0) {
				if (strncmp(oldline, "%%BeginFont", 11) == 0 ||
					strncmp(oldline, "%%BeginResource: font", 21) == 0) {
					if (verboseflag) puts(oldline);
					resource++;
				}
			}
		}
		if (strncmp(newline, "%%", 2) == 0) {
			if (strncmp(newline, "%%BeginDocument", 15) == 0) nesting++; 
			if (strncmp(newline, "%%EndDocument", 13) == 0) nesting--;
			if (nesting == 0) {
				if (strncmp(newline, "%%Page:", 7) == 0) break;
				if (strncmp(newline, "%%Trailer", 9) == 0) break;
			}
			if (nesting == 0) {
				if (strncmp(newline, "%%BeginFont", 11) == 0 ||
					strncmp(newline, "%%BeginResource: font", 21) == 0) {
					if (verboseflag) puts(newline);
					resource++;
				}
			}
		}
		oldpresent = ftell(oldfile);		/* remember page start */
		newpresent = ftell(newfile);		/* remember page start */
		if (strcmp(newline, oldline) != 0) {
			if (traceflag) {
				printf("old line: %s", oldline);
				printf("new line: %s", newline);
			}
			return 1;					/* found difference */
		}
	}
	if (olds == NULL && news == NULL) return EOF;	/* match & EOF */
	if (olds == NULL) return 1;			/* new longer than old */
	if (news == NULL) return EOF;		/* new shorter than old */
	if (nesting != 0) 					/* impossible ! */
		fprintf(stderr, "ERROR: invalid nesting level at end of page\n");
	fseek (oldfile, oldpresent, SEEK_SET);	/* step back to start of line */
	fseek (newfile, newpresent, SEEK_SET);	/* step back to start of line */
	if (traceflag) printf ("comparepage to old:   %s", oldline);
	if (strncmp(oldline, "%%Page:", 7) == 0) {
		oldpageone = 0; oldpagetwo = 0;
		if (sscanf (oldline+7, "%d %d", &oldpageone, &oldpagetwo) < 1) {
			printf("Sorry, don't understand: %s", oldline);
		}
	}
	if (traceflag) printf ("comparepage to new:   %s", newline);
	if (strncmp(newline, "%%Page:", 7) == 0) {
		newpageone = 0; newpagetwo = 0;
		if (sscanf (newline+7, "%d %d", &newpageone, &newpagetwo) < 1) {
			printf("Sorry, don't understand: %s", newline);
		}
	}	
	return 0;							/* pages match exactly */
}

/* get page numbers of pages we are currently faced with */

#ifdef IGNORE
void getpagenos(FILE *oldfile, FILE *newfile) {
	long oldpresent, newpresent;
	char *olds, *news;

	oldpresent = ftell(oldfile);
	newpresent = ftell(newfile);
	olds = fgetline(oldline, MAXLINE, oldfile);
	news = fgetline(newline, MAXLINE, newfile);
/*	if (olds == NULL || news == NULL) break; */
	if (strncmp(oldline, "%%Page:", 7) == 0) {
		oldpageone = 0; oldpagetwo = 0;
		if (sscanf (oldline+7, "%d %d", &oldpageone, &oldpagetwo) < 1) {
			printf("Sorry, don't understand: %s", oldline);
		}
	}
	else oldpageone = EOF;
	if (strncmp(newline, "%%Page:", 7) == 0) {
		newpageone = 0; newpagetwo = 0;
		if (sscanf (newline+7, "%d %d", &newpageone, &newpagetwo) < 1) {
			printf("Sorry, don't understand: %s", newline);
		}
	}	
	else newpageone = EOF;
	fseek (oldfile, oldpresent, SEEK_SET);
	fseek (newfile, newpresent, SEEK_SET);
}
#endif

void getoldpageno(FILE *oldfile) {
	long oldpresent;
	char *olds;

	oldpresent = ftell(oldfile);
	olds = fgetline(oldline, MAXLINE, oldfile);

	if (strncmp(oldline, "%%Page:", 7) == 0) {
		oldpageone = 0; oldpagetwo = 0;
		if (sscanf (oldline+7, "%d %d", &oldpageone, &oldpagetwo) < 1) {
			printf("Sorry, don't understand: %s", oldline);
		}
	}
	else oldpageone = EOF;
	fseek (oldfile, oldpresent, SEEK_SET);
}

void getnewpageno(FILE *newfile) {
	long newpresent;
	char *news;

	newpresent = ftell(newfile);
	news = fgetline(newline, MAXLINE, newfile);

	if (strncmp(newline, "%%Page:", 7) == 0) {
		newpageone = 0; newpagetwo = 0;
		if (sscanf (newline+7, "%d %d", &newpageone, &newpagetwo) < 1) {
			printf("Sorry, don't understand: %s", newline);
		}
	}
	else newpageone = EOF;
	fseek (newfile, newpresent, SEEK_SET);
}

void getpagenos (FILE *oldfile, FILE *newfile) {
	getoldpageno (oldfile);
	getnewpageno (newfile);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

unsigned long checkcopyright(char *s) {
	int c;
	unsigned long hash=0;
	while ((c = *s++) != '\0') {
		hash = (hash * 53 + c) & 16777215;
	}
	if (hash == COPYHASH) return 0; /*  change if copyright changed */
	fprintf(stderr, "HASHED %ld\n", hash);	
/*	(void) getch();  */
/*	(void) _getch();  */
	return hash;
}

int decodeflag (int c) {
	switch(c) {
		case '?': detailflag = 1; return 0;  
		case 'v': verboseflag = 1; return 0; 
		case 't': traceflag = 1; return 0; 
		case 'd': dotsflag = 0; return 0; 
		case 's': skipcheck = 0; return 0; 
		case 'f': tryfgets = 0; return 0; 
		case 'r': followorder = 0; return 0;
		case 'a': adjustsequence = 0; return 0; 
/* rest take arguments */
		case 'o': outputflag = 1; return -1;
/* reversed 1994/July/6 */
/* -p is `logical' (first page number) */
/* -P is `physical' (second page number) */
		case 'p': pagesflag = 1; usepagenone = 1; return -1;
		case 'P': pagesflag = 1; usepagenone = 0; return -1;
		default: {
				fprintf(stderr, "Invalid command line flag '%c'", c);
				exit(7);
		}
	}
}

void showusage (char *s) {
	printf("Usage: %s [-v] <old-ps-file> <new-ps-file>\n", s);
	printf("Usage: %s [-v] [-f] [-P=<list-of-page-ranges>] <ps-file>\n", s);
	printf("       v verbose mode\n");
	printf("       page ranges separated by comma (or +) in list e.g.\n");
	printf("            -P=23,7-4,63,39-42\n");
	printf("       P page numbers are physical (sequential)\n");
	printf("       p page numbers are logical\n");
	printf("       f output the pages in same order as found in input file\n");

/* pages are included in specified order */
	exit(1);
}

/* deal with command line flags */
int commandline(int argc, char *argv[], int firstarg) {
	int c;
	unsigned int i;
	char *s;

	if (argc < 2) showusage(argv[0]);

	while (firstarg < argc && argv[firstarg][0] == '-') { /* check for command flags */
		for(i=1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break;
			else if (decodeflag(c) != 0) { /* flag takes argument */
				if ((s = strchr(argv[firstarg], '=')) == NULL) {
					firstarg++; s = argv[firstarg];
				}
				else s++;
				if (outputflag != 0) {
					outdefault = s;
					outputflag = 0; 
				} 
				if (pagesflag != 0) {
					pagelist = s;
					pagesflag = 0; 
				} 
				break;
			}
		}
		firstarg++;
	}
	return firstarg;
}

void complainres (void) {
	printf("ERROR: the input PS files were not page independent\n");
	printf("       so the output PS file may not work correctly\n");
}

void processpages (FILE *oldfile, FILE *newfile) {
	int flag;
	int oldpage, newpage;
	int oldpageonesaved, newpageonesaved, oldpagetwosaved, newpagetwosaved;
	FILE *outfile=NULL;
	char outfilename[MAXFILENAME];
	long oldstart, newstart;
	int pageno;	

	if (traceflag) printf("Entering processpages\n");

	strcpy(outfilename, outdefault); 
	extension(outfilename, "ps");

	flag = 0;					/* keep compiler happy */

/*	if (skipcheck) goto restart;  */
	if (skipcheck == 0) {

/*		skip forward to first page in new file */
		if ((newpage = skippage(newfile, 1)) == EOF) {
			fprintf(stderr, "Hit EOF: new file contains no pages\n");
			fprintf(stderr, "Perhaps not a PS file, or not DSC complaint?\n");
			exit(1);
		}
/*		skip forward to first page in old file */
		if ((oldpage = skippage(oldfile, 1)) == EOF) {
			fprintf(stderr, "Hit EOF: old file contains no pages\n");
			fprintf(stderr, "Perhaps not a PS file, or not DSC complaint?\n");
			exit(1);
		}

/*		if (skipcheck) {
			fseek (newfile, 0, SEEK_SET);
			fseek (oldfile, 0, SEEK_SET);
			goto restart;
		} */

		flag = 0;		/* to keep compiler happy ... */

/*		now see whether all new pages match some old pages */
		for (;;) {
			getpagenos (oldfile, newfile);
			if (newpageone == EOF) {
				flag = EOF; break;	/* OK, new file ended before difference noted */
			}
			if (oldpageone == EOF) {
				flag = 1; break;	/* Not OK, old file ended before new file */
			}
/*		are there new pages that do not match old pages ? */
			if (newpageone < oldpageone) {
				flag = 1; break;	/* there are new pages inserted */
			}
/*		skip over old pages that do not match new pages */
			while (newpageone > oldpageone) {	/* ignore some old pages */
				skippage(oldfile, 0);
				getpagenos (oldfile, newfile);
				if (oldpageone == EOF) {
					flag = 1; break;	/* old file ended before new file */
				}
			}
/*		finally: go and actually compare a page */
			if ((flag = comparepage (oldfile, newfile)) != 0) break;
		}
/*		if all pages match, we hit end of file - then nothing to do */
		if (flag == EOF) {
			printf ("New pages match old pages --- no output file\n");
/*			goto closein;  */
		}
		else {					/* flag != EOF case */
/*	some mismatch, start over again */
			fseek (oldfile, 0, SEEK_SET);
			fseek (newfile, 0, SEEK_SET);
			if (verboseflag) printf("\nSome differences found, rewinding\n\n");
		}
	}

/*	restart:	*/		/* come here directly if -s used on command line */

	npages = 0;

	if (skipcheck || flag != EOF) {

/*		OK, there are differences, need to open an output file */
		if ((outfile = fopen(outfilename, "wb")) == NULL) {
			perror(outfilename); exit(1);
		}

/*		extractdifference (outfile, newfile, oldfile); */

/* void extractdifference (FILE *outfile, FILE *newfile, FILE *oldfile) { */

/*	copy header from new file */
		if ((newpage = copypage(outfile, newfile, 1)) == EOF) {
			fprintf(stderr, "Hit EOF: new file contains no pages\n");
			fprintf(stderr, "Perhaps not a PS file, or not DSC complaint?\n");
			exit(1);
		}
/*	skip to first page in old file */
		if ((oldpage = skippage(oldfile, 1)) == EOF) {
			fprintf(stderr, "Hit EOF: old file contains no pages\n");
			fprintf(stderr, "Perhaps not a PS file, or not DSC complaint?\n");
			exit(1);
		}

		for (;;) {
/*	 get the page numbers, just in case */
			getpagenos (oldfile, newfile);
			if (newpageone == EOF) {
				flag = EOF; break;	/* OK, new file ended */
			}
			if (oldpageone == EOF) {
				flag = 0; break;	/* copy rest of new file */
			}
/*	 are there new pages that do not match old pages ? */
			while (newpageone < oldpageone) {
				copypage(outfile, newfile, 0);
				if (ferror (outfile) != 0) {
					fprintf(stderr, "Error in output file ");
					perror(outfilename); 
					break;
				}
				getpagenos (oldfile, newfile);
				if (newpageone == EOF) {
					flag = EOF; break;		/* OK, new file ended */
				}
			}
/*	skip over old pages that do not match new pages */
			while (newpageone > oldpageone) {	/* ignore some old pages */
				skippage(oldfile, 0);
				getpagenos (oldfile, newfile);
				if (oldpageone == EOF) {
					flag = 0; break;	/* copy rest of new file */
				}
			}
/*		remember where we where */
			oldstart = ftell(oldfile);
			newstart = ftell(newfile);		
			oldpageonesaved = oldpageone;
			newpageonesaved = newpageone;
			oldpagetwosaved = oldpagetwo;
			newpagetwosaved = newpagetwo;
/*		finally: go and actually compare a page */
			if ((flag = comparepage (oldfile, newfile)) != 0) {
				if (flag == EOF) break;
				fseek (newfile, newstart, SEEK_SET);	/* rewind new file */
				if (verboseflag) {
					if (adjustsequence)
						printf("[%d %d] ", newpageonesaved, npages+1);
					else printf("[%d %d] ", newpageonesaved, newpagetwosaved);  
				}
				copypage(outfile, newfile, 0);
				if (ferror (outfile) != 0) {
					fprintf(stderr, "Error in output file ");
					perror(outfilename); 
					break;
				}
				newpageone = pageone;
				newpagetwo = pagetwo;
/*	following is there mostly so we can pick up on %%BeginResource */
				fseek (oldfile, oldstart, SEEK_SET);	/* rewind new file */
				skippage(oldfile, 0);			/* skip to end of the old page */
				oldpageone = pageone;
				oldpagetwo = pagetwo;
			}
			else {		/* if pages compared OK */
/*				if (dotsflag != 0) putc ('.', stdout); */
				if (usepagenone) pageno = newpageone;
				else pageno = newpagetwo;
				if (verboseflag != 0) printf("[%d] ", pageno);
				else if (dotsflag != 0) printf (". ");
			}
			if (strncmp(newline, "%%Trailer", 9) == 0) {
/*				if (traceflag) printf("Copying trailer\n"); */
/*				copypage(outfile, newfile, 0); */
				break;
			}
		}
		putc('\n', stdout);

		for (;;) {
			if (strncmp(newline, "%%Trailer", 9) == 0) {
				if (traceflag) printf("Copying trailer\n"); 
				flag = 1;
			}
			else flag = 0;
			if (copypage(outfile, newfile, flag) == EOF) break;
		} 

/* closeout: */

		if (npages == 0) {			/* if there were no differences */
			fclose (outfile);
			remove (outfilename);
			printf ("New pages match old pages --- no output file\n");
/*			goto closein; */
		}
		else {
			if (ferror (outfile) != 0) {
				fprintf(stderr, "Error in output file ");
				perror(outfilename); 
			}
			else fclose (outfile);
			if (verboseflag) 
				printf("Output file `%s' in current directory has %d pages\n",
					outfilename, npages);
			if (resource > 0) complainres();
		}
	}

}

void extractpages (FILE *oldfile) {
	int flag;
	int oldpage;
	int pageno;
/*	int oldpageonesaved, oldpagetwosaved; */
	FILE *outfile=NULL;
	char outfilename[MAXFILENAME];
	long oldstart;	

	if (traceflag) printf("Entering extractpages\n");

	strcpy(outfilename, outdefault); 
	extension(outfilename, "ps");

	flag = 0;		/* to keep compiler happy ... */

	npages = 0;

/*	if (skipcheck || flag != EOF) { */

	if ((outfile = fopen(outfilename, "wb")) == NULL) {
		perror(outfilename); exit(1);
	}

/*	copy header from old file */
	if ((oldpage = copypage(outfile, oldfile, 1)) == EOF) {
		fprintf(stderr, "Hit EOF: old file contains no pages\n");
		fprintf(stderr, "Perhaps not a PS file, or not DSC complaint?\n");
		exit(1);
	}

	for (;;) {
/*	 get the page numbers */
		getoldpageno (oldfile);
		if (oldpageone == EOF) {
			flag = EOF; break;		/* OK, old file ended */
		}
		oldstart = ftell(oldfile);
		if (usepagenone) pageno = oldpageone;
		else pageno = oldpagetwo;
		if (desired (pageno)) {
			copypage(outfile, oldfile, 0);
			if (ferror (outfile) != 0) {
				fprintf(stderr, "Error in output file ");
				perror(outfilename); 
				break;
			}
/*			if (dotsflag != 0) printf (". "); */
			if (verboseflag != 0) printf("[%d] ", pageno);
			else if (dotsflag != 0) printf (". ");

/*			npages++;	*/		/* redundant ? */
		}
		else {
			skippage(oldfile, 1);
		}
/*		putc('\n', stdout); */ /* ??? */

			if (strncmp(newline, "%%Trailer", 9) == 0) {
/*				if (traceflag) printf("Copying trailer\n"); */
/*				copypage(outfile, newfile, 0); */
				break;
			}
	}

	for (;;) {
		if (strncmp(oldline, "%%Trailer", 9) == 0) {
			if (traceflag) printf("Copying trailer\n"); 
			flag = 1;
		}
		else flag = 0;
		if (copypage(outfile, oldfile, flag) == EOF) break;
	} 

	if (npages == 0) {			/* if there were no pages copied */
		fclose (outfile);
		remove (outfilename);
		printf ("No pages match specified page ranges --- no output file\n");
/*			goto closein; */
	}
	else {
		if (ferror (outfile) != 0) {
			fprintf(stderr, "Error in output file ");
			perror(outfilename); 
		}
		else fclose (outfile);
		if (verboseflag) 
			printf("Output file `%s' in current directory has %d pages\n",
				outfilename, npages);
		if (resource > 0) complainres();
	}
}

void prescan (FILE *oldfile) {
	int flag;
	int oldpage;
	int k;
/*	int oldpageonesaved, oldpagetwosaved; */
/*	FILE *outfile=NULL; */
/*	char outfilename[MAXFILENAME]; */
/*	long oldstart;	 */

	if (traceflag) printf("Entering prescan\n");

	flag = 0;		/* to keep compiler happy ... */

	numpage = 0;

/*	copy header from old file */
	if ((oldpage = skippage(oldfile, 1)) == EOF) {
		fprintf(stderr, "Hit EOF: old file contains no pages\n");
		fprintf(stderr, "Perhaps not a PS file, or not DSC complaint?\n");
		exit(1);
	}

	for (;;) {
/*		get the page numbers */
		getoldpageno (oldfile);
		if (oldpageone == EOF) {
			flag = EOF; break;		/* OK, old file ended */
		}
		
		if (numpage > MAXPAGES) {
			fprintf(stderr, "ERROR: too many pages\n");
			exit(3);
		}

		pagepos[numpage] = ftell(oldfile);
		pageones[numpage] = oldpageone;
		pagetwos[numpage] = oldpagetwo;
		numpage++;
		skippage(oldfile, 1);

		if (strncmp(newline, "%%Trailer", 9) == 0) {
/*			if (traceflag) printf("Copying trailer\n"); */
/*			copypage(outfile, newfile, 0); */
			break;
		}
	}
	pagepos[numpage] = ftell (oldfile);	/* trailer position */
	if (traceflag) {
		printf("%ld\tHeader\n", 0L);
		for (k = 0; k < numpage; k++)
			printf("%ld\t%d\t%d\n", pagepos[k], pageones[k], pagetwos[k]);
		printf("%ld\tTrailer\n", pagepos[numpage]);
	}
}

/* returns zero if fails for some reason */

int copypagen (FILE *outfile, FILE *oldfile, int n) {
	int flag = 0;
	int k;
	
	if (traceflag) printf("Trying to copy page %d\n", n);

	for (k = 0; k < numpage; k++) {
		if (usepagenone) {
			if (pageones[k] == n) {
				flag = 1;
				break;
			}
		}
		else {
			if (pagetwos[k] == n) {
				flag = 1;
				break;
			}
		}
	}
	if (flag == 0) {
		fprintf(stderr, "ERROR: page %d not found\n", n);
		return 0;
	}
	fseek (oldfile, pagepos[k], SEEK_SET);
	copypage(outfile, oldfile, 0);
	if (ferror (outfile) != 0) {
		fprintf(stderr, "Error in output file ");
		perror("");
		return 0;
	}
/*	if (dotsflag != 0) printf (". "); */
	if (verboseflag != 0) printf("[%d] ", n);
	else if (dotsflag != 0) printf (". ");
	return 1;
}

void pulloutpages (FILE *oldfile) {
	int flag;
	int oldpage;
/*	int pageno; */
	int k, m;
	FILE *outfile=NULL;
	char outfilename[MAXFILENAME];
/*	long oldstart;	 */

	if (traceflag) printf("Entering pulloutpages\n");

	strcpy(outfilename, outdefault); 
	extension(outfilename, "ps");

	flag = 0;		/* to keep compiler happy ... */

	npages = 0; 

/*	if (skipcheck || flag != EOF) { */

	if ((outfile = fopen(outfilename, "wb")) == NULL) {
		perror(outfilename); exit(1);
	}

/*	copy header from old file */
	if ((oldpage = copypage(outfile, oldfile, 1)) == EOF) {
		fprintf(stderr, "Hit EOF: old file contains no pages\n");
		fprintf(stderr, "Perhaps not a PS file, or not DSC complaint?\n");
		exit(1);
	}

	for (k = 0; k < pageindex; k++) {
		if (endpage[k] >= beginpage[k]) {
			for (m = beginpage[k]; m <= endpage[k]; m++) 
				if (copypagen (outfile, oldfile, m) == 0) break;
		}
		else {
			for (m = beginpage[k]; m >= endpage[k]; m--) 
				if (copypagen (outfile, oldfile, m) == 0) break;
		}
	} 

	fseek (oldfile, pagepos[numpage], SEEK_SET);	/* go to trailer */
/*	if (copypage(outfile, oldfile, 0) == EOF) break; */
	if (copypage(outfile, oldfile, 1) != EOF)
		fprintf(stderr, "Trailer does not end with EOF?\n");	

	if (npages == 0) {			/* if there were no pages copied */
		fclose (outfile);
		remove (outfilename);
		printf ("No pages match specified page ranges --- no output file\n");
	}
	else {
		if (ferror (outfile) != 0) {
			fprintf(stderr, "Error in output file ");
			perror(outfilename); 
		}
		else fclose (outfile);
		if (verboseflag) {
			putc('\n', stdout);
			printf("Output file `%s' in current directory has %d pages\n",
				outfilename, npages);
		}
		if (resource > 0) complainres();
	}
}

int main (int argc, char *argv[]) {
	char oldfilename[MAXFILENAME];
	char newfilename[MAXFILENAME];
	char outfilename[MAXFILENAME]; 
/*	char bakfilename[MAXFILENAME]; */
/*	FILE  *outfile=NULL; */
	FILE *newfile=NULL, *oldfile=NULL;
/*	long oldstart, newstart;  */
	int firstarg=1;
/*	int flag; */
/*	int oldpage, newpage; */
/*	int oldpageonesaved, newpageonesaved, oldpagetwosaved, newpagetwosaved;*/

	firstarg = commandline(argc, argv, 1);

/*	if (firstarg > argc - 2) showusage(argv[0]); */
	if (firstarg > argc - 1) showusage(argv[0]);

	if (checkcopyright(copyright) != 0) {
/*		fprintf(stderr, "HASH %lu", checkcopyright(copyright)); */
		exit(1);	/* check for tampering */
	}

	if (traceflag) printf("The page list argument is: %s\n", pagelist);
/*	unless page range is specified, need two arguments */
	if (strcmp(pagelist, "") != 0) analpages(pagelist);	/* 1994/July/1 */
	else if (firstarg > argc - 2) showusage(argv[0]);

	printf("%s\n", programversion);
	if (wantcpyrght) puts(copyright); 

/*	set up the file names and open the files */

	strcpy(oldfilename, argv[firstarg]);
	extension(oldfilename, "ps");
	if (selectpages == 0) {
		strcpy(newfilename, argv[firstarg+1]);
		extension(newfilename, "ps");
	}
	else strcpy(newfilename, "");
/*	if (argc < firstarg+3) strcpy(outfilename, outdefault); */
/*	else strcpy(outfilename, argv[firstarg+2]); */
	strcpy(outfilename, outdefault); 
	extension(outfilename, "ps");

	if (strcmp(outfilename, oldfilename) == 0 ||
		strcmp(outfilename, newfilename) == 0) {
		fprintf(stderr,
		"ERROR: Output file name (%s) same as one of the input file names\n",
				outfilename);
		exit(1);
	}

	resource = 0;

	if ((oldfile = fopen(oldfilename, "rb")) == NULL) {
		perror(oldfilename); exit(1);
	}
	if (selectpages == 0) {
		if ((newfile = fopen(newfilename, "rb")) == NULL) {
			perror(newfilename); exit(1);
		}
		processpages(oldfile, newfile); 	/* work in progress */
		fclose (oldfile);
		fclose (newfile);
	}
	else {								/* page extraction case */
		if (pageindex > 0) {
			if (followorder) {
				prescan (oldfile);		/* find and remember page starts */
				rewind (oldfile);
				pulloutpages(oldfile);
			}
			else extractpages(oldfile);	/* work in progress */
		}
		fclose (oldfile);
	}

/*	 this is where everything used to live ... */	

/* closein: */
/*	fclose (oldfile); */
/*	fclose (newfile); */

	return 0;
}

/* end file with %%Trailer and %%EOF */

/* we are now missing this */

/* BeginResource: font, BeginFont */

/* BeginDocument, EndDocument */

/*		fprintf(output, "%%%%BeginFont: %s\n", filefontname);  */
/*		fprintf(output, "%%%%BeginResource: font %s\n", filefontname); 	 */

/*		if (strncmp(line, "%%BeginDocument", 15) == 0) nesting++; */
/*		else if (strncmp(line, "%%EndDocument", 13) == 0) nesting--; */

/* what if output file name equals one of the inputs ? */

/* allow command line specification of output file name ? -o= */
