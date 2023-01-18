#ifndef LIBIPC
#define LIBIPC

struct message {
	uint32_t size;
	char* payload;
};

enum event_types {
	  ERROR = 0         // A problem occured.
	, EXTERNAL = 1      // Message received from a non IPC socket.
	, SWITCH_RX = 2     // Message received from a switched FD.
	, SWITCH_TX = 3     // Message sent to a switched fd.
	, CONNECTION = 4    // New user.
	, DISCONNECTION = 5 // User disconnected.
	, MESSAGE = 6       // New message.
	, TIMER = 7         // Timeout in the poll(2) function.
	, TX = 8            // Message sent.
};

int ipc_context_init (void** ptr);
int ipc_service_init (void* ctx, int* servicefd, const char* service_name, unsigned short service_name_len);
int ipc_connect_service (void* ctx, int* servicefd, const char* service_name, unsigned short service_name_len);
void ipc_context_deinit (void* ctx);
int ipc_write (void* ctx, int servicefd, char* mcontent, unsigned int mlen);
int ipc_schedule (void* ctx, int servicefd, const char* mcontent, unsigned int mlen);
int ipc_read_fd (void* ctx, int fd, char* buffer, size_t* buflen);
int ipc_read (void* ctx, size_t index, char* buffer, size_t* buflen);
int ipc_wait_event(void* ctx, char* t, size_t* index, int* originfd, char* buffer, size_t* buflen);
void ipc_context_timer (void* ctx, int timer);
int ipc_close_fd (void* ctx, int fd);
int ipc_close (void* ctx, size_t index);
int ipc_add_external (void* ctx, int newfd);
int ipc_add_switch (void* ctx, int fd1, int fd2);

#endif
