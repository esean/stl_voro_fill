PROJECTS:
------------

voro_fill_stl/
	- (VTK+VORO++) given "any" STL file, fills the surface inside with voronoi mesh


translate_stl/
	- (VTK) simple tool allows translating an STL in (x,y,z) dimensions

clean_stl/
	- (VTK) runs vtkCleanFilter and writes STL

TubesFromSplines/
	- (VTK) This is used to convert the splines in a 9-pt CSV Voro++ '%B' output file (generated
		by 'voro_fill_stl') to 3D tubes, and output resultant STL

stl_remesh/
	- (VTK) Reduce triangle count in provided STL by X percentage. Used to reduce processing
		time of other stages.

ViewSTL/
	- (VTK) just open STL & display in VTK render window

9pt_csv_viewer/
	- (VTK) allows viewing mid-process data for debug. Overlays the input shrunk STL and voro
		output 9pt-CSV dataset. Used to ensure the voro correctly filled the input STL.



BETA (not working...)
------------
convert_to_stl/
	- (VTK) Reads in many formats and outputs an STL.

make_stl_solid/
	- (VTK) as it says



