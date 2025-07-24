#!/bin/bash

# Run publisher with DDS configuration
cd "$(dirname "$0")"
export DCPSConfigFile=dds_tcp_conf.ini
./bin/publisher -DCPSConfigFile dds_tcp_conf.ini "$@"
