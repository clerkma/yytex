/****************************************************************************************
       DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE 
                   Version 2, December 2004 

Copyright (C) 2013 Adrien Oliva <adrien.oliva@yapbreak.fr>

Everyone is permitted to copy and distribute verbatim or modified 
copies of this license document, and changing it is allowed as long 
as the name is changed. 

           DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE 
  TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION 

 0. You just DO WHAT THE FUCK YOU WANT TO.
****************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include "avl.h"

// Structure we want to store
// key is used to order data
struct data {
    int key;
    int value;
};

// Function that compares two struct data
int data_cmp(void *a, void *b)
{
    struct data *aa = (struct data *) a;
    struct data *bb = (struct data *) b;

    // Protect against NULL pointer
    // It could generally never happened
    if (!aa || !bb)
        return 0;

    return aa->key - bb->key;
}

// Function that dumps data structure
void data_print(void *d)
{
    struct data *dd = (struct data *) d;

    if (dd)
        printf("{ %d => %d }\n", dd->key, dd->value);
}

// Function that delete a data structure
void data_delete(void *d)
{
    struct data *dd = (struct data *) d;

    if (dd) {
        // You can put here all additional needed
        // memory deallocation
        free(dd);
    }
}

// Function that copy data structure
void data_copy(void *src, void *dst)
{
    struct data *s = (struct data *) src;
    struct data *d = (struct data *) dst;

    d->key = s->key;
    d->value = s->value;
}

int main(int argc, char **argv)
{
    tree *avl_tree = NULL;
    struct data tmp;
    unsigned result;

    (void) argc;
    (void) argv;

    // Initialize a new tree with our three previously defined
    // functions to store data structure.
    avl_tree = init_dictionnary(data_cmp, data_print, data_delete, data_copy);

    tmp.key = 42;
    tmp.value = 4242;

    // Add element {42, 4242} in our tree.
    result = insert_elmt(avl_tree, &tmp, sizeof(struct data));
    // Here result is equal to 1 since there is only 1 element in tree.
    printf("Result after first insert: %d\n", result);

    // Dump tree to stdout with data_print function
    print_tree(avl_tree);

    // For all search function, the only value needed in tmp structure
    // is key field.
    tmp.key = 20;
    tmp.value = 0;

    if (!is_present(avl_tree, &tmp))
        printf("Key 20 is not found.\n");

    tmp.key = 42;
    if (is_present(avl_tree, &tmp))
        printf("Key 42 exist in tree.\n");

    if (get_data(avl_tree, &tmp, sizeof(struct data)))
        printf("Now, tmp.key is equal to 4242\n");

    delete_node(avl_tree, &tmp);
    if (!is_present(avl_tree, &tmp))
        printf("Key 42 does not exist anymore.\n");

    // Free all memory
    delete_tree(avl_tree);

    return 0;
}


