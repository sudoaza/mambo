#!/bin/bash
zmap -i wlan0 -w wlist_ar -o nservs_ar.conf -B 64K -p 53 -P 1
