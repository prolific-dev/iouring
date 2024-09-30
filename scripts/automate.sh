#!/bin/sh

REMOTE_USER="w123694"
REMOTE_CLIENT="192.168.254.221"
REMOTE_SERVER="192.168.254.223"

DUMMY_SERVER_IP="17.0.0.1"
DUMMY_SERVER_PORT="12345"

CRS_TOOLS_DIR="/home/$REMOTE_USER/crs-tools"
BUILD_DIR="/home/$REMOTE_USER/build/iouring"
CONF_DIR="/home/$REMOTE_USER/build/iouring/conf"

CONFIG_FILE="nat.conf"

run_nat() {
    $BUILD_DIR/nat -c $BUILD_DIR/$CONFIG_FILE &
    sleep 2
}

run_nat_conf() {
    file=$1
    $BUILD_DIR/nat -c $BUILD_DIR/conf/$file &
    sleep 2
}

run_nat_conf_tcp() {
    file=$1
    $BUILD_DIR/nat -c $BUILD_DIR/conf/tcp/$file &
    sleep 2
}

close_nat() {
    echo "exit" | nc -u -w0 0.0.0.0 12000
    sleep 2
}

run_test() {
    for i in {1..5}; do
        echo "start capture" | nc -u -w0 0.0.0.0 12000
        ssh -t -t $REMOTE_USER@$REMOTE_CLIENT $1 &> /dev/null
        echo "stop capture" | nc -u -w0 0.0.0.0 12000
        sleep 2
    done
}

run_test_tcp() {
    run_nat
    for i in {1..2}; do
        echo "start capture" | nc -u -w0 0.0.0.0 12000
        ssh -t -t $REMOTE_USER@$REMOTE_CLIENT $1 &> /dev/null
        echo "stop capture" | nc -u -w0 0.0.0.0 12000
        sleep 2
    done
    close_nat
}

change_config() {
    key=$1
    value=$2
    conf=$3

    if [ -z "$key" ] || [ -z "$value" ]; then
        echo "Usage: change_config <key> <value>"
        return
    fi

    if [ "$key" = "logfile" ]; then
        sed -i "s|^$key=.*|$key=$value|" "$BUILD_DIR/conf/$conf"
    else
        sed -i "s/^$key=.*/$key=$value/" "$BUILD_DIR/conf/$conf"
    fi
}

# BPL 25Mbit
case_100BPL_750M_25Mbit() {
    run_test "iperf3 -u -n 750M -b 25M -l 100 -P 1 -c 17.0.0.1 -V"
}

case_200BPL_750M_25Mbit() {
    run_test "iperf3 -u -n 750M -b 25M -l 200 -P 1 -c 17.0.0.1 -V"
}

case_400BPL_750M_25Mbit() {
    run_test "iperf3 -u -n 750M -b 25M -l 400 -P 1 -c 17.0.0.1 -V"
}

case_800BPL_750M_25Mbit() {
    run_test "iperf3 -u -n 750M -b 25M -l 800 -P 1 -c 17.0.0.1 -V"
}

case_1200BPL_750M_25Mbit() {
    run_test "iperf3 -u -n 750M -b 25M -l 1200 -P 1 -c 17.0.0.1 -V"
}

case_1448BPL_750M_25Mbit() {
    run_test "iperf3 -u -n 750M -b 25M -l 1448 -P 1 -c 17.0.0.1 -V"
}

case_100BPL_750M_25Mbit_tcp() {
    run_test "iperf3 -n 750M -b 25M -l 100 -P 1 -c 17.0.0.1 -V"
}

case_200BPL_750M_25Mbit_tcp() {
    run_test "iperf3 -n 750M -b 25M -l 200 -P 1 -c 17.0.0.1 -V"
}

case_400BPL_750M_25Mbit_tcp() {
    run_test "iperf3 -n 750M -b 25M -l 400 -P 1 -c 17.0.0.1 -V"
}

case_800BPL_750M_25Mbit_tcp() {
    run_test "iperf3 -n 750M -b 25M -l 800 -P 1 -c 17.0.0.1 -V"
}

case_1200BPL_750M_25Mbit_tcp() {
    run_test "iperf3 -n 750M -b 25M -l 1200 -P 1 -c 17.0.0.1 -V"
}

case_1448BPL_750M_25Mbit_tcp() {
    run_test "iperf3 -n 750M -b 25M -l 1448 -P 1 -c 17.0.0.1 -V"
}

# BPL 100Mbit
case_100BPL_750M_100Mbit() {
    run_test "iperf3 -u -n 750M -b 100M -l 100 -P 1 -c 17.0.0.1 -V"
}

case_200BPL_750M_100Mbit() {
    run_test "iperf3 -u -n 750M -b 100M -l 200 -P 1 -c 17.0.0.1 -V"
}

case_400BPL_750M_100Mbit() {
    run_test "iperf3 -u -n 750M -b 100M -l 400 -P 1 -c 17.0.0.1 -V"
}

case_800BPL_750M_100Mbit() {
    run_test "iperf3 -u -n 750M -b 100M -l 800 -P 1 -c 17.0.0.1 -V"
}

case_1200BPL_750M_100Mbit() {
    run_test "iperf3 -u -n 750M -b 100M -l 1200 -P 1 -c 17.0.0.1 -V"
}

case_1448BPL_750M_100Mbit() {
    run_test "iperf3 -u -n 750M -b 100M -l 1448 -P 1 -c 17.0.0.1 -V"
}

case_100BPL_750M_100Mbit_tcp() {
    run_test "iperf3 -n 750M -b 100M -l 100 -P 1 -c 17.0.0.1 -V"
}

case_200BPL_750M_100Mbit_tcp() {
    run_test "iperf3 -n 750M -b 100M -l 200 -P 1 -c 17.0.0.1 -V"
}

case_400BPL_750M_100Mbit_tcp() {
    run_test "iperf3 -n 750M -b 100M -l 400 -P 1 -c 17.0.0.1 -V"
}

case_800BPL_750M_100Mbit_tcp() {
    run_test "iperf3 -n 750M -b 100M -l 800 -P 1 -c 17.0.0.1 -V"
}

