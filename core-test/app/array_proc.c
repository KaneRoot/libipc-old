#include "../../core/process.h"
#include <string.h> /* memset */
#include <stdio.h>

int main()
{
    int ret;
    struct ipc_process_array tab_proc;
    memset(&tab_proc, 0, sizeof(struct ipc_process_array));

    struct ipc_client process_tab[5];
    memset(&process_tab, 0, sizeof(struct ipc_client) * 5);

    int i;
    for (i = 0; i < 5; i++) {
        process_tab[i].proc_fd = i;
        ret = ipc_process_add(&tab_proc, &process_tab[i]);
        if (ret == -1) {
            printf("erreur realloc\n");
        }
    }

    ipc_process_array_print(&tab_proc);

    ret = ipc_process_del(&tab_proc, &process_tab[2]);
    if(ret < 0) {
        printf("erreur %d\n", ret );
    }
    ipc_process_array_print(&tab_proc);

    ipc_process_array_free (&tab_proc);

    return 0;
}
