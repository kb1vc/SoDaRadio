#!/bin/bash

#!/bin/bash

i=0
LEADER_IGNORE=64
WINDOW_SIZE=256
basefreq=143r75

for fsuff in wide_hi wide_lo wide_hi_fine wide_lo_fine close
do
  for i in {0..3}
  do
    fnbase="sweep_${basefreq}_${i}"
    echo "Filename: $fnbase"
    fn=${fnbase}_${fsuff}

    fnERMSplotdat="sweep_${basefreq}_${fsuff}_${i}.erms"
    fnPOWplotdat="sweep_${basefreq}_${fsuff}_${i}.pow"
  
    exp/DDC_Analyze ${fn}.bin temp.adat

    grep '^ERMS' temp.adat > ${fnERMSplotdat}
    grep '^Low_Spur' temp.adat > ${fnPOWplotdat}_lo
    grep '^Hi_Spur' temp.adat > ${fnPOWplotdat}_hi

    lo_sp_freq=`head -1 ${fnPOWplotdat}_lo | sed -e 's/[^ ]* \(.*\) .* .*/\1/' | sed -e 's/[ ]*//g'`
    hi_sp_freq=`head -1 ${fnPOWplotdat}_hi | sed -e 's/[^ ]* \(.*\) .* .*/\1/' | sed -e 's/[ ]*//g'`
    
    gnuplot <<EOF
unset key
set title 'RMS Deviation by DDC Frequency for LO Setting of 143.75 MHz'
set ylabel 'ERMS (dB)'
set xlabel 'DDC Frequency (MHz)'
set terminal png size 1024, 768
set output '${fnERMSplotdat}.png'
plot '${fnERMSplotdat}' using (\$2*1e-6):3 with lines 
EOF

    gnuplot <<EOF
unset key
set title 'Noise Power by for Best Frequency (${lo_sp_freq} Hz) for LO Setting of 143.75 MHz'
set ylabel 'Power at Offset (dB)'
set xlabel 'DDC Frequency (MHz)'
set terminal png size 1024, 768
set yrange [-30:-20]
set output '${fnPOWplotdat}_lo.png'
plot '${fnPOWplotdat}_lo' using (\$3*1e-6):(10*log10(\$4)) with lines 
EOF

    gnuplot <<EOF
unset key
set title 'Noise Power by for Worst Frequency (${hi_sp_freq} Hz) for LO Setting of 143.75 MHz'
set ylabel 'Power at Offset (dB)'
set xlabel 'DDC Frequency (MHz)'
set terminal png size 1024, 768
set yrange [-30:-20]
set output '${fnPOWplotdat}_hi.png'
plot '${fnPOWplotdat}_hi' using (\$3*1e-6):(10*log10(\$4)) with lines 
EOF
    i=$[$i+1]
  done

done

