# opengl-opencl-qt-interop
The project attempts to show minimum example of OpenGL and OpenCL interoparability within Qt framework. 
The steps of interoperability are based on following publications:
1) https://www.codeproject.com/Articles/685281/OpenGL-OpenCL-Interoperability-A-Case-Study-Using  
2) https://software.intel.com/en-us/articles/opencl-and-opengl-interoperability-tutorial  

This project extends the proposed ideas by utlizing Qt framework. The comparison of simple greyscale image conversion is made by implementing in two GPU-based technologies:  
A) OpenGL based, implementation in fragment shader.  
B) OpenCL based, implementation by OpenCL kernel.  

## Getting Started
As it is Qt based project one can open in QtCreator and build it.
CLI based, in project directory:
1) qmake OpenCL_openGL_interop_Example.pro
2) make

# Issues
So far it was only tested under Linux Ubuntu 16.04 and Ubuntu 17.10.
I tested it with several hardware platforms, with different OpenCL configurations:

1) Ubuntu 16.04 with nvidia Quadro K620 card.
It works here as expeceted. What is worth to mention is the CPU usage. OpenGL shader implementation consumes maximum up to 30% while OpenCL consumes 100% of the CPU load. I believe this is an issue, possibly with some buggy implementation or incorrect driver choice.

2) Ubuntu 16.04 and Intel Iris IGP (Integrated Graphics Processor) and using Intel SDK.
The issue is that on the call clCreateFromGLTexture(...) returns CL_INVALID_CONTEXT after the shared OpenCL context has been created correctly... So the example fails in that case. 
Checked the same with Beignet OpenCL implementation, the same error in that location.

3) Toshiba laptop with Intel HD Graphics 520 and Manjaro OS.
Beignet OpenCL implementation. The same clCreateFromGLTexture(...) returns CL_INVALID_CONTEXT...

To be continued...
