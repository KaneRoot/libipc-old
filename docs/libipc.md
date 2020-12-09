## Before starting

This file is a presentation.
Better get the point tools here: https://git.baguette.netlib.re/Baguette/pointtools

Have fun!

## Why libIPC?

Network code separation
  * networking: performed by dedicated services
  * other applications: think all communications are local

Library code separation
  * make libraries language independent
  * micro-services are great, each of them does its part
    why not on base systems?


Applications are better than libraries
  * implementations change
  * languages change
  * API… not so much (not as often at least)

## Available libraries

* libevent
* DBUS
* even more complicated stuff
  * RPC-style, like Corba

#pause
* ... or bare libc api
  * pipes
  * sockets (unix, inet, inet6)
  * shared memory

## libevent

* works with epoll and kqueue
  * great performances
  * works on Linux and *BSD

* a bit complicated at first

## DBUS

* not well suited for our needs
  (a polite way to say: what a bloody mess)

Is it designed *NOT* to be used?
  * over-engineered
  * complex
  * documentation isn't great
    * no code example

And kinda obsolete: a big chunk of the documentation is
about message format. Just use CBOR already!

They even admit they did a poor job on the C part:
> There is a low-level C binding, but that is probably too detailed
> and cumbersome for anything but writing other bindings.

Oh. And C++. YOU SHALL NOT PASS!

This is a Linux requirement nowadays, wth?

## Bare libc api

All have great performances


shared memory and semaphores

* (kinda) complicated api
* not about exchanging messages


pipes, sockets

* lack a conventional message format
  ... but that's about it
  Great to start with!


## LibIPC history

1. based on pipes
  * because we gotta go fast!
  * ... but implementation was a bit of a mess

#pause
2. rewrite to work with unix sockets
  * performances are excellent, no need for absolute best
  * way nicer implementation
  * select(2) for listening on file descriptors
#pause
  * ... wait, does select(2) support more than 1024 connections?

#pause
3. rewrite using poll(2)
  * many bugfixes later, way more tested than before
  * implementation now kinda production-ready
  * enough for altideal.com at least

## Current implementation of libIPC

implementation is simple: < 2000 lines of C code

usage is simple
  1. init connection or server
  2. loop over events

bindings are available in Crystal
  * as well as fancy mappings: JSON and CBOR class serialization

#pause
epoll (Linux) and kqueue (*BSD) were avoided
  * 'cause callbacks hell => harder to read and to write code
#pause
  * but we need them for better performances with many connections
    though, API should stay the same for non threaded applications (simple implementation)

#pause
LibIPC doesn't handle parallelism, yet

## How libIPC works

LibIPC has a high level API for the user

1. init a connection (client) or create an unix socket (service)

  example: ipc_server_init (context, "service")

#pause
2. loop, wait for events
   listening to file descriptors (libIPC ones or not)

  example:
    while(1) {
      wait_event (context, &event, &timer)
      switch (event.type) {
        case IPC_EVENT_TYPE_CONNECTION : ...
        case IPC_EVENT_TYPE_DISCONNECTION : ...
        case IPC_EVENT_TYPE_MESSAGE: {
          struct ipc_message *m = event.m;
          ...
        }
      }
    }

## How libIPC works

3. send messages

    struct ipc_message m
    m.payload   = ...
    m.length    = strlen(m.payload)
    m.fd        = event.fd
    m.type      = ...
    m.user_type = ...

    ipc_write (context, &m)

#pause
4. add and remove fd from the context

    ipc_add_fd (context, fd)
    ipc_del_fd (context, fd)

## How libIPC works

LibIPC also helps to create "protocol daemons" like TCPd with
automatic switching between file descriptors

LibIPC takes callbacks to obtain libipc payloads inside arbitrary message structure

Example: websocketd.
         Clients exchange data with a libipc service through websockets messages.

         websocketd binds both the client and its service file descriptors,
         then provides the libipc a callback to extract libipc messages from
         the websocket messages sent by the client.

         Same thing the other way.


         ipc_switching_callbacks (context, client_fd, cb_in, cb_out)

## libIPC internal structures

Main goal: simplest possible structures

Examples:

  Message
    struct ipc_message {
      char type;        => Internal message type, used by protocol daemons.
      char user_type;   => User-defined message type (arbitrary).
      int fd;           => File descriptor concerned about this message.
      uint32_t length;  => Payload length.
      char *payload;    => Actual payload.
    };

  Context of the whole networking state
    struct ipc_ctx {
      struct ipc_connection_info *cinfos;  => Keeps track of connections.
      struct pollfd *pollfd;               => List of "pollfd" structures within cinfos,
                                              so we can pass it to poll(2).
      size_t size;                         => Size of the connection list.
      struct ipc_messages tx;              => Messages to send.
      struct ipc_switchings switchdb;      => Relations between fd.
    };


## Future of libIPC

LibIPC will be rewritten in Zig

* simpler to read and write
  * data structures are simpler to create with comptime
  * less code redundancy (more generic functions)
  * already existing error management as written in current libIPC
  * already existing loggin system
  * no more C's pitfalls

* way better at exposing bugs
  * thanks to a better type system

* way safer: cannot ignore errors
  * so I won't

* simpler to cross-compile: same standard library for every OSs
* simpler (and more secure) memory management


Also: this won't change existing bindings! No excuses!


## Why not use it?

Current limitations

* performances (libIPC is based on poll(2), not epoll nor kqueue)
  * it really isn't an issue until you have hundreds or thousands of clients

* parallelism is not permitted
  nothing in libIPC is thread-safe


The future Zig implementation will overcome these issues.

## Questions?

Ask! `karchnu at karchnu.fr`
