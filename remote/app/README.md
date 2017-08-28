# remoted

This service creates a path on the relevent remote location, going through anything network-related: TCP, UDP, HTTP, ...

# TODO

* authorizations
* code the -d option

# Connection

Client -> Remoted: service to contact (ex: pongd)

           format: [u8 (action); u16 (length); XXX (options)]

Client -> Remoted: action (connect|listen) + options

           format: [u8 (action); u16 (length); XXX (options)]

        example 1: action = connect => options = uri (ex: udp://example.com:5000)
           format: [u8 (1); u16 (22); udp://example.com:5000]

        example 2: action = listen => options = uri (ex: tcp://localhost:9000)
           format: [u8 (2); u16 (20); tcp://localhost:9000]

(optional) Client -> Remoted: options (environement variables)

          example: action = options => option = VAR=X
           format: [u8 (4); u16 (20); VAR=X]

           The client sends all options this way, one at a time.
           This sequence of messages is ended with the following message.

Client -> Remoted: END

           format: [u8 (5)]

Remoted -> Client: unix socket

In the case the application has environement variables to pass to the remoted service, 

### authorizations

The idea is to have a simple configuration file for authentication of remote connections, such as:

    table dynusers    # dynamic user table
    
    clients = { "client123", alice.example.com, john@doe.com }
    localclients = { pamuser1, <dynusers> }
    
    level1services = { pongd, weather }
    
    ifext = enp0s25
    pass in on $ifext from any for all to local services $level1services
    pass out on $ifext from local for $localclients to any services $level1services
    
    block all
