# remoted

This service creates a path on the relevent remote location, going through anything network-related: TCP, UDP, HTTP, ...

# TODO

### authorizations

The idea is to have a simple configuration file for authentication of remote connections, such as:

    
    clients = { "client123", alice.example.com, john@doe.com }
    level1services = { pongd, weather }

    ifext = enp0s25
    pass in on $ifext from any for all to local services $level1services
    pass in on $ifext from any for all to local services $level1services
