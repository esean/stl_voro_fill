This project uses an input STL to generate an output STL which is the same form as the input STL, but filled with a cellular voronoi pattern.

For more information, see the sw/readme.txt

For example, for this input STL (in examples/bunny.stl),
```
ViewSTL examples/bunny.stl
```
![alt text](examples/bunny-input.png)

The rendered output STL,
```
ViewSTL sw/out.stl-0.stl
```
![alt text](examples/bunny-voro.png)

Using 'ViewSTL' (provided in this project) to view both input and output STL at the same time,
```
ViewSTL examples/bunny.stl sw/out.stl-0.stl
```
![alt text](examples/bunny-voro-together.png)

