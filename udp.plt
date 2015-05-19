set terminal postscript landscape
set nolabel
set xlabel "window"
set xrange [0:30]
set ylabel "usec"
set yrange [0:8000000]
set output "udp.ps"
plot "100mbps_sw.dat" title "100mbps sliding window" with linespoints, 
"1gbps_sw.dat" title "1gbps sliding window" with linespoints, 
4167057 title "100mbps stopNwait" with line, 4663662 title "1gbps stopNwait" with line
