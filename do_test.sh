#!/bin/bash

i=0

for i in {0..3}
do
    fnbase="sweep_143r75_${i}"
    echo "Filename: $fnbase"

    exp/DDC_Test --lo_freq=143750000 --file="${fnbase}_wide_hi" -s -5000000 -e -400000 -i 10000 -w 128 -c 4

    # this set will show the first LO image as a diagonal line
    # use coarse steps
    exp/DDC_Test --lo_freq=143750000 --file="${fnbase}_close" -s -400000 -e 400000 -i 10000 -w 128 -c 4

    exp/DDC_Test --lo_freq=143750000 --file="${fnbase}_wide_lo" -s 400000 -e 5000000 -i 10000 -w 128 -c 4
    
    i=$[$i+1]
done

for i in {0..3} 
do
    fnbase="sweep_143r75_${i}"
    echo "Filename: $fnbase"

    exp/DDC_Test --lo_freq=143750000 --file="${fnbase}_wide_hi_fine" -s -1400000 -e -400000 -i 1000 -w 128 -c 4


    exp/DDC_Test --lo_freq=143750000 --file="${fnbase}_wide_lo_fine" -s 400000 -e 1400000 -i 1000 -w 128 -c 4
    
    i=$[$i+1]
done

