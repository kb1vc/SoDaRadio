#!/bin/bash

i=0

while true ; do
    fnbase="sweep_143r75_${i}"
    echo "Filename: $fnbase"
    exp/DDC_Test --lo_freq=143750000 --file="$fnbase"
    i=$[$i+1]
done
