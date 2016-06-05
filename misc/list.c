#include <stdio.h>
#include <stdlib.h>
#include "../lib/queue.h"

// create the head of the list
LIST_HEAD(mlist, node);

// elements structure of the list
struct node {
    int content;
    LIST_ENTRY(node) entries;
};

int main(int argc, char * argv[])
{
    // the list
    struct mlist *list;
    list = malloc (sizeof(struct mlist));
    LIST_INIT(list);

    // create the elements
    struct node *n1 = malloc (sizeof(struct node));
    n1->content = 10;
    struct node *n2 = malloc (sizeof(struct node));
    n2->content = 20;

    // insert element into the list
    LIST_INSERT_HEAD(list, n1, entries);
    LIST_INSERT_HEAD(list, n2, entries);

    // loop over the list
    struct node *np = NULL;
    LIST_FOREACH(np, list, entries) {
        printf ("elem : %d\n", np->content);
    }

    // remove elements from the list
    LIST_REMOVE(n1, entries);
    LIST_REMOVE(n2, entries);

    // to be sure that nothing still is into the list
    np = NULL;
    LIST_FOREACH(np, list, entries) {
        printf ("\033[31mSHOULD NOT BE PRINTED : %d\033[00m\n", np->content);
    }

    // free the elements then the list itself
    free (n1);
    free (n2);
    free (list);

    return EXIT_SUCCESS;
}
