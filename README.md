# IO_URING

This is a brief description of what the project does and its main functionality.

## Getting Started

These instructions will guide you on how to get an executable of the project up and running on your local machine for development and testing purposes.

Build binary
```bash
make clean all
```

Cleanup
```bash
make clean
```

## Usage

```bash
vm-2:/home/user/build/iouring: ./nat -h
Usage: ./main -c <config_file_path>
       ./main -i <name> -a <addr/netmask> -r <route>
       ./main -p <name1>,<addr1/netmask1>:<name2>,<addr2/netmask2> -r <route>

Configuration over file:
        -c <config_file_path>       Config File Path.

Configuration over arguments:
        -i <name>                   Interface Name.
        -a <addr/netmask>           Interface Address and Netmask.
        -r <src_route>,<dest_route> Interface Routes.

Optional:
        -l <loglevel>               Log Level.
        -L <log_file_path>          File Path for Logging.
        -h                          Print Help.
        -?                          Print Help.

If configuration file is in use, other parameters won`t be allowed.


Default:
       name            = nat1
       address/netmask = 1.1.1.1/32
       route           = 17.0.0.1/24>18.0.0.1/24

Examples:
    Run with configuration file
       ./main -c /tmp/nat.conf

    Run with input parameters
       ./main -i nat1 -a 1.1.1.1/32 -r 17.0.0.1/24>18.0.0.1/24

    Run in multiprocess mode (creates 2 processes, each handle one direction)
       ./main -p nat1,1.1.1.1/32:nat2,2.2.2.2/32 -r 17.0.0.1/24>18.0.0.1/24

```


### Configuration File

Serving parameters over config file.

```bash
vm-2:/home/user/build/iouring: cat nat.conf

# Configuration File for NAT

[Network Interface]
name1=nat1
addr1=1.1.1.1\32

name2=nat2
addr2=2.2.2.2\32

[Route]
route1=17.0.0.1\32
route2=18.0.0.1\32

[Mode] # 0: NAT 1: iouring 2: liburing 3: liburing_parallel
mode=0

[Logging]
loglevel=INFO
logfile=/home/user/build/iouring/nat.log
```


## License


## Authors

Dennis Agostinho da Silva

## Acknowledgments