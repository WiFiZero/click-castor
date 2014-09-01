if (exists("tikz")) set term tikz
if (exists("outfile")) set output outfile
set boxwidth 1 relative
set style data histograms
set bars large
set style fill solid 0.7 border lt -1
# Set fill colors
set linetype 1 lc rgb 'black' lt -1
set linetype 2 lc rgb 'grey' lt -1
set linetype 3 lc rgb 'white' lt -1

set style histogram errorbars gap 3 lw 1 # gap must be odd for odd number of data

# Only show x- and y-axis
set border 3 front

# Remove tic markers
set ytics border in scale 0,0
set xtics border in scale 0,0

# Set legend to column titles
set key outside center top horizontal reverse Left samplen 1 spacing 1.35 autotitle columnhead nobox

set grid y

set yrange [0:*]
if (exists("maxy")) set yrange[0:maxy]
if (exists("maxx")) if (exists("minx")) set xrange [minx:maxx]

set ylabel metric
set xlabel setting

plot for [COL=2:6:2] filename using COL:COL+1:xticlabels(1)
if (!exists("tikz")) pause -1