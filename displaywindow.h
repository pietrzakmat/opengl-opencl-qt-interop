#ifndef DISPLAYWINDOW_H
#define DISPLAYWINDOW_H

#include <QMutexLocker>
#include <QVector2D>
#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>

#include <QDebug>
#include <iostream>
#include <string.h>
#include <openglwindow.h>

// OpenCL
#include <CL/cl.h>
#include <CL/cl_gl.h>
static const char* CL_GL_SHARING_EXT = "cl_khr_gl_sharing";
#define MAX_SOURCE_SIZE (0x100000)

class DisplayWindow : public OpenGLWindow
{

public:
    explicit DisplayWindow(QWindow *parent=0, int width=1280, int height=960);
    ~DisplayWindow();

    enum Rendering {
        OpenGL = 0,
        OpenCL
    };

    void setRenderType(int type) {m_renderType = type;}

    void render();

    inline void setInImage(const unsigned char* imageBuffer)
    {
        // copy images data to texture buffers
        for (int y=0; y<m_height; y++)
        {
            memcpy(m_image_texture + y * m_imageRgb_line_size, imageBuffer + y * m_imageRgb_line_size, m_imageRgb_line_size);
        }
    }

    void createTextureRGB(GLuint texId, unsigned int width, unsigned int height, GLuint internalFormat, GLuint inputFormat, GLuint minMagFilterType);

    void convertToLumOpenGL();

    // OpenCL
    void initializeOpenCl_Intel();
    void initializeOpenCl();
    void convertToLumOpenCL();

    char *strnstr(const char *s1, const char *s2, size_t len)
    {
        size_t l2;

        l2 = strlen(s2);
        if (!l2)
            return (char *)s1;
        while (len >= l2) {
            len--;
            if (!memcmp(s1, s2, l2))
                return (char *)s1;
            s1++;
        }
        return NULL;
    }

    int isExtensionSupported(const char* support_str, const char* ext_string, size_t ext_buffer_size);

protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();

private:
     QOpenGLFunctions *m_funcs;
     bool m_initialized;

     int m_renderType;

     QVector2D  m_displayVertices[4];
     QVector2D  m_displayTexCoords[4];
     QVector2D  m_processTexCoords[4];

     QMutex m_mutex;

     const int m_width;
     const int m_height;
     // Texture buffers
     unsigned char* m_image_texture;
     int m_imageRgb_line_size;
     // Framebuffer objects:
     QOpenGLFramebufferObject *m_fboIn;
     QOpenGLFramebufferObject *m_fboOut;

     // OpenCL
     cl_context m_contextCL;
     cl_command_queue m_queue;
     cl_kernel m_kernel;
     cl_mem m_memCurSrc;
     cl_mem m_memCurDst;
};

#endif // DISPLAYWINDOW_H
