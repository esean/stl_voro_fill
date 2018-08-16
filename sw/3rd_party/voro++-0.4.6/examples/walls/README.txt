2/21:
cat /tmp/points | grep 'inside? 1'  | cut -d: -f3 | nl | tr ',' ' '| awk '{print $1" "$2" "$3" "$4}' > pc.new 

1/29:
=======================
 (AM)
../plot_vtk.sh -p cad_small-cylinder.csv 0.1 0.1 (another 1.5" & 4" much finer mesh, 0.6x xlate scaling)
./xlate_pack_cylinder.py 0.45 > pcx.new
# STAT: 2057.728237 seconds, 2.012251 remaining: 30678 / 30708 splines, 6054 / 6062 faces, 470 / 472 cells, min_radius 0.898107
# (below for 4" no sphere top, just cylinder)
# STAT: 2056.007443 seconds, 4.611231 remaining: 30765 / 30834 splines, 6068 / 6083 faces, 470 / 472 cells, min_radius 0.869978

 (PM)
../plot_vtk.sh -p cad_small-cylinder.csv 0.1 0.1 (4" much finer mesh, 0.6x xlate scaling)
./xlate_pack_cylinder.py 0.49 > pc-0.49.new

1/27:
=======================
../plot_vtk.sh -p cad_small-cylinder.csv 0.1 0.1 (another 1.5" much finer mesh, 0.6x xlate scaling)
# STAT: 379.579064 seconds, 1.418825 remaining: 13109 / 13158 splines, 2618 / 2631 faces, 216 / 219 cells, min_radius 0.896431

../plot_vtk.sh -p cad_small-cylinder.csv 0.1 0.1 (4" much finer mesh, 0.6x xlate scaling)
# STAT: 4043.610170 seconds, 5.228110 remaining: 42539 / 42594 splines, 8434 / 8447 faces, 672 / 674 cells, min_radius 0.896733


1/25:
=======================
../plot_vtk.sh -p cad_small-cylinder.csv 0.3 0.7 (another 1.5" testing print, smaller tubes)
# STAT: 4.563358 seconds, 0.131455 remaining: 1458 / 1500 splines, 301 / 312 faces, 29 / 31 cells, min_radius 0.771223

../plot_vtk.sh -p cad_small-cylinder.csv 0.3 0.7 (final 4" at xlate 1.2x)



1/24/17:
=======================
# for 1.5x1.5 cylinder sphere top
../plot_vtk.sh -p cad_small-cylinder.csv 0.285 0.575 (for outer tube radius = 1.21)
# STAT: 3.760070 seconds, 0.090009 remaining: 1295 / 1326 splines, 268 / 275 faces, 26 / 27 cells, min_radius 1.211250
and 3d printing is producing *much* less stringies because tubes are thicker, maybe not so much hopping?

# smaller outside tubes
../plot_vtk.sh -p cad_small-cylinder.csv 0.285 0.64 (for outer tube radius = 1.0)

# and now smaller top sphere tube radius 
../plot_vtk.sh -p cad_small-cylinder.csv 0.3 0.63
# STAT: 3.526501 seconds, 0.084418 remaining: 1295 / 1326 splines, 268 / 275 faces, 26 / 27 cells, min_radius 1.030247

# and finally this 3" was printed
../plot_vtk.sh -p cad_small-cylinder.csv 0.3 0.64 (3" model - not printed)
# STAT: 17.604311 seconds, 0.268679 remaining: 3014 / 3060 splines, 612 / 622 faces, 55 / 56 cells, min_radius 1.080000
../plot_vtk.sh -p cad_small-cylinder.csv 0.3 0.63 (1.5" model - printed)
# STAT: 3.933182 seconds, 0.176965 remaining: 1378 / 1440 splines, 284 / 298 faces, 27 / 29 cells, min_radius 1.013701




1/21/17:
=======================
../plot_vtk.sh -p cad_small-cylinder.csv 0.1 0	# for 3in 3d-printing (1st proto)
#../plot_vtk.sh -p cad_small-cylinder.csv 0.25 0.675	# for 3d-printing (thicker inside, but outside was too thick, cell windows too small





(older)
=======================

#../plot_vtk.sh cad_small.csv 0.1 10

# cylinder
#../plot_vtk.sh -p cad_small-cylinder.csv 0.075 0.88
../plot_vtk.sh -p cad_small-cylinder.csv 0.06 0.75

# others...
../plot_vtk.sh -p cad_small-frustrum.csv 0.1 0.75
