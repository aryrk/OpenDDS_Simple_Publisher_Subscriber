#!/bin/bash

# Run publisher without ownership QoS for better monitoring compatibility
cd "$(dirname "$0")"
export DCPSConfigFile=dds_tcp_conf.ini
./bin/publisher -DCPSConfigFile dds_tcp_conf.ini -no-ownership "$@"
