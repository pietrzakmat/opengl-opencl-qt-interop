#include "displaywindow.h"
#include <myshader.h>

#include <GL/glx.h>

DisplayWindow::DisplayWindow(QWindow *parent, int width, int height)
    : OpenGLWindow(parent)
    , m_funcs(NULL)
    , m_initialized(false)
    , m_width(width)
    , m_height(height)
{
    // Vertex array:
    m_displayVertices[0] = QVector2D(-1.0,-1.0);
    m_displayVertices[1] = QVector2D( 1.0,-1.0);
    m_displayVertices[2] = QVector2D( 1.0, 1.0);
    m_displayVertices[3] = QVector2D(-1.0, 1.0);

    m_displayTexCoords[0] = QVector2D(0.0, 1.0);
    m_displayTexCoords[1] = QVector2D(1.0, 1.0);
    m_displayTexCoords[2] = QVector2D(1.0, 0.0);
    m_displayTexCoords[3] = QVector2D(0.0, 0.0);

    m_processTexCoords[0] = QVector2D(0.0, 0.0);
    m_processTexCoords[1] = QVector2D(1.0, 0.0);
    m_processTexCoords[2] = QVector2D(1.0, 1.0);
    m_processTexCoords[3] = QVector2D(0.0, 1.0);

    m_image_texture = new unsigned char[m_width*m_height*3];
    m_imageRgb_line_size = width*3;
}

DisplayWindow::~DisplayWindow()
{
    delete m_image_texture;

    if (m_initialized)
    {
        delete m_fboIn;
        delete m_fboOut;
    }
    qDebug() << Q_FUNC_INFO;
}

