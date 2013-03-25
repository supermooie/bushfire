bushfire
========

Real-time simulation of radiant heat transfer from a bushfire

File/Folder Information
=======================

* data: A generated bushfire at one time step - includes positional and size information.
* vrml/
  * top-extrusion.wrl: Building and heights extruded from ESRI Shapefiles and terrain data. Use Viewer, and open this file.
  * top-extrusion-spheres.wrl: Same as above, but increased the number of spheres to 100 000 to find the upper limit for a machine with 16GB RAM and Quadro 4000M.
* .../SimpleScene.cpp: Loads the bushfire data from the aforementioned data directory. Use wxMinimal2 to run the simulation.
