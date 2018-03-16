set term png size 800,600
set output 'tree-2-compare.png'

set title "Predicted Value Comparison: P=2"
set grid

set xlabel "Message size"
set logscale x 2
set xrange [4:1073741824]
set xtics ("8 B" 8,  "64 B" 64,  "1 KiB" 1024,  "16 KiB" 16384,  "256 KiB" 262144,  "4 MiB" 4194304, "64 MiB" 67108864, "1 GiB" 1073741824)

set ylabel "Broadcast time"
set logscale y 10
set yrange [1e-7:1e1]
set mytics 10

plot "tree-2.dat" using 2:3 title 'observed' with linespoints, \
     "tree-2-predicted.dat" using 2:3 title 'predicted' with linespoints

# eof
