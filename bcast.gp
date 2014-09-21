set term png size 1024,768
set output 'bcast-2.png'

set title "Broadcast algorithms: P=2"
set grid

set xlabel "Message size"
set logscale x 2
set xrange [4:134217728]
set xtics ("4 B" 4,  "64 B" 64,  "1 KiB" 1024,  "16 KiB" 16384,  "256 KiB" 262144,  "4 MiB" 4194304)

set ylabel "Broadcast time"
set logscale y 10
set yrange [1e-6:1e1]
set mytics 10

plot "serial-2.dat" using 2:3 title "serial" with linespoints, \
     "tree-2.dat" using 2:3 title "tree" with linespoints, \
     "bigvec-2.dat" using 2:3 title "bigvec" with linespoints

# eof
