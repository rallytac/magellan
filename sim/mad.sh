#!/bin/bash

#
#  Copyright (c) 2020 Rally Tactical Systems, Inc.
#  All rights reserved.
#

echo "---------------------------------------------------------------------------"
echo "Magellan REST Server Advertiser (mad) version 0.1 "
echo ""
echo "Copyright (c) 2020 Rally Tactical Systems, Inc."
echo "---------------------------------------------------------------------------"

function doSsdp()
{
    echo "Advertising using SSDP"

    while true
    do 
        nc -w 1 -4 -u 239.255.255.250 1900 < response.ssdp
        sleep 1
    done
}

function doMdns()
{
    echo "Advertising using MDNS"
    
    avahi-publish-service rock-and-roll-gw _magellan._tcp 8081 "id={d7107580-952d-4fd4-a4c2-a01f4067fd39}" "cv=234"
}

if [[ "${1}" == "-mdns" ]]; then
    doMdns
elif [[ "${1}" == "-ssdp" ]]; then
    doSsdp
fi