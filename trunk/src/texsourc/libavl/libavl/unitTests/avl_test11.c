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

static void data_copy(void *src, void *dst)
{
    *((int *) dst) = *((int *) src);
}

char *same_element_values_tests()
{
    tree *first = NULL;
    int data;
    int get_data_result = 0;
    unsigned int result;

    unsigned long rand_seed = (unsigned long) time(NULL);
    ILOG("Random seed: %lu", rand_seed);
    srand(rand_seed);
    // Try to allocate a new tree.
    first = init_dictionnary(data_cmp, data_print, data_delete, data_copy);
    if (first == NULL) {
        ELOG("Init dictionnary error");
        return "Init dictionnary error";
    }
    if (sizeof(*first) != sizeof(tree)) {
        ELOG("Wrong returned size");
        return "Wrong returned size";
    }

    data = rand();

    // Insert one element
    result = insert_elmt(first, &data, sizeof(int));
    if (result != 1) {
        ELOG("Wrong result of insert element");
        return "Wrong result of insert element";
    }

    // Check if element is in tree
    if (!is_present(first, &data)) {
        ELOG("Data not found in tree");
        return "Data not found in tree";
    }

    // Check value of element
    get_data_result = data;
    if (!get_data(first, &get_data_result, sizeof(int))) {
        ELOG("Wrong get data result");
        return "Wrong get data result";
    }

    if (data != get_data_result) {
        ELOG("Wrong data stored");
        return "Wrong data stored";
    }

    // Insert the same element
    result = insert_elmt(first, &data, sizeof(int));
    if (result != 1) {
        ELOG("Wrong result of insert element");
        return "Wrong result of insert element";
    }

    // Check if element is in tree
    if (!is_present(first, &data)) {
        ELOG("Data not found in tree");
        return "Data not found in tree";
    }

    // Check value of element
    get_data_result = data;
    if (!get_data(first, &get_data_result, sizeof(int))) {
        ELOG("Wrong get data result");
        return "Wrong get data result";
    }

    if (data != get_data_result) {
        ELOG("Wrong data stored");
        return "Wrong data stored";
    }

    // Try to delete it
    delete_tree(first);



    return NULL;
}
