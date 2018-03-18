#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_window(NULL)
{
    ui->setupUi(this);
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
    m_imgIn = QImage("../images/frontier_color57sb.jpg").convertToFormat(QImage::Format_RGB888);
//    m_imgIn = QImage("../images/Tux.png").convertToFormat(QImage::Format_RGB888);
    qDebug() << "m_imgIn.format():" << m_imgIn.format();

    if (m_imgIn.isNull())
    {
        qDebug() << "Image is not valid";
        return;
    }

    m_window = new DisplayWindow(NULL, m_imgIn.width(), m_imgIn.height());
    ui->centralWidget->layout()->addWidget(QWidget::createWindowContainer(m_window, this));

    QRect windowGeometry = geometry();
    windowGeometry.setSize(m_imgIn.size());
    setGeometry(windowGeometry);
    QApplication::processEvents();

    m_window->render(); // first time, to init

    int fps = 25;
    int t_fps = 1000.0f / fps;
    startTimer(t_fps);
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    QMainWindow::timerEvent(event);

    m_window->setInImage(m_imgIn.bits());
    m_window->render();
}

void MainWindow::on_openGL_radioButton_toggled(bool checked)
{
    if (checked == (bool)Qt::Checked)
    {
        m_window->setRenderType(DisplayWindow::OpenGL);
    }
}

void MainWindow::on_openCL_radioButton_toggled(bool checked)
{
    if (checked == (bool)Qt::Checked)
    {
        m_window->setRenderType(DisplayWindow::OpenCL);
    }
}
