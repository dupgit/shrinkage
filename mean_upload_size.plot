set encoding iso_8859_15
set key right
set grid
set xlabel "Time"
set ylabel "Bytes"
set style line 1 lw 1
set title "Mean size of upload size parameter (to transfert one buffer of 5987456 bytes)"
set output "mean_upload_size.png"
set terminal png size 1024,768 enhanced font "Verdana,10"
plot "shrinkserver.output" using 3 title "Mean size in bytes" with lines
