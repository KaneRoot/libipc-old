#ifndef LIBIPC
#define LIBIPC

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

#endif
