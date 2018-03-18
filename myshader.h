#ifndef MYSHADER_H
#define MYSHADER_H

#include <openglwindow.h>
#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLFramebufferObject>


class DisplayShader : public QOpenGLShaderProgram
{
public:
    DisplayShader(OpenGLWindow *parent);
    void setVerticesArray(const QVector2D* vertices);
    void setTextureCoordinates(const QVector2D* texCoords);
    void begin();
    void end();

private:
    GLuint m_vertexLocation;
    GLuint m_texCoordLocation;
};

class ShaderLuminance : public QOpenGLShaderProgram
{
public:
    ShaderLuminance(OpenGLWindow *parent);
    void setTextureCoordinates(const QVector2D* texCoords);
    void setVerticesArray(const QVector2D* vertices);
    void begin();
    void end();

private:
    GLuint m_vertexLocation;
    GLuint m_texCoordLocation;
};

class MyShader
{
public:
    static void initialize(OpenGLWindow *glWindow);
    static void deleteShaders();

    static DisplayShader *displayShader;
    static ShaderLuminance *shaderLuminance;
};

#endif // MYSHADER_H
