# opencl-opencl-qt-interop
The project attempts to show minimum example of OpenGL and OpenCL interoparability within Qt framework. 
The steps of interoperability are based on the publication in:
https://www.codeproject.com/Articles/685281/OpenGL-OpenCL-Interoperability-A-Case-Study-Using.
This project extends the proposed idea by utlizing Qt framework. The comparison of simple greyscale image conversion is made by implementing in two GPU-based technologies:
1) OpenGL based, implementation in fragment shader.
2) OpenCL based, implementation by OpenCL kernel.

## Getting Started
As it is Qt based project one can open in QtCreator and build it.
CLI based, in project directory:
1) qmake OpenCL_openGL_interop_Example.pro
2) make

# Issues
So far it was only tested under Linux.
