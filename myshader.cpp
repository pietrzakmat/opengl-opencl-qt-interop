#include "myshader.h"

DisplayShader* MyShader::displayShader = NULL;
ShaderLuminance* MyShader::shaderLuminance = NULL;

void MyShader::initialize(OpenGLWindow *glWindow)
{
    displayShader = new DisplayShader(glWindow);
    shaderLuminance = new ShaderLuminance(glWindow);
}

void MyShader::deleteShaders()
{
    delete displayShader;
    delete shaderLuminance;
}

//------------------------------------------------------------------------------
DisplayShader::DisplayShader(OpenGLWindow *parent) :
    QOpenGLShaderProgram(parent)
{
    if (!addShaderFromSourceFile(QOpenGLShader::Vertex, "../shaders/bypass_v.glsl"))
    {
        qWarning() << Q_FUNC_INFO << "vertex shader error:" << log();
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "vertex shader compiled";
    }

    if (!addShaderFromSourceCode(QOpenGLShader::Fragment,
        "#version 130\n"
        "in vec2 f_texCoord;\n"
        "out vec4 fragColor;\n"
        "uniform sampler2D tex;\n"
        "void main(void)\n"
        "{\n"
        "    fragColor = texture2D(tex, f_texCoord.xy);\n"
        "}\n"
    ))
    {
        qWarning() << Q_FUNC_INFO << "fragment shader error:" << log();
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "fragment shader compiled";
    }

    link();
    m_vertexLocation = attributeLocation("v_vertex");
    m_texCoordLocation = attributeLocation("v_texCoord");
}


void DisplayShader::setTextureCoordinates(const QVector2D* texCoords)
{
    setAttributeArray(m_texCoordLocation , texCoords);
}

void DisplayShader::setVerticesArray(const QVector2D* vertices)
{
    setAttributeArray(m_vertexLocation, vertices);
}

void DisplayShader::begin()
{
    bind();
    enableAttributeArray(m_vertexLocation);
    enableAttributeArray(m_texCoordLocation);
}

void DisplayShader::end()
{
    disableAttributeArray(m_vertexLocation);
    disableAttributeArray(m_texCoordLocation);
    release();
}

ShaderLuminance::ShaderLuminance(OpenGLWindow *parent) :
    QOpenGLShaderProgram(parent)
{
    if (!addShaderFromSourceFile(QOpenGLShader::Vertex, "../shaders/bypass_v.glsl"))
    {
        qWarning() << Q_FUNC_INFO <<  "vertex shader error:" << log();
    }
    else
    {
        qWarning() << Q_FUNC_INFO <<  "vertex shader compiled";
    }
    if (!addShaderFromSourceCode(QOpenGLShader::Fragment,
        "#version 130\n"
        "in vec2 f_texCoord;\n"
        "out vec4 fragColor;\n"
        "uniform sampler2D tex;\n"

         "float luminance( vec3 color )\n"
         "{\n"
             "return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;\n"
         "}\n"

        "void main(void)\n"
        "{\n"
            "float lum = luminance(texture2D(tex, f_texCoord.xy).rgb);\n"
            "fragColor = vec4(vec3(lum), 1.0f);\n"
        "}\n"
    ))
    {
        qWarning() << Q_FUNC_INFO <<  "fragment shader error:" << log();
    }
    else
    {
        qWarning() << Q_FUNC_INFO <<  "fragment shader compiled";
    }

    link();

    m_vertexLocation = attributeLocation("v_vertex");
    m_texCoordLocation = attributeLocation("v_texCoord");
}

void ShaderLuminance::setTextureCoordinates(const QVector2D* texCoords)
{
    setAttributeArray(m_texCoordLocation , texCoords);
}

void ShaderLuminance::setVerticesArray(const QVector2D* vertices)
{
    setAttributeArray(m_vertexLocation, vertices);
}

void ShaderLuminance::begin()
{
    bind();
    enableAttributeArray(m_vertexLocation);
    enableAttributeArray(m_texCoordLocation);
}

void ShaderLuminance::end()
{
    disableAttributeArray(m_vertexLocation);
    disableAttributeArray(m_texCoordLocation);
    release();
}