case_1200BPL_750M_100Mbit_tcp() {
    run_test "iperf3 -n 750M -b 100M -l 1200 -P 1 -c 17.0.0.1 -V"
}

case_1448BPL_750M_100Mbit_tcp() {
    run_test "iperf3 -n 750M -b 100M -l 1448 -P 1 -c 17.0.0.1 -V"
}

# UDP
case_750M_25Mbit() {
    run_test "iperf3 -u -n 750M -b 25M -P 1 -c 17.0.0.1 -V"
}

case_750M_50Mbit() {
    run_test "iperf3 -u -n 750M -b 50M -P 1 -c 17.0.0.1 -V"
}

case_750M_100Mbit() {
    run_test "iperf3 -u -n 750M -b 100M -P 1 -c 17.0.0.1 -V"
}

case_750M_100Mbit() {
    run_test "iperf3 -u -n 750M -b 100M -P 1 -c 17.0.0.1 -V"
}

case_1500M_200Mbit() {
    run_test "iperf3 -u -n 1500M -b 200M -P 1 -c 17.0.0.1 -V"
}

case_3G_400Mbit() {
    run_test "iperf3 -u -n 3G -b 400M -P 1 -c 17.0.0.1 -V"
}

case_6G_800Mbit() {
    run_test "iperf3 -u -n 6G -b 800M -P 1 -c 17.0.0.1 -V"
}

case_7G_1000Mbit() {
    run_test "iperf3 -u -n 7G -b 1000M -P 1 -c 17.0.0.1 -V"
}

case_8G_1200Mbit() {
run_test "iperf3 -u -n 8G -b 1200M -P 1 -c 17.0.0.1 -V"
}

case_10G_1400Mbit() {
    run_test "iperf3 -u -n 10G -b 1400M -P 1 -c 17.0.0.1 -V"
}

case_12G_1600Mbit() {
    run_test "iperf3 -u -n 12G -b 800M -P 2 -c 17.0.0.1 -V"
}

case_14G_2000Mbit() {
    run_test "iperf3 -u -n 14G -b 1000M -P 2 -c 17.0.0.1 -V"
}

case_16G_2400Mbit() {
    run_test "iperf3 -u -n 16G -b 1200M -P 2 -c 17.0.0.1 -V"
}

# TCP 
case_3G_400Mbit_tcp() {
    run_test "iperf3 -n 3G -b 400M -P 1 -c 17.0.0.1 -V"
}

case_6G_800Mbit_tcp() {
    run_test "iperf3 -n 6G -b 800M -P 1 -c 17.0.0.1 -V"
}

case_7G_1000Mbit_tcp() {
    run_test "iperf3 -n 7G -b 1000M -P 1 -c 17.0.0.1 -V"
}

case_8G_1200Mbit_tcp() {
run_test "iperf3 -n 8G -b 1200M -P 1 -c 17.0.0.1 -V"
}

case_10G_1400Mbit_tcp() {
    run_test "iperf3 -n 10G -b 1400M -P 1 -c 17.0.0.1 -V"
}

case_12G_1600Mbit_tcp() {
    run_test "iperf3 -n 12G -b 800M -P 2 -c 17.0.0.1 -V"
}

case_14G_2000Mbit_tcp() {
    run_test "iperf3 -n 14G -b 1000M -P 2 -c 17.0.0.1 -V"
}

case_16G_2400Mbit_tcp() {
    run_test "iperf3 -n 16G -b 1200M -P 2 -c 17.0.0.1 -V"
}

case_18G_3000Mbit_tcp() {
    run_test "iperf3 -n 18G -b 750M -P 4 -c 17.0.0.1 -V"
}

case_20G_3600Mbit_tcp() {
    run_test "iperf3 -n 20G -b 900M -P 4 -c 17.0.0.1 -V"
}

start_iperf3_server() {
    ssh -t -t $REMOTE_USER@$REMOTE_SERVER "iperf3 -s" &> /dev/null &
    sleep 2
}

stop_iperf3_server() {
    ssh $REMOTE_USER@$REMOTE_SERVER "ps aux | grep iperf3 | grep -v grep | awk '{print $2}' | xargs kill -9" &> /dev/null
    sleep 2
}

final_case() {
    run_test "iperf3 -u -n 30G -b 800M -P 6 -c 17.0.0.1 -V"
}

final_case_tcp() {
    run_test "iperf3 -n 10G -b 800M -P 6 -c 17.0.0.1 -V"
}

# advanced

#start_iperf3_server
#run_nat_conf "nat.conf"
#case_750M_100Mbit
#close_nat
#stop_iperf3_server


#start_iperf3_server
#run_nat_conf "io_32G_4800Mbit.conf"
#final_case
#close_nat
#stop_iperf3_server

start_iperf3_server
run_nat_conf "liburing1_32G_4800Mbit.conf"
final_case
close_nat
stop_iperf3_server

start_iperf3_server
run_nat_conf "liburing2_32G_4800Mbit.conf"
final_case
close_nat
stop_iperf3_server

start_iperf3_server
run_nat_conf "liburing4_32G_4800Mbit.conf"
final_case
close_nat
stop_iperf3_server

start_iperf3_server
run_nat_conf "liburing8_32G_4800Mbit.conf"
final_case
close_nat
stop_iperf3_server

start_iperf3_server
run_nat_conf "liburing16_32G_4800Mbit.conf"
final_case
close_nat
stop_iperf3_server

start_iperf3_server
run_nat_conf "liburing32_32G_4800Mbit.conf"
final_case
close_nat
stop_iperf3_server

start_iperf3_server
run_nat_conf "liburing64_32G_4800Mbit.conf"
final_case
close_nat
stop_iperf3_server

start_iperf3_server
run_nat_conf "liburing128_32G_4800Mbit.conf"
final_case
close_nat
stop_iperf3_server

start_iperf3_server
run_nat_conf "liburing256_32G_4800Mbit.conf"
final_case
close_nat
stop_iperf3_server

start_iperf3_server
run_nat_conf "liburing512_32G_4800Mbit.conf"
final_case
close_nat
stop_iperf3_server

start_iperf3_server
run_nat_conf "liburing1024_32G_4800Mbit.conf"
final_case
close_nat
stop_iperf3_server

