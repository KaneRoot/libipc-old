# Problem

End-user applications are huge, with tons of libraries used and are a pain in the ass to maintain.
Libraries are huge, with a lot of things happening, with changes that break everything on a regular basis, with almost each time very few people working on it.
Libraries are written for every project, most of the code can be re-used because it is not fundamentally bounded to a single project.

Start a new project is not an easy task neither.

- What language to use?
- I am doing something already coded somewhere?
- Is this library working on every platform?
- Are the libraries I need (and the good version) available for my platform, do I need to install SomeBullshitOS or ProtoContenerizator3000 to code?

# How to change that?

**Network protocols**

Network protocols are awesome: very few changes, well documented, programming language agnostics.

Don't (just) write libraries, write applications !

Your need a functionality in your application, why do you have to code it ?
Just ask a service !

**Example**

You want to download a file, you will always have the same input: a string corresponding to the file to get, such as _ftp://example.com/file.txt_.
You don't have to worry about the protocol to use in your own application, the burden is on the dedicated *downloading* service.

# Benefits

**Awesome abstractions**.

You will be able to do things without any code.

* applications don't have to know if the services they use is on the network or on your own computer
* applications don't need to change anything to handle new protocols, no recompilation
* applications can be statically compiled, the memory footprint should be extremely low (yes, even for a browser)

Let's write *abstractions* together, and less application-specific code !

**Simple applications**.

You only need to code the specific parts of your own application.
The only thing your application should have to take care is its functionality, and to communicate to services providing abstractions.

**Consistency**.

Everything will be developed in the same repository: same [coding standards][codingstyle], changes will be tested on every provided applications…

**Code review**.

We should always try to provide new abstractions, reducing the code needed in both services and end-user applications.
To that end, code review is a must.

**No need to rewrite everything**.

You have great libraries?
Don't redevelop them!
We can use already existing libraries to provide new functionalities to our programs: we just have to write a service and to define new messages to request it, period.

**Language independent**.

You have an awesome library to do X, but it's written in an obscure language.
Who cares?
Write a simple service that can be requested following our protocol, everybody will be able to use your library without painful-to-maintain bindings!
We may even assist you doing that by providing templates for your language, or check the other services!

**The end of "oh, I would like to dev something but this requires too much painful-to-install dependencies"**.

You only need the communication library and the service running (not even on your own computer) without any other dependencies.
That's it, you're good to go!

# Not adapted to everything

If you need incredible performances for your application, maybe this won't fit.
There is no silver bullet or one-fit-all solution.
Still, we think performances won't be much of a problem for most of the everyday life applications and if there are performances hits we still have plenty of room for optimisations!

# Application and services

- Services: daemons providing a feature (windowing, audio, network, input, pubsub, …)
- Applications: end-user applications (browser, mail user agent, instant messaging app, …)

#### Examples

A browser that can download everything, via every existing protocol.
No any specific code in the browser itself and no configuration.

You want to play a game on your raspberry pi, but it is not powerful enough, run your application on your laptop but take the inputs from the rpi (or anywhere on the network) !
No specific code needed.

# TODO

Figures, a lot of them, to explain everything.

# Connection init (draft)

## How things happen

1. Service creates a unix socket /tmp/service-index-version.sock
1. Application connects to /tmp/service-index-version.sock

__legend__:
    - service: service name
    - index: process index (to launch a service several times)
    - version: service version

# Networking point of view (what should go in the sockets)

#### Connection
1. Application connects to /tmp/service-index-version.sock
1. Service acknowledges (empty message)

#### Disconnection
1. Application sends a message "CLOSE" to the server

#### Data
1. Application or server sends a message "DATA", no acknowledgement

# Message formats

In order to communicate between the application and the service, we use the Type-Length-Value format.
This will be used with some conventions.

The type will be a simple byte :

    * <0 - 15>   : control, meta data
    * <16 - 127> : later use
    * <128 - 255> : application specific (windowing system, audio system, …)

    index   | abbreviation  | semantic
    0       | close         | to close the communication between the application and the service
    1       | connection    | to connect to the service
    2       | error         | to send an error message
    3       | ack           | to send an acknowledgment
    4       | message       | to send data 

# Service Status

Go to the relevant directory for details.

- pongd: stable
- tcpd: experimental
- pubsub: experimental

# Inspiration

This project is inspired by a number of great projects:

- [OpenBSD][openbsd] and UNIX in general for most of the concepts
- Plan9 for the great abstractions and simplicity
- [suckless][suckless] for the [coding style][codingstyle] and [cat-v][catv] for the philosophy

[codingstyle]: http://suckless.org/coding_style
[suckless]: http://suckless.org
[catv]: http://cat-v.org
[openbsd]: https://openbsd.org
