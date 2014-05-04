/*
 *   Libavl is a library to manage AVL structure to store and organize
 *   everykind of data. You just need to implement function to compare,
 *   to desallocate and to print your structure.
 *
 *       DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE 
 *                   Version 2, December 2004 
 *
 *   Copyright (C) 2013 Adrien Oliva <adrien.oliva@yapbreak.fr>
 *
 *   Everyone is permitted to copy and distribute verbatim or modified 
 *   copies of this license document, and changing it is allowed as long 
 *   as the name is changed. 
 *
 *           DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE 
 *   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION 
 *
 *   0. You just DO WHAT THE FUCK YOU WANT TO.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../syslog.h"
#include "../avl.h"

static int data_cmp(void *a, void *b)
{
    int aa = *((int *) a);
    int bb = *((int *) b);

    return aa - bb;
}

static void data_print(void *d)
{
    printf("%p|%d", d, *((int *) d));
}

static void data_delete(void *d)
{
    free(d);
}

#define MAX_ELEMENT 10000

char *add_existing_tests()
{
    tree *first = NULL;
    int data[MAX_ELEMENT];
    unsigned int result;
    unsigned int element_in_tree = 0;
    int i = 0;

    unsigned long rand_seed = (unsigned long) time(NULL);
    ILOG("Random seed: %lu", rand_seed);
    srand(rand_seed);

    for (i = 0; i < MAX_ELEMENT; i++) {
        data[i] = rand();
    }


    // Try to allocate a new tree.
    first = init_dictionnary(data_cmp, data_print, data_delete, NULL);
    if (first == NULL) {
        ELOG("Init dictionnary error");
        return "Init dictionnary error";
    }

    verif_tree(first);
    for (i = 0; i < MAX_ELEMENT; i++) {
        if (!is_present(first, &(data[i]))) {
            element_in_tree++;
        }
        result = insert_elmt(first, &(data[i]), sizeof(int));
        if (result != element_in_tree) {
            ELOG("Wrong result of inserted element");
            return "Wrong result of inserted element";
        }
        verif_tree(first);
    }

    // Try to add existing data
    for (i = 0; i < MAX_ELEMENT; i++) {
        if (!is_present(first, &(data[i]))) {
            ELOG("Element is not present, it said! F**k");
            return "Element is not present";
        }
        result = insert_elmt(first, &(data[i]), sizeof(int));
        if (result != element_in_tree) {
            ELOG("Wrong result of inserted element");
            return "Wrong result of inserted element";
        }
        verif_tree(first);
    }


    // Try to delete it
    delete_tree(first);



    return NULL;
}
