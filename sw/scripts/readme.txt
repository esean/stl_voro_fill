Python common functions:
--------------------------
python_helpers.py
	- collection of common python functions that can be imported to scripts


Main scripts:
--------------------------
RunMe_MakeSTL.py
	- Generates a cell-packing-filled STL shape

		$ RunMe_MakeSTL.py [STL]


Other scripts:
--------------------------
run_voro_fill_stl.sh
	- Main worker script called by RunMe_MakeSTL.py

fix_stl.sh
	- fixes STL using admesh

mac_popup.sh
	- prompts the user with provided text

make_vtk_app.sh
	- compiles a VTK app using cmake

Dataviz helpers:
--------------------------
plot_vtk.sh
viz_packing.sh


Packing-cube helpers:
--------------------------
make_cube_this_big.sh
place_pack_cube_at_xyz.py
trim_pack_cylinder.py
xlate_pack_cylinder.py


data-csv helpers:
--------------------------
fpget_csv_column.rb
minmax.pl
minmax_col.sh


