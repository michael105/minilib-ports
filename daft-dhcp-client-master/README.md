## state

wip - compiles, but sent ip isn't parsed correctly.

;) Bug fix welcome.

misc 2021




# daft-dhcp-client

Small and stupid DHCP client written in C.
The only thing shared with the DHCP server is your MAC address.

#### Motivation
Study linux networking programming.

#### Installation
```bash
git clone --depth=1 https://github.com/anryko/daft-dhcp-client.git
cd daft-dhcp-client
make
```

#### Usage
```bash
$ ./daft-dhcp-client -h
Usage: ./daft-dhcp-client -i <interface> [-h] [-v] [-d] [-r <ip>] [-q <ip>] [-m <MAC>] [-t <sec>]

        -h --help                       This help message
        -v --verbose                    Print debugging info to stderr
        -i --interface <interface>      Interface name
        -d --discover                   Discover DHCP Server
        -r --request <ip>               Request IP lease
        -q --release <ip>               Release IP lease
        -m --mac <MAC>                  MAC address
        -t --timeout <sec>              Timeout (default 5 sec.)

$ sudo ./daft-dhcp-client -i eth0
Your-MAC c0:28:96:11:f4:7a
Your-IP 10.0.2.16
Message-Type OFFER
Subnet-Mask 255.255.255.0
Default-Gateways 10.0.2.2
Domain-Name-Servers 10.0.2.3
Lease-Time 86400
Server-ID 10.0.2.2
$ sudo ./daft-dhcp-client -i eth0 -r 10.0.2.16
Your-MAC c0:28:96:11:f4:7a
Your-IP 10.0.2.16
Message-Type ACK
Subnet-Mask 255.255.255.0
Default-Gateways 10.0.2.2
Domain-Name-Servers 10.0.2.3
Lease-Time 86400
Server-ID 10.0.2.2
$ sudo ./daft-dhcp-client -i eth0 -q 10.0.2.16
```

You can parse *stdout* and configure network interface accordingly.