start_iperf3_server
run_nat_conf "liburing2048_32G_4800Mbit.conf"
final_case
close_nat
stop_iperf3_server

# tcp

#start_iperf3_server
#run_nat_conf_tcp "nat.conf"
#case_750M_100Mbit
#close_nat
#stop_iperf3_server
#
#
#start_iperf3_server
#run_nat_conf_tcp "io_32G_4800Mbit.conf"
#final_case_tcp
#close_nat
#stop_iperf3_server
#
#start_iperf3_server
#run_nat_conf_tcp "liburing1_32G_4800Mbit.conf"
#final_case_tcp
#close_nat
#stop_iperf3_server
#
#start_iperf3_server
#run_nat_conf_tcp "liburing2_32G_4800Mbit.conf"
#final_case_tcp
#close_nat
#stop_iperf3_server
#
#start_iperf3_server
#run_nat_conf_tcp "liburing4_32G_4800Mbit.conf"
#final_case_tcp
#close_nat
#stop_iperf3_server
#
#start_iperf3_server
#run_nat_conf_tcp "liburing8_32G_4800Mbit.conf"
#final_case_tcp
#close_nat
#stop_iperf3_server
#
#start_iperf3_server
#run_nat_conf_tcp "liburing16_32G_4800Mbit.conf"
#final_case_tcp
#close_nat
#stop_iperf3_server
#
#start_iperf3_server
#run_nat_conf_tcp "liburing32_32G_4800Mbit.conf"
#final_case_tcp
#close_nat
#stop_iperf3_server
#
#start_iperf3_server
#run_nat_conf_tcp "liburing64_32G_4800Mbit.conf"
#final_case_tcp
#close_nat
#stop_iperf3_server
#
#start_iperf3_server
#run_nat_conf_tcp "liburing128_32G_4800Mbit.conf"
#final_case_tcp
#close_nat
#stop_iperf3_server
#
#start_iperf3_server
#run_nat_conf_tcp "liburing256_32G_4800Mbit.conf"
#final_case_tcp
#close_nat
#stop_iperf3_server
#
#start_iperf3_server
#run_nat_conf_tcp "liburing512_32G_4800Mbit.conf"
#final_case_tcp
#close_nat
#stop_iperf3_server
#
#start_iperf3_server
#run_nat_conf_tcp "liburing1024_32G_4800Mbit.conf"
#final_case_tcp
#close_nat
#stop_iperf3_server
#
#start_iperf3_server
#run_nat_conf_tcp "liburing2048_32G_4800Mbit.conf"
#final_case_tcp
#close_nat
#stop_iperf3_server
















# default settings
#change_config "txqlen" "1000"
#change_config "mode" "0"
#change_config "qdepth" "1"
#change_config "cpu_affinity" "0"
#
## io
#start_iperf3_server
#change_config "logfile" "/home/w123694/build/iouring/logs/io_750_100Mbit.log"
#change_config "statsname" "io_750_100Mbit"
#case_750M_100Mbit
#
#stop_iperf3_server
#
##liburing
#change_config "mode" "2"
#
## qdepth 1
#change_config "qdepth" "1"
#
#start_iperf3_server
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_750_100Mbit.log"
#change_config "statsname" "liburing1_750_100Mbit"
#case_750M_100Mbit
#
#stop_iperf3_server
#
## qdepth 4
#change_config "qdepth" "4"
#
#start_iperf3_server
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing4_750_100Mbit.log"
#change_config "statsname" "liburing4_750_100Mbit"
#case_750M_100Mbit
#
#stop_iperf3_server
#
## qdepth 8
#change_config "qdepth" "8"
#
#start_iperf3_server
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing8_750_100Mbit.log"
#change_config "statsname" "liburing8_750_100Mbit"
#case_750M_100Mbit
#
#stop_iperf3_server
#
## qdepth 32
#change_config "qdepth" "32"
#
#start_iperf3_server
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing32_750_100Mbit.log"
#change_config "statsname" "liburing32_750_100Mbit"
#case_750M_100Mbit
#
#stop_iperf3_server
#
## qdepth 64
#change_config "qdepth" "64"
#
#start_iperf3_server
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing64_750_100Mbit.log"
#change_config "statsname" "liburing64_750_100Mbit"
#case_750M_100Mbit
#
#stop_iperf3_server
#
## qdepth 256
#change_config "qdepth" "256"
#
#start_iperf3_server
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing256_750_100Mbit.log"
#change_config "statsname" "liburing256_750_100Mbit"
#case_750M_100Mbit
#
#stop_iperf3_server
#
##qdepth 512
#change_config "qdepth" "512"
#
#start_iperf3_server
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing512_750_100Mbit.log"
#change_config "statsname" "liburing512_750_100Mbit"
#case_750M_100Mbit
#
#stop_iperf3_server
#
##qdepth 1024
#change_config "qdepth" "1024"
#
#start_iperf3_server
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1024_750_100Mbit.log"
#change_config "statsname" "liburing1024_750_100Mbit"
#case_750M_100Mbit
#
#stop_iperf3_server

