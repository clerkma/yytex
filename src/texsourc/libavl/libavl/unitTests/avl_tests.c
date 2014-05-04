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

#include "../syslog.h"
#include "../avl.h"
#include "minunit.h"

int tests_run = 0;

extern char *alloc_tests();
extern char *insert_tests();
extern char *lookup_tests();
extern char *add_existing_tests();
extern char *delete_tests();
extern char *struct_tests();
extern char *get_data_tests();
extern char *print_tests();
extern char *explore_tests();
extern char *explore_restrain_tests();
extern char *same_element_values_tests();

static char *all_tests() {
    mu_run_test(alloc_tests);
    mu_run_test(insert_tests);
    mu_run_test(lookup_tests);
    mu_run_test(add_existing_tests);
    mu_run_test(delete_tests);
    mu_run_test(struct_tests);
    mu_run_test(get_data_tests);
    mu_run_test(print_tests);
    mu_run_test(explore_tests);
    mu_run_test(explore_restrain_tests);
    mu_run_test(same_element_values_tests);

    return NULL;
}

int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    mu_run_all(all_tests);

    return 0;
}
