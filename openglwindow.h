#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H

#include <QWindow>
#include <QOpenGLContext>
#include <QOpenGLFunctions>

class OpenGLWindow : public QWindow, protected QOpenGLFunctions
{
public:
    explicit OpenGLWindow(QWindow *parent = 0)
        : QWindow(parent)
        , m_context(0)
    {
        // Use OpenGL for this window
        setSurfaceType(OpenGLSurface);

        QSurfaceFormat format;
        format.setSwapBehavior(QSurfaceFormat::DefaultSwapBehavior);
        format.setSwapInterval(0); // attempt to switch ON the VSync. If =1 then VSync ON. Parameter defines number of frames of delay.
        // Create the native window
        setFormat(format);
        create();

        // Create an OpenGL context
        m_context = new QOpenGLContext;
        m_context->setFormat(format);
        if (!m_context->create()) {
            qFatal("Context creation failed");
        }
    }

    QOpenGLContext *openglContext() const { return m_context; }

protected:
    virtual void initializeGL() = 0;
    virtual void resizeGL( int width, int height ) = 0;
    virtual void paintGL() = 0;

    bool event(QEvent *event)
    {
        switch (event->type()) {
        case QEvent::Resize:
                m_context->makeCurrent(this);
                resizeGL(width(),height());
            return QWindow::event(event);

        default:
            return QWindow::event(event);
        }
    }

private:
    QOpenGLContext* m_context;
};

#endif // OPENGLWINDOW_H