# io
#start_iperf3_server
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_750M_25Mbit.log"
#change_config "statsname" "io_750M_25Mbit"
#case_750M_25Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_3G_400Mbit_tcp.log"
#change_config "statsname" "io_3G_400Mbit_tcp"
#case_3G_400Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_6G_800Mbit_tcp.log"
#change_config "statsname" "io_6G_800Mbit_tcp"
#case_6G_800Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_7G_1000Mbit_tcp.log"
#change_config "statsname" "io_7G_1000Mbit_tcp"
#case_7G_1000Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_8G_1200Mbit_tcp.log"
#change_config "statsname" "io_8G_1200Mbit_tcp"
#case_8G_1200Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_10G_1400Mbit_tcp.log"
#change_config "statsname" "io_10G_1400Mbit_tcp"
#case_10G_1400Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_12G_1600Mbit_tcp.log"
#change_config "statsname" "io_12G_1600Mbit_tcp"
#case_12G_1600Mbit_tcp
#
#stop_iperf3_server
#
##liburing
#change_config "mode" "2"
#
## qdepth 1
#change_config "qdepth" "1"
#
#start_iperf3_server
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_750M_25Mbit.log"
#change_config "statsname" "liburing1_750M_25Mbit"
#case_750M_25Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_3G_400Mbit_tcp.log"
#change_config "statsname" "liburing1_3G_400Mbit_tcp"
#case_3G_400Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_6G_800Mbit_tcp.log"
#change_config "statsname" "liburing1_6G_800Mbit_tcp"
#case_6G_800Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_7G_1000Mbit_tcp.log"
#change_config "statsname" "liburing1_7G_1000Mbit_tcp"
#case_7G_1000Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_8G_1200Mbit_tcp.log"
#change_config "statsname" "liburing1_8G_1200Mbit_tcp"
#case_8G_1200Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_10G_1400Mbit_tcp.log"
#change_config "statsname" "liburing1_10G_1400Mbit_tcp"
#case_10G_1400Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_12G_1600Mbit_tcp.log"
#change_config "statsname" "liburing1_12G_1600Mbit_tcp"
#case_12G_1600Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_14G_2000Mbit_tcp.log"
#change_config "statsname" "liburing1_14G_2000Mbit_tcp"
#case_14G_2000Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_16G_2400Mbit_tcp.log"
#change_config "statsname" "liburing1_16G_2400Mbit_tcp"
#case_16G_2400Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_18G_3000Mbit_tcp.log"
#change_config "statsname" "liburing1_18G_3000Mbit_tcp"
#case_18G_3000Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_20G_3600Mbit_tcp.log"
#change_config "statsname" "liburing1_20G_3600Mbit_tcp"
#case_20G_3600Mbit_tcp
#
#stop_iperf3_server
#
## qdepth 4
#change_config "qdepth" "4"
#
#start_iperf3_server
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing4_750M_25Mbit.log"
#change_config "statsname" "liburing4_750M_25Mbit"
#case_750M_25Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing4_3G_400Mbit_tcp.log"
#change_config "statsname" "liburing4_3G_400Mbit_tcp"
#case_3G_400Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing4_6G_800Mbit_tcp.log"
#change_config "statsname" "liburing4_6G_800Mbit_tcp"
#case_6G_800Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing4_7G_1000Mbit_tcp.log"
#change_config "statsname" "liburing4_7G_1000Mbit_tcp"
#case_7G_1000Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing4_8G_1200Mbit_tcp.log"
#change_config "statsname" "liburing4_8G_1200Mbit_tcp"
#case_8G_1200Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing4_10G_1400Mbit_tcp.log"
#change_config "statsname" "liburing4_10G_1400Mbit_tcp"
#case_10G_1400Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing4_12G_1600Mbit_tcp.log"
#change_config "statsname" "liburing4_12G_1600Mbit_tcp"
#case_12G_1600Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing4_14G_2000Mbit_tcp.log"
#change_config "statsname" "liburing4_14G_2000Mbit_tcp"
#case_14G_2000Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing4_16G_2400Mbit_tcp.log"
#change_config "statsname" "liburing4_16G_2400Mbit_tcp"
#case_16G_2400Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing4_18G_3000Mbit_tcp.log"
#change_config "statsname" "liburing4_18G_3000Mbit_tcp"
#case_18G_3000Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing4_20G_3600Mbit_tcp.log"
#change_config "statsname" "liburing4_20G_3600Mbit_tcp"
#case_20G_3600Mbit_tcp
#
#stop_iperf3_server
#
## qdepth 8
#change_config "qdepth" "8"
#
#start_iperf3_server
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing8_750M_25Mbit.log"
#change_config "statsname" "liburing8_750M_25Mbit"
#case_750M_25Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing8_3G_400Mbit_tcp.log"
#change_config "statsname" "liburing8_3G_400Mbit_tcp"
#case_3G_400Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing8_6G_800Mbit_tcp.log"
#change_config "statsname" "liburing8_6G_800Mbit_tcp"
#case_6G_800Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing8_7G_1000Mbit_tcp.log"
#change_config "statsname" "liburing8_7G_1000Mbit_tcp"
#case_7G_1000Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing8_8G_1200Mbit_tcp.log"
#change_config "statsname" "liburing8_8G_1200Mbit_tcp"
#case_8G_1200Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing8_10G_1400Mbit_tcp.log"
#change_config "statsname" "liburing8_10G_1400Mbit_tcp"
#case_10G_1400Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing8_12G_1600Mbit_tcp.log"
#change_config "statsname" "liburing8_12G_1600Mbit_tcp"
#case_12G_1600Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing8_14G_2000Mbit_tcp.log"   
#change_config "statsname" "liburing8_14G_2000Mbit_tcp"
#case_14G_2000Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing8_16G_2400Mbit_tcp.log"
#change_config "statsname" "liburing8_16G_2400Mbit_tcp"
#case_16G_2400Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing8_18G_3000Mbit_tcp.log"
#change_config "statsname" "liburing8_18G_3000Mbit_tcp"
#case_18G_3000Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing8_20G_3600Mbit_tcp.log"
#change_config "statsname" "liburing8_20G_3600Mbit_tcp"
#case_20G_3600Mbit_tcp
#
#stop_iperf3_server
#
## qdepth 32
#change_config "qdepth" "32"
#
#start_iperf3_server
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing32_750M_25Mbit.log"
#change_config "statsname" "liburing32_750M_25Mbit"
#case_750M_25Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing32_3G_400Mbit_tcp.log"
#change_config "statsname" "liburing32_3G_400Mbit_tcp"
#case_3G_400Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing32_6G_800Mbit_tcp.log"
#change_config "statsname" "liburing32_6G_800Mbit_tcp"
#case_6G_800Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing32_7G_1000Mbit_tcp.log"
#change_config "statsname" "liburing32_7G_1000Mbit_tcp"
#case_7G_1000Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing32_8G_1200Mbit_tcp.log"
#change_config "statsname" "liburing32_8G_1200Mbit_tcp"
#case_8G_1200Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing32_10G_1400Mbit_tcp.log"
#change_config "statsname" "liburing32_10G_1400Mbit_tcp"
#case_10G_1400Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing32_12G_1600Mbit_tcp.log"
#change_config "statsname" "liburing32_12G_1600Mbit_tcp"
#case_12G_1600Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing32_14G_2000Mbit_tcp.log"   
#change_config "statsname" "liburing32_14G_2000Mbit_tcp"
#case_14G_2000Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing32_16G_2400Mbit_tcp.log"
#change_config "statsname" "liburing32_16G_2400Mbit_tcp"
#case_16G_2400Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing32_18G_3000Mbit_tcp.log"
#change_config "statsname" "liburing32_18G_3000Mbit_tcp"
#case_18G_3000Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing32_20G_3600Mbit_tcp.log"
#change_config "statsname" "liburing32_20G_3600Mbit_tcp"
#case_20G_3600Mbit_tcp
#
#stop_iperf3_server
#
## qdepth 64
#change_config "qdepth" "64"
#
#start_iperf3_server
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing64_750M_25Mbit.log"
#change_config "statsname" "liburing64_750M_25Mbit"
#case_750M_25Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing64_3G_400Mbit_tcp.log"
#change_config "statsname" "liburing64_3G_400Mbit_tcp"
#case_3G_400Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing64_6G_800Mbit_tcp.log"
#change_config "statsname" "liburing64_6G_800Mbit_tcp"
#case_6G_800Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing64_7G_1000Mbit_tcp.log"
#change_config "statsname" "liburing64_7G_1000Mbit_tcp"
#case_7G_1000Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing64_8G_1200Mbit_tcp.log"
#change_config "statsname" "liburing64_8G_1200Mbit_tcp"
#case_8G_1200Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing64_10G_1400Mbit_tcp.log"
#change_config "statsname" "liburing64_10G_1400Mbit_tcp"
#case_10G_1400Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing64_12G_1600Mbit_tcp.log"
#change_config "statsname" "liburing64_12G_1600Mbit_tcp"
#case_12G_1600Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing64_14G_2000Mbit_tcp.log"   
#change_config "statsname" "liburing64_14G_2000Mbit_tcp"
#case_14G_2000Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing64_16G_2400Mbit_tcp.log"
#change_config "statsname" "liburing64_16G_2400Mbit_tcp"
#case_16G_2400Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing64_18G_3000Mbit_tcp.log"
#change_config "statsname" "liburing64_18G_3000Mbit_tcp"
#case_18G_3000Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing64_20G_3600Mbit_tcp.log"
#change_config "statsname" "liburing64_20G_3600Mbit_tcp"
#case_20G_3600Mbit_tcp
#
#stop_iperf3_server
#
## qdepth 256
#change_config "qdepth" "256"
#
#start_iperf3_server
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing256_750M_25Mbit.log"
#change_config "statsname" "liburing256_750M_25Mbit"
#case_750M_25Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing256_3G_400Mbit_tcp.log"
#change_config "statsname" "liburing256_3G_400Mbit_tcp"
#case_3G_400Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing256_6G_800Mbit_tcp.log"
#change_config "statsname" "liburing256_6G_800Mbit_tcp"
#case_6G_800Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing256_7G_1000Mbit_tcp.log"
#change_config "statsname" "liburing256_7G_1000Mbit_tcp"
#case_7G_1000Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing256_8G_1200Mbit_tcp.log"
#change_config "statsname" "liburing256_8G_1200Mbit_tcp"
#case_8G_1200Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing256_10G_1400Mbit_tcp.log"
#change_config "statsname" "liburing256_10G_1400Mbit_tcp"
#case_10G_1400Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing256_12G_1600Mbit_tcp.log"
#change_config "statsname" "liburing256_12G_1600Mbit_tcp"
#case_12G_1600Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing256_14G_2000Mbit_tcp.log"
#change_config "statsname" "liburing256_14G_2000Mbit_tcp"
#case_14G_2000Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing256_16G_2400Mbit_tcp.log"
#change_config "statsname" "liburing256_16G_2400Mbit_tcp"
#case_16G_2400Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing256_18G_3000Mbit_tcp.log"
#change_config "statsname" "liburing256_18G_3000Mbit_tcp"
#case_18G_3000Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing256_20G_3600Mbit_tcp.log"
#change_config "statsname" "liburing256_20G_3600Mbit_tcp"
#case_20G_3600Mbit_tcp
#
#stop_iperf3_server
#
## qdepth 512
#change_config "qdepth" "512"
#
#start_iperf3_server
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing512_750M_25Mbit.log"
#change_config "statsname" "liburing512_750M_25Mbit"
#case_750M_25Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing512_3G_400Mbit_tcp.log"
#change_config "statsname" "liburing512_3G_400Mbit_tcp"
#case_3G_400Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing512_6G_800Mbit_tcp.log"
#change_config "statsname" "liburing512_6G_800Mbit_tcp"
#case_6G_800Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing512_7G_1000Mbit_tcp.log"
#change_config "statsname" "liburing512_7G_1000Mbit_tcp"
#case_7G_1000Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing512_8G_1200Mbit_tcp.log"
#change_config "statsname" "liburing512_8G_1200Mbit_tcp"
#case_8G_1200Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing512_10G_1400Mbit_tcp.log"
#change_config "statsname" "liburing512_10G_1400Mbit_tcp"
#case_10G_1400Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing512_12G_1600Mbit_tcp.log"
#change_config "statsname" "liburing512_12G_1600Mbit_tcp"
#case_12G_1600Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing512_14G_2000Mbit_tcp.log"
#change_config "statsname" "liburing512_14G_2000Mbit_tcp"
#case_14G_2000Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing512_16G_2400Mbit_tcp.log"
#change_config "statsname" "liburing512_16G_2400Mbit_tcp"
#case_16G_2400Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing512_18G_3000Mbit_tcp.log"
#change_config "statsname" "liburing512_18G_3000Mbit_tcp"
#case_18G_3000Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing512_20G_3600Mbit_tcp.log"
#change_config "statsname" "liburing512_20G_3600Mbit_tcp"
#case_20G_3600Mbit_tcp
#
#stop_iperf3_server
#
## qdepth 1024
#change_config "qdepth" "1024"
#
#start_iperf3_server
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1024_750M_25Mbit.log"
#change_config "statsname" "liburing1024_750M_25Mbit"
#case_750M_25Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1024_3G_400Mbit_tcp.log"
#change_config "statsname" "liburing1024_3G_400Mbit_tcp"
#case_3G_400Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1024_6G_800Mbit_tcp.log"
#change_config "statsname" "liburing1024_6G_800Mbit_tcp"
#case_6G_800Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1024_7G_1000Mbit_tcp.log"
#change_config "statsname" "liburing1024_7G_1000Mbit_tcp"
#case_7G_1000Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1024_8G_1200Mbit_tcp.log"
#change_config "statsname" "liburing1024_8G_1200Mbit_tcp"
#case_8G_1200Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1024_10G_1400Mbit_tcp.log"
#change_config "statsname" "liburing1024_10G_1400Mbit_tcp"
#case_10G_1400Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1024_12G_1600Mbit_tcp.log"
#change_config "statsname" "liburing1024_12G_1600Mbit_tcp"
#case_12G_1600Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1024_14G_2000Mbit_tcp.log"
#change_config "statsname" "liburing1024_14G_2000Mbit_tcp"
#case_14G_2000Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1024_16G_2400Mbit_tcp.log"
#change_config "statsname" "liburing1024_16G_2400Mbit_tcp"
#case_16G_2400Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1024_18G_3000Mbit_tcp.log"
#change_config "statsname" "liburing1024_18G_3000Mbit_tcp"
#case_18G_3000Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1024_20G_3600Mbit_tcp.log"
#change_config "statsname" "liburing1024_20G_3600Mbit_tcp"
#case_20G_3600Mbit_tcp
#
#stop_iperf3_server

