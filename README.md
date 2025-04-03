# iouring

This project leverages the Linux `io_uring` interface to provide high-performance asynchronous I/O operations. It was developed as part of my thesis to evaluate the performance of the `io_uring` interface compared to traditional I/O mechanisms like standard I/O and `liburing`. The project focuses on scenarios requiring efficient data transfer and low-latency networking, such as NAT (Network Address Translation) and other network-related tasks.

The implementation includes multiple modes of operation, such as:
- **Standard I/O**: Using traditional blocking and non-blocking I/O mechanisms.
- **io_uring**: A low-level interface for asynchronous I/O operations.
- **liburing**: A higher-level library built on top of `io_uring` for easier integration.
- **Parallel Modes**: Advanced configurations for parallel processing using `liburing`.

The project also includes benchmarking tools to measure the performance of these mechanisms under various configurations, such as queue depth, buffer size, and CPU affinity. These benchmarks help analyze the trade-offs between latency, throughput, and resource utilization.

## Getting Started

These instructions will guide you on how to build and run the project on your local machine for development and testing purposes.

### Build Binary
```bash
make clean all
```

### Cleanup
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

If a configuration file is in use, other parameters won’t be allowed.
```

### Default Configuration:
- **Name**: `nat1`
- **Address/Netmask**: `1.1.1.1/32`
- **Route**: `17.0.0.1/24 > 18.0.0.1/24`

### Examples:
1. **Run with a configuration file**:
   ```bash
   ./main -c /tmp/nat.conf
   ```

2. **Run with input parameters**:
   ```bash
   ./main -i nat1 -a 1.1.1.1/32 -r 17.0.0.1/24>18.0.0.1/24
   ```

3. **Run in multiprocess mode**:
   ```bash
   ./main -p nat1,1.1.1.1/32:nat2,2.2.2.2/32 -r 17.0.0.1/24>18.0.0.1/24
   ```

## Configuration File

You can serve parameters through a configuration file for easier management.

### Example Configuration File:
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

This project is licensed under the MIT License. See the [LICENSE](./LICENSE) file for details.

## Authors

Dennis Agostinho da Silva

## Acknowledgments

Special thanks to the open-source community for their support and inspiration.