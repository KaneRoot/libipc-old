# Service ping-pong

This service is a brain-dead application. It is only to a pedagogic end.

The purpose is only to communicate with an application once, the application
sends a message and the service answer with the same message.

# How it works

 * **S**: service
 * **A**: application

 1. **S** creates the named pipe /tmp/pingpong, then listens
 2. **S** opens the named pipes in & out
 3. **A** talks with the test program *pingpong.sh*
 4. **S** closes the test program named pipes
 5. **S** removes the named pipe /tmp/pingpong after 10 served applications

# pingpong.sh

The script *pingpong.sh* lets you test the service.

Usage :

    pingpong.sh [NB]
    # NB is the number of exchanged messages

    or

    pingpong.sh clean
    # it is to clean the /tmp/ipc/ directory
