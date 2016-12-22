#include "../../core/process.h"
#include <string.h> /* memset */
#include <stdio.h>

int main()
{
    int ret;
    struct array_proc tab_proc;
    memset(&tab_proc, 0, sizeof(struct array_proc));

    struct process process_tab[5];
    memset(&process_tab, 0, sizeof(struct process) * 5);

    int i;
    for (i = 0; i < 5; i++) {
        process_tab[i].proc_fd = i;
        ret = add_proc(&tab_proc, &process_tab[i]);
        if (ret == -1) {
            printf("erreur realloc\n");
        }
    }

    array_proc_print(&tab_proc);

    ret = del_proc(&tab_proc, &process_tab[2]);
    if(ret < 0) {
        printf("erreur %d\n", ret );
    }
    array_proc_print(&tab_proc);

    array_proc_free (&tab_proc);

    return 0;
}
