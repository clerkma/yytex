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

/* LIST.C - 16 bit / 32 bit */ /* usage: list [nodesize] */

/* cl /AL list.c */
/* list */

/* cl386 /c list.c */
/* 386link list @msc386 */
/* list */

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <time.h>

typedef struct node {
	unsigned long num;
	void *data;
	struct node *next;
} NODE;

int main(int argc, char *argv[]) {
	NODE *p, *q;
	time_t t1, t2, t3;
	unsigned long nodes, freed;
	unsigned nodesize = (argc > 1) ? atoi(argv[1]) : 512;

	time (&t1);

	for (q = NULL, nodes = 0 ; ; q->next = p) {
		p = q;
		if ((q = malloc (sizeof(NODE))) == NULL) break;
		if ((q->data = malloc(nodesize)) == NULL) {
			free(q); break;
		}
		q->num = nodes++;
	}

	time (&t2);

	for (freed = nodes; p != NULL; p = q) {
		q = p->next;
		if (p->num != --freed) {
			printf("List corrupted: freed=%lu num=%lu\n",
				freed, p->num);
			return 1;
		}
		free(p->data);
		free(p);
	}

	if (freed != 0) {
		printf("List corrupted: allocated %lu blocks, freed %lu blocks\n",
			nodes, nodes - freed);
		return 1;
	}

	time (&t3);

	printf("\nAllocated %lu nodes, %uK\n",
		nodes, (nodes * (sizeof(NODE)+nodesize)) >> 10);
	printf("Total time: %lu seconds\n", t3 - t1); 
/*	printf("Total time: %lg seconds\n", difftime(t3, t1)); */
	printf("Allocate time: %lu, freeing time: %lu\n", 
		t2 - t1, t3 - t2);
/*	printf("Allocate time: %lg, freeing time: %lg\n",
		difftime(t2, t1), difftime(t3, t2)); */

	return 0;
}