# default settings
#change_config "txqlen" "1000"
#change_config "mode" "0"
#change_config "qdepth" "1"
#change_config "cpu_affinity" "0"
#
#start_iperf3_server
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_100BPL_750M_100Mbit.log"
#change_config "statsname" "io_100BPL_750M_100Mbit"
#case_100BPL_750M_100Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_200BPL_750M_100Mbit.log"
#change_config "statsname" "io_200BPL_750M_100Mbit"
#case_200BPL_750M_100Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_400BPL_750M_100Mbit.log"
#change_config "statsname" "io_400BPL_750M_100Mbit"
#case_400BPL_750M_100Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_800BPL_750M_100Mbit.log"
#change_config "statsname" "io_800BPL_750M_100Mbit"
#case_800BPL_750M_100Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_1200BPL_750M_100Mbit.log"
#change_config "statsname" "io_1200BPL_750M_100Mbit"
#case_1200BPL_750M_100Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_1448BPL_750M_100Mbit.log"
#change_config "statsname" "io_1448BPL_750M_100Mbit"
#case_1448BPL_750M_100Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_100BPL_750M_100Mbit_tcp.log"
#change_config "statsname" "io_100BPL_750M_100Mbit_tcp"
#case_100BPL_750M_100Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_200BPL_750M_100Mbit_tcp.log"
#change_config "statsname" "io_200BPL_750M_100Mbit_tcp"
#case_200BPL_750M_100Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_400BPL_750M_100Mbit_tcp.log"
#change_config "statsname" "io_400BPL_750M_100Mbit_tcp"
#case_400BPL_750M_100Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_800BPL_750M_100Mbit_tcp.log"
#change_config "statsname" "io_800BPL_750M_100Mbit_tcp"
#case_800BPL_750M_100Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_1200BPL_750M_100Mbit_tcp.log"
#change_config "statsname" "io_1200BPL_750M_100Mbit_tcp"
#case_1200BPL_750M_100Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_1448BPL_750M_100Mbit_tcp.log"
#change_config "statsname" "io_1448BPL_750M_100Mbit_tcp"
#case_1448BPL_750M_100Mbit_tcp
#
#stop_iperf3_server
#
##liburing
#change_config "mode" "2"
#
## qdepth 1
#change_config "qdepth" "1"
#
#start_iperf3_server
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_100BPL_750M_100Mbit.log"
#change_config "statsname" "liburing1_100BPL_750M_100Mbit"
#case_100BPL_750M_100Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_200BPL_750M_100Mbit.log"
#change_config "statsname" "liburing1_200BPL_750M_100Mbit"
#case_200BPL_750M_100Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_400BPL_750M_100Mbit.log"
#change_config "statsname" "liburing1_400BPL_750M_100Mbit"
#case_400BPL_750M_100Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_800BPL_750M_100Mbit.log"
#change_config "statsname" "liburing1_800BPL_750M_100Mbit"
#case_800BPL_750M_100Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_1200BPL_750M_100Mbit.log"
#change_config "statsname" "liburing1_1200BPL_750M_100Mbit"
#case_1200BPL_750M_100Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_1448BPL_750M_100Mbit.log"
#change_config "statsname" "liburing1_1448BPL_750M_100Mbit"
#case_1448BPL_750M_100Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_100BPL_750M_100Mbit_tcp.log"
#change_config "statsname" "liburing1_100BPL_750M_100Mbit_tcp"
#case_100BPL_750M_100Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_200BPL_750M_100Mbit_tcp.log"
#change_config "statsname" "liburing1_200BPL_750M_100Mbit_tcp"
#case_200BPL_750M_100Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_400BPL_750M_100Mbit_tcp.log"
#change_config "statsname" "liburing1_400BPL_750M_100Mbit_tcp"
#case_400BPL_750M_100Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_800BPL_750M_100Mbit_tcp.log"
#change_config "statsname" "liburing1_800BPL_750M_100Mbit_tcp"
#case_800BPL_750M_100Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_1200BPL_750M_100Mbit_tcp.log"
#change_config "statsname" "liburing1_1200BPL_750M_100Mbit_tcp"
#case_1200BPL_750M_100Mbit_tcp
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_1448BPL_750M_100Mbit_tcp.log"
#change_config "statsname" "liburing1_1448BPL_750M_100Mbit_tcp"
#case_1448BPL_750M_100Mbit_tcp
#
#stop_iperf3_server

