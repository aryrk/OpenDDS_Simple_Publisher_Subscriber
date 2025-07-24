#!/bin/bash

# Run subscriber without ownership QoS for better monitoring compatibility  
cd "$(dirname "$0")"
export DCPSConfigFile=dds_tcp_conf.ini
./bin/subscriber -DCPSConfigFile dds_tcp_conf.ini -no-ownership "$@"
