#!/bin/bash
zmap -i eth0 -w wlist_ar -o nservs_ar.conf -B 140K -p 53 --probe-module=udp -P 1 -N 4000