# io
#start_iperf3_server
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_750M_50Mbit.log"
#change_config "statsname" "io_750M_50Mbit"
#case_750M_50Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_1500M_200Mbit.log"
#change_config "statsname" "io_1500M_200Mbit"
#case_1500M_200Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_3G_400Mbit.log"
#change_config "statsname" "io_3G_400Mbit"
#case_3G_400Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_6G_800Mbit.log"
#change_config "statsname" "io_6G_800Mbit"
#case_6G_800Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_7G_1000Mbit.log"
#change_config "statsname" "io_7G_1000Mbit"
#case_7G_1000Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_8G_1200Mbit.log"
#change_config "statsname" "io_8G_1200Mbit"
#case_8G_1200Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_10G_1400Mbit.log"
#change_config "statsname" "io_10G_1400Mbit"
#case_10G_1400Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_12G_1600Mbit.log"
#change_config "statsname" "io_12G_1600Mbit"
#case_12G_1600Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_14G_2000Mbit.log"
#change_config "statsname" "io_14G_2000Mbit"
#case_14G_2000Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/io_16G_2400Mbit.log"
#change_config "statsname" "io_16G_2400Mbit"
#case_16G_2400Mbit
#
#stop_iperf3_server
#
##liburing
#change_config "mode" "2"
#
## qdepth 1
#change_config "qdepth" "1"
#
#start_iperf3_server
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_750M_50Mbit.log"
#change_config "statsname" "liburing1_750M_50Mbit"
#case_750M_50Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_1500M_200Mbit.log"
#change_config "statsname" "liburing1_1500M_200Mbit"
#case_1500M_200Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_3G_400Mbit.log"
#change_config "statsname" "liburing1_3G_400Mbit"
#case_3G_400Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_6G_800Mbit.log"
#change_config "statsname" "liburing1_6G_800Mbit"
#case_6G_800Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_7G_1000Mbit.log"
#change_config "statsname" "liburing1_7G_1000Mbit"
#case_7G_1000Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_8G_1200Mbit.log"
#change_config "statsname" "liburing1_8G_1200Mbit"
#case_8G_1200Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_10G_1400Mbit.log"
#change_config "statsname" "liburing1_10G_1400Mbit"
#case_10G_1400Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_12G_1600Mbit.log"
#change_config "statsname" "liburing1_12G_1600Mbit"
#case_12G_1600Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_14G_2000Mbit.log"
#change_config "statsname" "liburing1_14G_2000Mbit"
#case_14G_2000Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1_16G_2400Mbit.log"
#change_config "statsname" "liburing1_16G_2400Mbit"
#case_16G_2400Mbit
#
#stop_iperf3_server
#
## qdepth 4
#change_config "qdepth" "4"
#
#start_iperf3_server
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing4_750M_50Mbit.log"
#change_config "statsname" "liburing4_750M_50Mbit"
#case_750M_50Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing4_1500M_200Mbit.log"
#change_config "statsname" "liburing4_1500M_200Mbit"
#case_1500M_200Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing4_3G_400Mbit.log"
#change_config "statsname" "liburing4_3G_400Mbit"
#case_3G_400Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing4_6G_800Mbit.log"
#change_config "statsname" "liburing4_6G_800Mbit"
#case_6G_800Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing4_7G_1000Mbit.log"
#change_config "statsname" "liburing4_7G_1000Mbit"
#case_7G_1000Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing4_8G_1200Mbit.log"
#change_config "statsname" "liburing4_8G_1200Mbit"
#case_8G_1200Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing4_10G_1400Mbit.log"
#change_config "statsname" "liburing4_10G_1400Mbit"
#case_10G_1400Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing4_12G_1600Mbit.log"
#change_config "statsname" "liburing4_12G_1600Mbit"
#case_12G_1600Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing4_14G_2000Mbit.log"
#change_config "statsname" "liburing4_14G_2000Mbit"
#case_14G_2000Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing4_16G_2400Mbit.log"
#change_config "statsname" "liburing4_16G_2400Mbit"
#case_16G_2400Mbit
#
#stop_iperf3_server
#
## qdepth 8
#change_config "qdepth" "8"
#
#start_iperf3_server
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing8_750M_50Mbit.log"
#change_config "statsname" "liburing8_750M_50Mbit"
#case_750M_50Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing8_1500M_200Mbit.log"
#change_config "statsname" "liburing8_1500M_200Mbit"
#case_1500M_200Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing8_3G_400Mbit.log"
#change_config "statsname" "liburing8_3G_400Mbit"
#case_3G_400Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing8_6G_800Mbit.log"
#change_config "statsname" "liburing8_6G_800Mbit"
#case_6G_800Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing8_7G_1000Mbit.log"
#change_config "statsname" "liburing8_7G_1000Mbit"
#case_7G_1000Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing8_8G_1200Mbit.log"
#change_config "statsname" "liburing8_8G_1200Mbit"
#case_8G_1200Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing8_10G_1400Mbit.log"
#change_config "statsname" "liburing8_10G_1400Mbit"
#case_10G_1400Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing8_12G_1600Mbit.log"
#change_config "statsname" "liburing8_12G_1600Mbit"
#case_12G_1600Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing8_14G_2000Mbit.log"
#change_config "statsname" "liburing8_14G_2000Mbit"
#case_14G_2000Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing8_16G_2400Mbit.log"
#change_config "statsname" "liburing8_16G_2400Mbit"
#case_16G_2400Mbit
#
#stop_iperf3_server
#
## qdepth 32
#change_config "qdepth" "32"
#
#start_iperf3_server
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing32_750M_50Mbit.log"
#change_config "statsname" "liburing32_750M_50Mbit"
#case_750M_50Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing32_1500M_200Mbit.log"
#change_config "statsname" "liburing32_1500M_200Mbit"
#case_1500M_200Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing32_3G_400Mbit.log"
#change_config "statsname" "liburing32_3G_400Mbit"
#case_3G_400Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing32_6G_800Mbit.log"
#change_config "statsname" "liburing32_6G_800Mbit"
#case_6G_800Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing32_7G_1000Mbit.log"
#change_config "statsname" "liburing32_7G_1000Mbit"
#case_7G_1000Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing32_8G_1200Mbit.log"
#change_config "statsname" "liburing32_8G_1200Mbit"
#case_8G_1200Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing32_10G_1400Mbit.log"
#change_config "statsname" "liburing32_10G_1400Mbit"
#case_10G_1400Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing32_12G_1600Mbit.log"
#change_config "statsname" "liburing32_12G_1600Mbit"
#case_12G_1600Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing32_14G_2000Mbit.log"
#change_config "statsname" "liburing32_14G_2000Mbit"
#case_14G_2000Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing32_16G_2400Mbit.log"
#change_config "statsname" "liburing32_16G_2400Mbit"
#case_16G_2400Mbit
#
#stop_iperf3_server
#
## qdepth 64
#change_config "qdepth" "64"
#
#start_iperf3_server
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing64_750M_50Mbit.log"
#change_config "statsname" "liburing64_750M_50Mbit"
#case_750M_50Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing64_1500M_200Mbit.log"
#change_config "statsname" "liburing64_1500M_200Mbit"
#case_1500M_200Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing64_3G_400Mbit.log"
#change_config "statsname" "liburing64_3G_400Mbit"
#case_3G_400Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing64_6G_800Mbit.log"
#change_config "statsname" "liburing64_6G_800Mbit"
#case_6G_800Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing64_7G_1000Mbit.log"
#change_config "statsname" "liburing64_7G_1000Mbit"
#case_7G_1000Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing64_8G_1200Mbit.log"
#change_config "statsname" "liburing64_8G_1200Mbit"
#case_8G_1200Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing64_10G_1400Mbit.log"
#change_config "statsname" "liburing64_10G_1400Mbit"
#case_10G_1400Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing64_12G_1600Mbit.log"
#change_config "statsname" "liburing64_12G_1600Mbit"
#case_12G_1600Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing64_14G_2000Mbit.log"
#change_config "statsname" "liburing64_14G_2000Mbit"
#case_14G_2000Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing64_16G_2400Mbit.log"
#change_config "statsname" "liburing64_16G_2400Mbit"
#case_16G_2400Mbit
#
#stop_iperf3_server
#
## qdepth 256
#change_config "qdepth" "256"
#
#start_iperf3_server
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing256_750M_50Mbit.log"
#change_config "statsname" "liburing256_750M_50Mbit"
#case_750M_50Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing256_1500M_200Mbit.log"
#change_config "statsname" "liburing256_1500M_200Mbit"
#case_1500M_200Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing256_3G_400Mbit.log"
#change_config "statsname" "liburing256_3G_400Mbit"
#case_3G_400Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing256_6G_800Mbit.log"
#change_config "statsname" "liburing256_6G_800Mbit"
#case_6G_800Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing256_7G_1000Mbit.log"
#change_config "statsname" "liburing256_7G_1000Mbit"
#case_7G_1000Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing256_8G_1200Mbit.log"
#change_config "statsname" "liburing256_8G_1200Mbit"
#case_8G_1200Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing256_10G_1400Mbit.log"
#change_config "statsname" "liburing256_10G_1400Mbit"
#case_10G_1400Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing256_12G_1600Mbit.log"
#change_config "statsname" "liburing256_12G_1600Mbit"
#case_12G_1600Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing256_14G_2000Mbit.log"
#change_config "statsname" "liburing256_14G_2000Mbit"
#case_14G_2000Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing256_16G_2400Mbit.log"
#change_config "statsname" "liburing256_16G_2400Mbit"
#case_16G_2400Mbit
#
#stop_iperf3_server
#
## qdepth 512
#change_config "qdepth" "512"
#
#start_iperf3_server
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing512_750M_50Mbit.log"
#change_config "statsname" "liburing512_750M_50Mbit"
#case_750M_50Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing512_1500M_200Mbit.log"
#change_config "statsname" "liburing512_1500M_200Mbit"
#case_1500M_200Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing512_3G_400Mbit.log"
#change_config "statsname" "liburing512_3G_400Mbit"
#case_3G_400Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing512_6G_800Mbit.log"
#change_config "statsname" "liburing512_6G_800Mbit"
#case_6G_800Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing512_7G_1000Mbit.log"
#change_config "statsname" "liburing512_7G_1000Mbit"
#case_7G_1000Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing512_8G_1200Mbit.log"
#change_config "statsname" "liburing512_8G_1200Mbit"
#case_8G_1200Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing512_10G_1400Mbit.log"
#change_config "statsname" "liburing512_10G_1400Mbit"
#case_10G_1400Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing512_12G_1600Mbit.log"
#change_config "statsname" "liburing512_12G_1600Mbit"
#case_12G_1600Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing512_14G_2000Mbit.log"
#change_config "statsname" "liburing512_14G_2000Mbit"
#case_14G_2000Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing512_16G_2400Mbit.log"
#change_config "statsname" "liburing512_16G_2400Mbit"
#case_16G_2400Mbit
#
#stop_iperf3_server
#
## qdepth 1024
#change_config "qdepth" "1024"
#
#start_iperf3_server
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1024_750M_50Mbit.log"
#change_config "statsname" "liburing1024_750M_50Mbit"
#case_750M_50Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1024_1500M_200Mbit.log"
#change_config "statsname" "liburing1024_1500M_200Mbit"
#case_1500M_200Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1024_3G_400Mbit.log"
#change_config "statsname" "liburing1024_3G_400Mbit"
#case_3G_400Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1024_6G_800Mbit.log"
#change_config "statsname" "liburing1024_6G_800Mbit"
#case_6G_800Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1024_7G_1000Mbit.log"
#change_config "statsname" "liburing1024_7G_1000Mbit"
#case_7G_1000Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1024_8G_1200Mbit.log"
#change_config "statsname" "liburing1024_8G_1200Mbit"
#case_8G_1200Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1024_10G_1400Mbit.log"
#change_config "statsname" "liburing1024_10G_1400Mbit"
#case_10G_1400Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1024_12G_1600Mbit.log"
#change_config "statsname" "liburing1024_12G_1600Mbit"
#case_12G_1600Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1024_14G_2000Mbit.log"
#change_config "statsname" "liburing1024_14G_2000Mbit"
#case_14G_2000Mbit
#
#change_config "logfile" "/home/w123694/build/iouring/logs/liburing1024_16G_2400Mbit.log"
#change_config "statsname" "liburing1024_16G_2400Mbit"
#case_16G_2400Mbit
#
#stop_iperf3_server






