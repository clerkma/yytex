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

struct _tree_data {
    int key;
    int value;
};

static int data_cmp(void *a, void *b)
{
    struct _tree_data aa = *((struct _tree_data *) a);
    struct _tree_data bb = *((struct _tree_data *) b);

    return aa.key - bb.key;
}

static void data_print(void *d)
{
    printf("%p|%d-%d", d,
            ((struct _tree_data *) d)->key, ((struct _tree_data *) d)->value);
}

static void data_delete(void *d)
{
    free(d);
}

#define MAX_ELEMENT 10000

char *get_data_tests()
{
    tree *first = NULL;
    struct _tree_data data[MAX_ELEMENT];
    struct _tree_data tmp_elmnt;
    struct _tree_data current_min;
    unsigned int result;
    unsigned int element_in_tree = 0;
    int i = 0;
    int j = 0;

    unsigned long rand_seed = (unsigned long) time(NULL);
    ILOG("Random seed: %lu", rand_seed);
    srand(rand_seed);

    for (i = 0; i < MAX_ELEMENT; i++) {
        data[i].key = rand();
        data[i].value = rand();
    }


    verif_tree(first);

    // Try to allocate a new tree.
    first = init_dictionnary(data_cmp, data_print, data_delete, NULL);
    if (first == NULL) {
        ELOG("Init dictionnary error");
        return "Init dictionnary error";
    }

    // Add data
    verif_tree(first);
    for (i = 0; i < MAX_ELEMENT; i++) {
        tmp_elmnt.key = data[i].key;
        if (!is_present(first, &(tmp_elmnt))) {
            element_in_tree++;
        }
        result = insert_elmt(first, &(data[i]), sizeof(struct _tree_data));
        if (result != element_in_tree) {
            ELOG("Wrong result of inserted element");
            return "Wrong result of inserted element";
        }
        verif_tree(first);
    }

    current_min.key     = (int) 0x80000000;
    current_min.value   = (int) 0x80000000;

    for (i = 0; i < MAX_ELEMENT && element_in_tree != 0; i++) {
        tmp_elmnt.key       = (int) 0x7fffffff;
        tmp_elmnt.value     = (int) 0x7fffffff;
        // Get minimum data
        for (j = 0; j < MAX_ELEMENT; j++) {
            if (    data[j].key < tmp_elmnt.key
                &&  data[j].key > current_min.key) {
                tmp_elmnt.key   = data[j].key;
                tmp_elmnt.value = data[j].value;
            }

        }

        current_min.key     = tmp_elmnt.key;
        current_min.value   = tmp_elmnt.value;

        if (!is_present(first, &tmp_elmnt)) {
            ELOG("Minimum data not in tree");
            return "Minimum data not in tree";
        }
        delete_node_min(first);
        if (is_present(first, &tmp_elmnt)) {
            ELOG("Minimum element deleted");
            return "Minimum element deleted";
        }
        element_in_tree--;
        verif_tree(first);
    }

    // Try to delete it
    delete_tree(first);



    return NULL;
}
