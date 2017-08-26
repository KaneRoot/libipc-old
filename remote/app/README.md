# remoted

This service creates a path on the relevent remote location, going through anything network-related: TCP, UDP, HTTP, ...

# TODO

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