using namespace std;
void DisplayWindow::initializeOpenCl_Intel()
{
    // Load the kernel source code into the array source_str
    FILE *fp;
    char *source_str;
    long unsigned int source_size;

    fp = fopen("../kernels/conv2lum.cl", "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }
    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose( fp );

    const char* required_platform_subname = "Intel";
    cl_int err = CL_SUCCESS;
    cl_uint num_of_platforms = 0;
    // get total number of available platforms:
    err = clGetPlatformIDs(0, 0, &num_of_platforms);
    cout << "clGetPlatformIDs err:" << err << endl;
    cout << "Number of available platforms: " << num_of_platforms << endl;

    cl_platform_id* platforms = new cl_platform_id[num_of_platforms];
    // get IDs for all platforms:
    err = clGetPlatformIDs(num_of_platforms, platforms, 0);
    cout << "clGetPlatformIDs err:" << err << endl;

    cl_uint selected_platform_index = num_of_platforms;
    cout << "Platform names:\n";

    for(cl_uint i = 0; i < num_of_platforms; ++i)
    {
        // Get the length for the i-th platform name
        size_t platform_name_length = 0;
        err = clGetPlatformInfo(
            platforms[i],
            CL_PLATFORM_NAME,
            0,
            0,
            &platform_name_length
            );

        // Get the name itself for the i-th platform
        char* platform_name = new char[platform_name_length];
        err = clGetPlatformInfo(
            platforms[i],
            CL_PLATFORM_NAME,
            platform_name_length,
            platform_name,
            0
            );

        cout << "    [" << i << "] " << platform_name;

        // decide if this i-th platform is what we are looking for
        // we select the first one matched skipping the next one if any
        if(
            strstr(platform_name, required_platform_subname) &&
            selected_platform_index == num_of_platforms // have not selected yet
            )
        {
            cout << " [Selected]";
            selected_platform_index = i;
            // do not stop here, just see all available platforms
        }

        cout << endl;
        delete [] platform_name;
    }

    if(selected_platform_index == num_of_platforms)
    {
        cerr
            << "There is no found platform with name containing \""
            << required_platform_subname << "\" as a substring.\n";
        return;
    }

    cl_platform_id platform = platforms[selected_platform_index];
    struct
    {
        cl_device_type type;
        const char* name;
        cl_uint count;
    }
    devices[] =
    {
        { CL_DEVICE_TYPE_CPU, "CL_DEVICE_TYPE_CPU", 0 },
        { CL_DEVICE_TYPE_GPU, "CL_DEVICE_TYPE_GPU", 0 },
        { CL_DEVICE_TYPE_ACCELERATOR, "CL_DEVICE_TYPE_ACCELERATOR", 0 }
    };

    const int NUM_OF_DEVICE_TYPES = sizeof(devices)/sizeof(devices[0]);

    cout << "Number of devices available for each type:\n";
    cout << "NUM_OF_DEVICE_TYPES: " << NUM_OF_DEVICE_TYPES << endl;
    // Now iterate over all device types picked above and
    // initialize num_of_devices
    for(int i = 0; i < NUM_OF_DEVICE_TYPES; ++i)
    {
        err = clGetDeviceIDs(
            platform,
            devices[i].type,
            0,
            0,
            &devices[i].count
            );

        if(CL_DEVICE_NOT_FOUND == err)
        {
            // that's OK to fall here, because not all types of devices, which
            // you query for may be available for a particular system
            devices[i].count = 0;
            err = CL_SUCCESS;
        }

        cout
            << "    " << devices[i].name << ": "
            << devices[i].count << endl;
    }

    const cl_uint type_index = 1;
    const cl_uint device_index = 0;
    cout << "Choice:\n"
         << devices[type_index].name << ": "
         << devices[type_index].count << endl;
    // Retrieve a list of device IDs with type selected by type_index
    cl_uint cur_num_of_devices = devices[type_index].count;
    cl_device_id* devices_of_type = new cl_device_id[cur_num_of_devices];
    err = clGetDeviceIDs(
        platform,
        devices[type_index].type,
        cur_num_of_devices,
        devices_of_type,
        0
        );
    cout << "***clGetDeviceIDs err= " << err << endl;

    cl_device_id device = devices_of_type[device_index];
    // Get string containing supported device extensions
    size_t ext_size = 1024;
    char* ext_string = (char*)malloc(ext_size);
    err = clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, ext_size, ext_string, &ext_size);
    cout << "***clGetDeviceInfo err= " << err << endl;
    cout << "***clGetDeviceInfo device= " << device << endl;

    // Search for GL support in extension string (space delimited)
    int supported = isExtensionSupported(CL_GL_SHARING_EXT, ext_string, ext_size);
    if( supported )
    {
        // Device supports context sharing with OpenGL
        std::cout << "Found GL Sharing Support!\n";
    }
    else
    {
        // Device supports context sharing with OpenGL
        std::cout << "Error: No GL Sharing Support!\n";
        return;
    }

//    cout << "CL_INVALID_CONTEXT= " << CL_INVALID_CONTEXT << endl;
     //-------------------
    // Select the default platform and create a context using this platform and the GPU on Linux:
    cl_context_properties cps[] = {
        CL_GL_CONTEXT_KHR,
//        (cl_context_properties)glXGetCurrentContext(),
        (cl_context_properties)this->openglContext()->globalShareContext(),
        CL_GLX_DISPLAY_KHR,
        (cl_context_properties)glXGetCurrentDisplay(),
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)platform,
        0
    };

    // Create an OpenCL context
//    m_contextCL = clCreateContext( cps, 1, &device_id, NULL, NULL, &ret);
    m_contextCL = clCreateContext( cps, 1, &device, NULL, NULL, &err);
    std::cout << "clCreateContext err= " << err << std::endl;

    if (err != CL_SUCCESS)
    {
        std::cout << "Error: clCreateContext err= " << err << std::endl;
        return;
    }
    // Create a command queue
    m_queue = clCreateCommandQueue(m_contextCL, device, 0, &err);
    std::cout << "clCreateCommandQueue err= " << err << std::endl;

    // Create a program from the kernel source
    cl_program program = clCreateProgramWithSource(m_contextCL, 1,
            (const char **)&source_str, (const long unsigned int *)&source_size, &err);
    std::cout << Q_FUNC_INFO << "clCreateProgramWithSource err=" << err << std::endl;
    // Build the program
    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    std::cout << Q_FUNC_INFO << "clBuildProgram err=" << err << std::endl;
    // Create the OpenCL kernel
    m_kernel = clCreateKernel(program, "conv2lum", &err);
    std::cout << Q_FUNC_INFO << "clCreateKernel err=" << err << std::endl;
    std::cout << "clCreateCommandQueue err= " << err << std::endl;

    cl_mem mem = clCreateFromGLTexture(
                m_contextCL ,
                CL_MEM_READ_ONLY ,
                GL_TEXTURE_2D ,
                0 ,
                m_fboIn->texture() ,
                &err
                );
    std::cout << "clCreateFromGLTexture err= " << err << std::endl;
    if (err != CL_SUCCESS)
    {
        std::cout << "Error: clCreateFromGLTexture err= " << err << std::endl;
    }
}


