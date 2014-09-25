set term png size 800,600
set output 'serial.png'

set title "Broadcast: serial algorithm"
set grid

set xlabel "Message size"
set logscale x 2
set xrange [4:134217728]
set xtics ("4 B" 4,  "64 B" 64,  "1 KiB" 1024,  "16 KiB" 16384,  "256 KiB" 262144,  "4 MiB" 4194304)

set ylabel "Broadcast time"
set logscale y 10
set yrange [1e-7:1e1]
set mytics 10

plot "serial-2.dat" using 2:3 title 'P=2' with linespoints, \
     "serial-4.dat" using 2:3 title '4' with linespoints, \
     "serial-8.dat" using 2:3 title '8' with linespoints

# eof
