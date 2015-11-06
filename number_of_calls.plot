set encoding iso_8859_15
set key right
set grid
set xlabel "Time"
set ylabel "Number of ahc calls"
set style line 1 lw 1
set title "Number of ahc calls to transfert one buffer of 5593840 bytes"
set output "number_of_calls.png"
set terminal png size 1024,768 enhanced font "Verdana,10"
plot "cdpfglserver.2" using 2 title "Number of calls" with lines