void DisplayWindow::initializeOpenCl()
{
     cout << "initializeOpenCl" << endl;
    // Load the kernel source code into the array source_str
    FILE *fp;
    char *source_str;
    long unsigned int source_size;

    fp = fopen("../kernels/conv2lum.cl", "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }
    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose( fp );

    // Get platform and device information
    cl_platform_id platform_id = NULL;
    cl_device_id device_id = NULL;
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int err = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    err = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_ALL, 1,
            &device_id, &ret_num_devices);

    // Get string containing supported device extensions
    size_t ext_size = 1024;
    char* ext_string = (char*)malloc(ext_size);
    err = clGetDeviceInfo(device_id, CL_DEVICE_EXTENSIONS, ext_size, ext_string, &ext_size);

    // Search for GL support in extension string (space delimited)
    int supported = isExtensionSupported(CL_GL_SHARING_EXT, ext_string, ext_size);
    if( supported )
    {
        // Device supports context sharing with OpenGL
        cout << "initializeOpenCl: Found GL Sharing Support!\n";
    }

    // Select the default platform and create a context using this platform and the GPU on Linux:
    cl_context_properties cps[] = {
        CL_GL_CONTEXT_KHR,
        (cl_context_properties)glXGetCurrentContext(),
//        (cl_context_properties)m_glWindow->openglContext()->globalShareContext(),
        CL_GLX_DISPLAY_KHR,
        (cl_context_properties)glXGetCurrentDisplay(),
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)platform_id,
        0
    };

    // Create an OpenCL context
    m_contextCL = clCreateContext( cps, 1, &device_id, NULL, NULL, &err);
    cout << "clCreateContext ret=" << err << endl;
    // Create a command queue
    m_queue = clCreateCommandQueue(m_contextCL, device_id, 0, &err);
    cout << "clCreateCommandQueue ret=" << err << endl;

    // Create a program from the kernel source
    cl_program program = clCreateProgramWithSource(m_contextCL, 1,
            (const char **)&source_str, (const long unsigned int *)&source_size, &err);
    cout << "clCreateProgramWithSource ret=" << err << endl;
    // Build the program
    err = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    cout << "clBuildProgram ret=" << err << endl;
    // Create the OpenCL kernel
    m_kernel = clCreateKernel(program, "conv2lum", &err);
    cout << "clCreateKernel ret=" << err << endl; //CL_SUCCESS

    // Create memory pointers to textures:
    m_memCurSrc = clCreateFromGLTexture(
                m_contextCL ,
                CL_MEM_READ_WRITE ,
                GL_TEXTURE_2D ,
                0 ,
                m_fboIn->texture() ,
                &err
                );

//    std::cout << get_error_string(err) << std::endl;

    m_memCurDst = clCreateFromGLTexture(
                m_contextCL ,
                CL_MEM_READ_WRITE ,
                GL_TEXTURE_2D ,
                0 ,
                m_fboOut->texture() ,
                &err
                );

    cout << "Create memory pointers to textures ret=" << err << endl;;
    if (err != CL_SUCCESS)
    {
        cout << "Error: Create memory pointers to textures FAILED! (error code CL_MEM_OBJECT_ALLOCATION_FAILURE = -4), ret=" << err;
        return;
    }

    // Now that we initialized the OpenCL texture buffer, we can set
    // them as kernel parameters, they won't change, the kernel will
    // always be executed on those buffers.
    err = clSetKernelArg( m_kernel , 0 , sizeof( m_memCurSrc ) , &m_memCurSrc );
    err = clSetKernelArg( m_kernel , 1 , sizeof( m_memCurDst ) , &m_memCurDst );

    cout << "clSetKernelArg ret=" << err << endl;;
}

