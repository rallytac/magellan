#!/bin/bash

echo "--------------------------------------------------------------------"
echo "Magellan Radio Gateway REST Simulator"
echo "Copyright (c) 2020 Rally Tactical Systems, Inc."
echo "--------------------------------------------------------------------"

if [[ "${1}" == "" || "${2}" == "" ]]; then
    echo "usage: ./runsim.sh <name_of_root_dir> <port>"
    exit
fi

cd ${1}
php -S 0.0.0.0:${2} -c ./
