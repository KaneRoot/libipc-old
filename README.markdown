# connection init (draft)

## how things happen

    * Service : daemon providing a feature (windowing, audio, …)
    * Application : specific application (browser, instant messaging, …)
    
    * [service] : service name
    * $pid : application PID
    * $index : process index (application point of view)

    1. Service creates a pipe named /tmp/[service]
    2. Application creates pipes named /tmp/$pid-$index-$version-{in,out}
    3. Application sends in /tmp/[service] : $pid $index $version

## pure "networking" view (what should go in the pipes)

1. Application sends in /tmp/[service] : $pid $index $version [...]

# messages format

First of all, the application will send a message to the service's pipe in **plain text** with its PID, the number of time the process already used the service (index) and the version of the communication protocol we want to use between the application and the service.

In order to communicate between the application and the service, we use the [CBOR format (RFC 7049)][cbor].
This will be used with some conventions.

## overview

The format will be "type : value".

The type will be a simple byte :

    * <0 - 15>   : control, meta data
    * <16 - 127> : later use
    * <128 - 255> : application specific (windowing system, audio system, …)

## CBOR type convention

0 - 15

    index   | abbreviation  | semantic
    0       | close         | to close the communication between the application and the service

[cbor]: https://tools.ietf.org/html/rfc7049
