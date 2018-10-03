#include "../../core/client.h"
#include <string.h> /* memset */
#include <stdio.h>

int main()
{
    int ret;
    struct ipc_client_array tab_proc;
    memset(&tab_proc, 0, sizeof(struct ipc_client_array));

    struct ipc_client client_tab[5];
    memset(&client_tab, 0, sizeof(struct ipc_client) * 5);

    int i;
    for (i = 0; i < 5; i++) {
        client_tab[i].proc_fd = i;
        ret = ipc_client_add(&tab_proc, &client_tab[i]);
        if (ret == -1) {
            printf("erreur realloc\n");
        }
    }

    ipc_client_array_print(&tab_proc);

    ret = ipc_client_del(&tab_proc, &client_tab[2]);
    if(ret < 0) {
        printf("erreur %d\n", ret );
    }
    ipc_client_array_print(&tab_proc);

    ipc_client_array_free (&tab_proc);

    return 0;
}
