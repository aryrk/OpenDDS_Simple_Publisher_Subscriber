#!/bin/bash

# Run subscriber with DDS configuration
cd "$(dirname "$0")"
export DCPSConfigFile=dds_tcp_conf.ini
./bin/subscriber -DCPSConfigFile dds_tcp_conf.ini "$@"
