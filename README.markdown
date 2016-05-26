# connection init (draft)

## what the programs should do and how they interact

    * service : application providing a feature to others (windows, audio, …)
    * program : specific application (browser, instant messaging, …)
    
    * [service] : service name
    * [index] : process index in the program point of view
    * [pindex] : process index in the service point of view

    1. the service creates a pipe, named /tmp/[service]
    2. the program creates pipes named /tmp/$pid-[index]-{in,out}
    3. the program prints in the pipe /tmp/[service] : $pid-[index] version
    4. depending on the configuration and service type, the service will
    
      * thread, to spare resources
      * fork, not to compromise the security
    
    5. the service prints [pindex] in /tmp/$pid-[index]-in

## pure "networking" view (what should go in the pipes)

1. the program prints in the pipe /tmp/[service] : $pid-[index] version
2. the service prints [pindex] in /tmp/$pid-[index]-in

# messages format

QUESTION : no CBOR for 1 & 2, or CBOR everywhere ?

## overview

format "type : value"

type will be a simple byte :

    * <0 - 15>   : control, meta data
    * <16 - 127> : later use
    * <128 - 255> : application specific (windowing system, audio system, …)

## CBOR table (TODO)

index | semantic