void DisplayWindow::initializeGL()
{
    if (!m_funcs)
        m_funcs = openglContext()->functions();

    m_funcs->initializeOpenGLFunctions();

    std::cout << "glGetString(GL_SHADING_LANGUAGE_VERSION): " << m_funcs->glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    // Initialize stuff:
    MyShader::initialize(this);

    m_fboIn = new QOpenGLFramebufferObject(m_width, m_height, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RGB);
    m_fboOut = new QOpenGLFramebufferObject(m_width, m_height, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RGB);

    createTextureRGB(m_fboIn->texture(),m_fboIn->width(),m_fboIn->height(),m_fboIn->format().internalTextureFormat(), GL_UNSIGNED_BYTE, GL_LINEAR);
    createTextureRGB(m_fboOut->texture(),m_fboOut->width(),m_fboOut->height(),m_fboOut->format().internalTextureFormat(), GL_UNSIGNED_BYTE, GL_LINEAR);

//    initializeOpenCl_Intel();
    initializeOpenCl();
    m_initialized = true;
    cout << "finished";

    resizeGL(width(), height());
}

void DisplayWindow::render()
{
    QMutexLocker locker(&m_mutex);
    openglContext()->makeCurrent(this);

    // initialize at the first call:
    if (!m_initialized)
    {
        initializeGL();
        resizeGL(width(), height());
        return;
    }

    paintGL();

    // Swap front/back buffers
    openglContext()->swapBuffers(this);
    openglContext()->doneCurrent();
}

void DisplayWindow::convertToLumOpenCL()
{
    cl_int ret;
    // We must make sure that OpenGL is done with the textures, so
    // we ask to sync.
    m_funcs->glFinish();

    cl_mem lObjects[] = { m_memCurSrc , m_memCurDst };
    const int lNbObjects = sizeof( lObjects ) / sizeof( lObjects[0] );
    ret = clEnqueueAcquireGLObjects( m_queue , lNbObjects , lObjects , 0 , NULL , NULL );
    if (ret != CL_SUCCESS)
        cout << "Error clEnqueueAcquireGLObjects ret=" << ret;

    ret = clSetKernelArg( m_kernel , 0 , sizeof( m_memCurSrc ) , &m_memCurSrc );
    ret = clSetKernelArg( m_kernel , 1 , sizeof( m_memCurDst ) , &m_memCurDst );

    if (ret != CL_SUCCESS)
        cout << "Error clSetKernelArg ret=" << ret;

    cl_uint work_dim = 2;
    const size_t* global_work_offset = NULL;
    const size_t  global_work_size[2] = {(size_t)m_width, (size_t)m_height};
    const size_t* local_work_size = NULL;

    // Perform computations.
    ret = clEnqueueNDRangeKernel(
                m_queue ,
                m_kernel ,
                work_dim ,
                global_work_offset ,
                global_work_size ,
                local_work_size ,
                0 ,
                NULL ,
                NULL
                );
    if (ret != CL_SUCCESS)
        cout << "Error clEnqueueNDRangeKernel ret=" << ret;

    // with the event:
//    cl_event opengl_get_completion;
//    ret = clEnqueueNDRangeKernel(
//        m_queue ,
//        m_kernel ,
//        work_dim ,
//        global_work_offset ,
//        global_work_size ,
//        local_work_size ,
//        0 ,
//        NULL ,
//        &opengl_get_completion
//        );
//    clWaitForEvents(1, &opengl_get_completion);
//    clReleaseEvent(opengl_get_completion);

    // Before returning the objects to OpenGL, we sync to make sure OpenCL is done.
    ret = clEnqueueReleaseGLObjects( m_queue , lNbObjects , lObjects , 0 , NULL , NULL ); // this stops all
    ret = clFinish( m_queue ); // this slows down processing significantly
    if (ret != CL_SUCCESS)
        cout << "Error clEnqueueReleaseGLObjects ret=" << ret;

    cout << "convertToLumOpenCL" << endl;
}

