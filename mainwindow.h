#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <displaywindow.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void init();

protected:
    void timerEvent(QTimerEvent *event);
//    void resizeEvent(QResizeEvent *e);

private slots:

    void on_openGL_radioButton_toggled(bool checked);

    void on_openCL_radioButton_toggled(bool checked);

private:
    Ui::MainWindow *ui;

    DisplayWindow *m_window;

    unsigned int m_cnt;
    QImage m_imgIn;
};

#endif // MAINWINDOW_H
