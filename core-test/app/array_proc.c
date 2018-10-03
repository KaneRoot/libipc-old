#include "../../core/client.h"
#include <string.h> /* memset */
#include <stdio.h>

int main()
{
    int ret;
    struct ipc_client_array clients;
    memset(&clients, 0, sizeof(struct ipc_client_array));

    struct ipc_client client_tab[5];
    memset(&client_tab, 0, sizeof(struct ipc_client) * 5);

    int i;
    for (i = 0; i < 5; i++) {
        client_tab[i].proc_fd = i;
        ret = ipc_client_add(&clients, &client_tab[i]);
        if (ret == -1) {
            printf("erreur realloc\n");
        }
    }

    ipc_client_array_print(&clients);

    ret = ipc_client_del(&clients, &client_tab[2]);
    if(ret < 0) {
        printf("erreur %d\n", ret );
    }
    ipc_client_array_print(&clients);

    ipc_client_array_free (&clients);

    return 0;
}
