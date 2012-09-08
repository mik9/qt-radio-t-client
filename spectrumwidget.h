#ifndef SPECTRUMWIDGET_H
#define SPECTRUMWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMutexLocker>
#include <QtConcurrentRun>
#include <QResizeEvent>
#include <QWaitCondition>
#include <QElapsedTimer>
#include <QTimer>

#define FPS_NORMAL 100
#define FPS_SLOW 10
#define SHIFT_PIXELS_PER_SECOND 100
#define WIDTH 1.0

class SpectrumWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit SpectrumWidget(QWidget *parent = 0);
    ~SpectrumWidget();

    void setSampleSize(int _s) { _sample_size = _s; }
    void setSPF(int _s) { _samples_per_frame = _s; }
    void setTPF(double _s) { _time_per_frame = _s; }
    
    void new_data(float *_data, size_t len);
    void main_window_focus_changed(bool focused);

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
    void mousePressEvent(QMouseEvent *);

private:
    QImage *currentImage;
    QImage *bufferImage;

    QMutex dataMutex;
    QMutex waitMutex;
    QFuture<void> painterResult;
    QPoint lastPoint;
    QWaitCondition waiter;
    QTimer change_color_timer;

    float *data;
    volatile bool painting;
    volatile size_t dataLen;
    int _sample_size;
    int _samples_per_frame;
    double _time_per_frame;
    int bottom_color;
    int new_bottom;
    volatile int top_color;
    volatile int new_top;
    int __fps;

    void paintFunction();
    void __new_data(float *_data, size_t len);
    void swapBuffers();

private slots:
    void new_color();
    void change_color();
};

#endif // SPECTRUMWIDGET_H
