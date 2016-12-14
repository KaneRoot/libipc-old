# connection init (draft)

## how things happen

    * Service: daemon providing a feature (windowing, audio, pubsub, …)
    * Application: specific application (browser, instant messaging, …)
    
    * service: service name
    * index: process index (to launch a service several times)
    * version: service version

    1. Service creates a unix socket /tmp/service-index-version.sock
    2. Application connects to /tmp/service-index-version.sock

## pure "networking" view (what should go in the sockets)

1. Application connects to /tmp/service-index-version.sock
1. Service acknowledges (empty message)

# messages format

In order to communicate between the application and the service, we use the Type-Length-Value format.
This will be used with some conventions.

## programming, debug

## overview

The format will be "type : value".

The type will be a simple byte :

    * <0 - 15>   : control, meta data
    * <16 - 127> : later use
    * <128 - 255> : application specific (windowing system, audio system, …)

    index   | abbreviation  | semantic
    0       | close         | to close the communication between the application and the service
    1       | message       | to send data 
    2       | error         | to send an error message
    3       | ack           | to send an acknowledgment
