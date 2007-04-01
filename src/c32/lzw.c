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

#define MAXCODES 4096

#define MAXCHR 256

#define CLEAR 256

#define EOD 257

#define FIRSTCODE 258

#define DEBUGNEWLZW 1

struct NODE {
//	int code;			// code to emit when here, and next is new
	int chr;			// byte to get to this node --- or -1
//	NODE nextinlist;	// next in list at this level
	int nextinlist;		// next in list at this level
//	NODE nextlevel;		// first at next level
	int nextlevel;		// first at next level
#ifdef DEBUGNEWLZW
	char *str;			// debugging --- string for this code
#endif
};

struct NODE *node=NULL;		// array of nodes --- allocated

int currentnode;		// code of node currently at
int nextnode;			// code of next node to be used
int codelength;

#ifdef DEBUGNEWLZW
char str[MAXCODES+1];	// scratch string area DEBUGNEWLZW
#endif

int traceflag=1;

#ifdef DEBUGNEWLZW
void ShowCodeTable (void) {
	int k;
	printf("CODE TABLE:\n");
	fflush(stdout);
//	for (k = 0; k < nextnode; k++) 
	for (k = FIRSTCODE; k < nextnode; k++) {
		printf("%d\t%d\t%d\t%d\t(%s)\n",
			   k, node[k].chr, node[k].nextinlist, node[k].nextlevel, node[k].str);
	}
}
#endif

void LZWput (int code) {
#ifdef DEBUGNEWLZW
	printf("emit\t%d\t(%s)\n", code, node[code].str);	// debugging
#else
	printf("emit\t%d\n", code);		// debugging
#endif
	fflush(stdout);
}

void CleanOut (int n) {
#ifdef DEBUGNEWLZW
	printf("CLEANOUT %d\n", n);
	fflush(stdout);
#endif
	LZWput(CLEAR);
	nextnode = FIRSTCODE;
	currentnode = n;
	codelength = 9;
}

int SetupNodes (void) {
	int k, nlen;
	if (node == NULL) {
		nlen = MAXCODES * sizeof(struct NODE);
		node = (struct NODE *) malloc(nlen);
		if (node == NULL) {
			printf("Unable to allocate %d bytes for NODE table\n", nlen);
			return -1;
		}
#ifdef DEBUGNEWLZW
		if (traceflag) printf("Allocated %d bytes for NODE table\n", nlen);
		fflush(stdout);
#endif
	}

//	0 to MAXCHR, and CLEAR and EOD
	for (k = 0; k < FIRSTCODE; k++) {
//		node[k].code = k;
		node[k].chr = -1;
		node[k].nextinlist = -1;
		node[k].nextlevel = -1;
#ifdef DEBUGNEWLZW
		{
			char *s=str;
			*s = (char) k;
			*(s+1) = '\0';
			node[k].str = _strdup(str);
//			printf("STRDUP %s\n", str);
//			fflush(stdout);
		}
#endif
	}
#ifdef DEBUGNEWLZW
	node[CLEAR].str = "CLEAR";	// DEBUGNEWLZW
	node[EOD].str = "EOD";		// DEBUGNEWLZW
#endif
//	LZWput(CLEAR);			// ?
//	nextnode = FIRSTCODE;
//	currentnode = -1;
//	codelength = 9;
	cleanout(-1);
	return 0;
}

// we are at node n and adding a new branch for byte chr -- NOT USED

// int addanode (NODE n, int chr) {
void AddaNode (int n, int chr, int previous) {
	int k;
//	is there already a node at the next level ?
	if (node[n].nextlevel < 0) {		// no, start next level list
		node[n].nextlevel = nextnode;	// link to new node
	}
	else {								// yes, link to end of that list
		k = node[n].nextlevel;			// ???
//		find end of linked list
		while ((k = node[k].nextinlist) >= 0) ;
		node[k].nextinlist = nextnode;	// link new node to end of list
	}
//	set up new node being added
//	node[nextnode].code = nextnode;
	node[nextnode].chr = chr;
	node[nextnode].nextinlist = -1;
	node[nextnode].nextlevel = -1;
#ifdef DEBUGNEWLZW
	{
		char *s;
		strcpy(str, node[previous].str);
		s = str + strlen(str) - 1;
		*s = (char) chr;
		*(s+1) = '\0';
		node[nextnode].str = _strdup(str);
//		printf("STRDUP %s\n", str);
//		fflush(stdout);
	}
#endif
	nextnode++;
	if (nextnode == 512) codelength++;
	else if (nextnode == 1024) codelength++;
	else if (nextnode == 2048) codelength++;
	else if (nextnode == 4095) cleanout(chr);
}