void DisplayWindow::convertToLumOpenGL()
{
    m_fboOut->bind();
    m_funcs->glViewport(0, 0, m_width, m_height);
    m_funcs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_funcs->glActiveTexture(GL_TEXTURE0);
    m_funcs->glBindTexture(GL_TEXTURE_2D, m_fboIn->texture());
    MyShader::shaderLuminance->begin();
    MyShader::shaderLuminance->setVerticesArray(m_displayVertices);
    MyShader::shaderLuminance->setTextureCoordinates(m_processTexCoords);

    m_funcs->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    MyShader::shaderLuminance->end();
    m_fboOut->release();

//    cout << "convertToLumOpenGL"<< endl;
}


void DisplayWindow::paintGL()
{
    if (!m_initialized)
        return;

    if (!m_funcs)
        m_funcs = openglContext()->functions();

    m_funcs->glViewport(0, 0, m_width, m_height);
    m_funcs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_funcs->glActiveTexture(GL_TEXTURE0);

    m_fboIn->bind();
    m_funcs->glBindTexture(GL_TEXTURE_2D, m_fboIn->texture());
    // Copy CPU buffer to OpenGL texture:
    m_funcs->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, m_image_texture);
    m_fboIn->release();


    // Modify:
//    if (m_renderType == OpenGL){
//        convertToLumOpenGL();
//    }
    switch (m_renderType) {
    case OpenCL:
        convertToLumOpenCL();
        break;
    case OpenGL:
    default:
        convertToLumOpenGL();
        break;
    }

    // Display
    m_funcs->glViewport(0, 0, m_width, m_height);
    m_funcs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_funcs->glActiveTexture(GL_TEXTURE0);
    m_funcs->glBindTexture(GL_TEXTURE_2D, m_fboOut->texture());
    MyShader::displayShader->begin();
    MyShader::displayShader->setVerticesArray(m_displayVertices);
    MyShader::displayShader->setTextureCoordinates(m_displayTexCoords);

    m_funcs->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    MyShader::displayShader->end();
}


void DisplayWindow::resizeGL(int w, int h)
{
    qDebug() << Q_FUNC_INFO << "w=" << w << "h=" << h;
}


void DisplayWindow::createTextureRGB(GLuint texId, unsigned int width, unsigned int height, GLuint internalFormat, GLuint inputFormat, GLuint minMagFilterType)
{
    if (!m_funcs){
        std::cout << "Error creating texture" << std::endl;
        return;
    }

    m_funcs->glBindTexture(GL_TEXTURE_2D, texId);
    m_funcs->glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GL_RGB, inputFormat, NULL);
    m_funcs->glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,minMagFilterType);
    m_funcs->glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,minMagFilterType);
    m_funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    m_funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
}


int DisplayWindow::isExtensionSupported(const char* support_str, const char* ext_string, size_t ext_buffer_size)
{
    size_t offset = 0;
    const char* space_substr = strnstr(ext_string + offset, " ", ext_buffer_size - offset);
    size_t space_pos = space_substr ? space_substr - ext_string : 0;
    while (space_pos < ext_buffer_size)
    {
        if( strncmp(support_str, ext_string + offset, space_pos) == 0 )
        {
            // Device supports requested extension
            qDebug() << "Info: Found extension support " << support_str;
            return 1;
        }
        // Keep searching -- skip to next token string
        offset = space_pos + 1;
        space_substr = strnstr(ext_string + offset, " ", ext_buffer_size - offset);
        space_pos = space_substr ? space_substr - ext_string : 0;
    }
    qDebug() << "Warning: Extension not supported  " << support_str;
    return 0;
}
