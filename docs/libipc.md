## Before starting

This file is a presentation based on the point tools:
https://git.baguette.netlib.re/Baguette/pointtools

To see it, type 'make'.

TODO: Explain the problem
TODO: Explain the solution
TODO: Explain why LibIPC
TODO: Explain how LibIPC
TODO: Explain other possible implementations
TODO: Explain future of LibIPC
TODO: Explain what can be done right now
TODO: Explain what I actually do with it
TODO: Explain LibIPC isn't a silver bullet

Have fun!

## Programming problems

Libraries
* change often = hard to follow
* don't often provide a high-level interface
* each library is coded in its own way
* availability vary depending on the language

Example: libraries to access databases
* languages often have their own implementation
* functions may vary from a language to another

## Infrastructure problems

Infrastructure
* Always need to install all required libraries
* No clear way to sandbox part of an application

## What solution?

MAKE APPLICATIONS, NOT LIBRARIES
  * apps talking to apps

Create an abstraction for libraries
  * languages, implementations, code modifications in general

Create an abstraction for network code
  * applications think communications are local
  * networking is performed by dedicated services
    * examples: TCPd, UDPd, TLSd, HTTPd...
  * apps are independant from protocols and formats
    (unless they are fundamentaly network-related)

## In practice

usage must be simple

  1. init connection or service
  2. loop over events

#pause
events are simple and high level

  1. connection and disconnection
  2. message received and sent

#pause
message have a simple format: length + value

#pause
And that's it.
## When

LibIPC is useful when the app:

- cannot be a simple shell command
- needs a bidirectional communication
- is an abstraction over a library

## Available libraries

* DBUS
* libevent
* even more complicated stuff
  * RPC-style, like Corba

#pause
* ... or bare libc api
  * shared memory
  * pipes
  * sockets (unix, inet, inet6)

## DBUS

* not well suited for our needs
  (a polite way to say: what a bloody mess)

Is it designed *NOT* to be used?
  * over-engineered
  * complex
  * documentation isn't great
    * no code example

## DBUS (bonus page!)

DBUS feels obsolete: a big chunk of the documentation is
about message format. Just use CBOR already!

#pause
They even admit they did a poor job on the C part:
> There is a low-level C binding, but that is probably too detailed
> and cumbersome for anything but writing other bindings.

#pause
Oh. And C++. YOU SHALL NOT PASS!

This is a Linux requirement nowadays, wth?

## libevent

* works with epoll and kqueue
  * great performances
  * works on Linux and *BSD

* a bit complicated

## Bare libc api

shared memory and semaphores

* (kinda) complicated api
* not about exchanging messages


pipes, sockets

* lack a conventional message format
  ... but that's about it
* Great to start with!

All have great performances to exchange data.

What is slow is the function to _wait_ for new events.

## LibIPC's choice

Unix sockets
- fast, simple, reliable, bidirectional
- remote connections will have their own service (ex: TCPd)

Dumbest possible message format
- length + value
- build your own format on top of it!

Wait on file descriptors with poll(2)
- slow, but available everywhere
- may upgrade to libevent

## LibIPC history (1/3)

1. based on pipes
  * because we gotta go fast!
  * ... but implementation was a bit of a mess

#pause
2. rewrite to work with unix sockets
  * performances are excellent, no need for _absolute best_
  * way nicer implementation
  * select(2) for listening on file descriptors
#pause
  * ... wait, does select(2) support more than 1024 connections?

## LibIPC history (2/3)

3. rewrite using poll(2)
  * many bugfixes later, way more tested than before
  * implementation was (kinda) production-ready
  * implementation was simple: < 2000 lines of C code

Still wasn't as simple as I wanted

## LibIPC history (3/3)

4. rewrite in Zig
  * still uses poll(2) (at least for now)
  * C-compatible bindings are available

## Why Zig? (1/2)

error management is built-in and mandatory

simpler to read and write
  * nicer data structures (contain functions)
  * less code redundancy (defer, more generic functions)
  * no more C's pitfalls
  * fully qualified names

## Why Zig? (2/2)

better standard library
  * usual structures: lists, hashtables
  * log system

memory management is simpler, more secure and more flexible

better at exposing bugs (better type system)

simpler to cross-compile: same standard library for all OSs

## Current implementation of libIPC

bindings available in Crystal
  * as well as fancy mappings: JSON and CBOR class serialization

#pause
epoll (Linux) and kqueue (*BSD) were avoided
  * because callbacks hell => harder to read and to write code
#pause
  * still a possibility for someday, not the priority right now

#pause
LibIPC doesn't handle parallelism, yet

## How libIPC works (in Zig)

LibIPC has a high level API

	var context = try Context.init(allocator);
	defer context.deinit();

#pause

	var pong_fd = try context.connect_service ("pong");
	var message = try Message.init (pong_fd, allocator, "hello");
	try context.schedule (message);

## How libIPC works (in Zig)

	var event = try context.wait_event();

	switch (event.t) {
	    ...
	}

## How libIPC works (in Zig)

	var event = try context.wait_event();

	switch (event.t) {

	    .CONNECTION => {
	        print ("New client!\n", .{});
	    },

	    ...
	}


## How libIPC works (in Zig)

	var event = try context.wait_event();

	switch (event.t) {

	    .CONNECTION => {
	        print ("New client!\n", .{});
	    },

	    .MESSAGE_RX => {
	        if (event.m) |m| {
	            print ("a message has been received: {s}\n", .{m});
	        }
	    }
	    ...
	}

## How libIPC works (bindings)

1. init a connection (client) or create an unix socket (service)

  ipc_connect_service (context, &fd, service_name, service_len)
  ipc_service_init (context, &fd, service_name, service_len)

#pause
2. loop, wait for events
   listening to file descriptors (libIPC ones or not)

  example:

    while(1) {
      ipc_wait_event (context, &type, &index, &fd, buffer, &buffer_len)
      switch (type) {
        case IPC_CONNECTION : ...
        case IPC_DISCONNECTION : ...
        case IPC_MESSAGE: ...
      }
    }

## How libIPC works

3. send messages

```c
    ipc_schedule (context, fd, buffer, buffer_len)
    or
    ipc_write (context, fd, buffer, buffer_len)
```

#pause
4. add a file descriptor to listen to

    ipc_add_external (context, fd)

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

## libIPC internal structures (1/2)

Main goal: simplest possible structures

Examples (nothing hidden):

    Message {
      fd: i32                        => File descriptor concerned about this message.
      payload: []u8                  => Actual payload.
      allocator: std.mem.Allocator   => Memory management.
    };

    Event {
      t: Event.Type     => Example: connection, message tx, ...
      m: ?Message       => Message, if there is one.
      index: usize      => (Internal stuff).
      originfd: i32     => File descriptor related to the event.
    };

## libIPC internal structures (2/2)

Context structure is slightly more complicated, but _reasonable_.

	Context {
	    rundir: [] u8,      // Where the UNIX sockets are.
	    pollfd: PollFD,     // File descriptors to manage.
	    tx: Messages,       // Messages to send, once their fd is available.
	    ...
	};

The rest is implementation details (and more advanced usage of LibIPC).

## Future of libIPC

## Why not use it?

Current limitations

* performances (libIPC is based on poll(2), not epoll nor kqueue)
  * it really isn't an issue until you have hundreds of clients
  * LibIPC could someday use libevent

* nothing in libIPC is thread-safe

These limitations are the price for a simple implementation.

## Questions?

Ask! `karchnu at karchnu.fr`