// int newnode (NODE n, int chr) {
// int newnode (int chr) {
void NewNode (int chr, int previous) {
//	set up new node being added
//	node[nextnode].code = nextnode;
//	printf("NEW NODE %d %d\n", chr, previous);
//	fflush(stdout);
	node[nextnode].chr = chr;
	node[nextnode].nextinlist = -1;
	node[nextnode].nextlevel = -1;
#ifdef DEBUGNEWLZW
	{
		char *s;
		strcpy(str, node[previous].str);
		s = str + strlen(str);
		*s = (char) chr;
		*(s+1) = '\0';
		node[nextnode].str = _strdup(str);
//		printf("STRDUP %s\n", str);
//		fflush(stdout);
	}
#endif
#ifdef DEBUGNEWLZW
	if (traceflag)  printf("new node %d chr %d previous %d (%s)\n",
						   nextnode, chr, previous, node[nextnode].str);
	fflush(stdout);
#endif
	nextnode++;
	if (nextnode == 512) codelength++;
	else if (nextnode == 1024) codelength++;
	else if (nextnode == 2048) codelength++;
	else if (nextnode == 4095) cleanout(chr);
}

// Process next incomiong byte

void DoNextByte (int chr) {
	int k;
#ifdef DEBUGNEWLZW
	if (traceflag) printf("next\t%d\t%c\tcurrent %d (%s)\n",
						  chr, chr, currentnode, (currentnode < 0) ? "" : node[currentnode].str);
	fflush(stdout);
#endif
	if (currentnode < 0) {	// starting from scratch ?
		currentnode = chr;
		return;
	}
	if (node[currentnode].nextlevel < 0) {	// new string NOT in table
//		AddaNode(currentnode, n);
		LZWput(currentnode);
		node[currentnode].nextlevel = nextnode;
		NewNode(chr, currentnode);	// ???
//		currentnode = nextnode-1;
		currentnode = chr;
#ifdef DEBUGNEWLZW
		if (traceflag) printf("current\t%d\t%c\n", chr, chr);
		fflush(stdout);

#endif
		return;
	}
	k = node[currentnode].nextlevel;
	while (k >= 0 && node[k].chr != chr) {
		k = node[k].nextinlist;
	}
	if (k < 0) {	// new string NOT in table
//		AddaNode(currentnode, n);
		LZWput(currentnode);
		node[k].nextinlist = nextnode;
		NewNode(chr, currentnode);	// ???
//		currentnode = nextnode-1;	// ???
		currentnode = chr;
#ifdef DEBUGNEWLZW
		if (traceflag) printf("current\t%d\t%c\n", chr, chr);
		fflush(stdout);

#endif
		return;
	}
#ifdef DEBUGNEWLZW
	if (traceflag) printf("%d (%s) is in table\n", k, node[k].str);
	fflush(stdout);

#endif
	currentnode = k;	// it IS in table, no output
}

void DoCleanup (void) {
	if (currentnode >= 0) LZWput(currentnode);
#ifdef DEBUGNEWLZW
	if (traceflag) showcodetable();
#endif
//	LZWput(CLEAR);	
//	nextnode = FIRSTCODE;
//	currentnode = -1;
//	codelength = 9;
	CleanOut(-1);
}

void compressfile (FILE *input) {
	int chr;
	SetupNodes();
	while ((chr = getc(input)) != EOF)
		DoNextByte(chr);
	DoCleanup();
}

int main (int argc, char *argv[]) {
	FILE *input;
	char *filename="test.txt";

//	SetupNodes();
//	if (traceflag) showcodetable();
//	return 0;
	
	if (argc > 1) filename = argv[1];

	input = fopen(filename, "rb");
	if (input == NULL) {
		perror(filename);
		exit(1);
	}
	if (traceflag) printf("FILE %s open\n", filename);
	fflush(stdout);
	compressfile(input);
	fclose(input);
	if (traceflag) printf("FILE %s closed\n", filename);
	fflush(stdout);
}

/* After adding table entry 511, switch to 10-bit codes (i.e. entry 512
 * should be a 10-bit code.) Likewise, switch to 11-bit codes after
 * table entry 1023,a nd 12-bit codes after table entry 2047 */

/* Whenever you add a code to the output stream, it "counts" towards
 * the decision about bumping the code bit length.  This is important
 * when writing the last code word before an EOD code or ClearCode, to
 * avoid code length errors. */

/* As soon as we use entry 4094, we write out a (12-bit) ClearCode.
 * (If we wait any longer to write the ClearCode, the decompressor
 * might try and interpret the ClearCode as a 13-bit code). */

/* In decompression description: Must siwtch to 10-bit codes as soon
 * as string 510 is stored into the table. Similarly, the switch is
 * made to 11-bit codes after #1022 and to 12-bit codes after #2046 */
